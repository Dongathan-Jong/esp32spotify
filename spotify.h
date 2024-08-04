#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <time.h>
#include <LittleFS.h>
#include <ESP8266mDNS.h>
#include <ESP8266WebServer.h>
#include <ArduinoJson.h>
#include <ESP8266HTTPClient.h>
#include <Ticker.h>

#include "secrets.h"
#include "lcd2004.h"

#define PLAYBACK_REFRSH_INTERVAL        2000
#define PLAYBACK_RETRY_INTERVAL         250
#define REQUEST_TIMEOUT_MS              500

LCD2004 lcd(2);
Ticker displayTicker;
ESP8266WebServer server(80);

typedef struct {
    String accessToken;
    String refreshToken;
} SpotifyToken;
SpotifyToken auth;

typedef struct {
    unsigned long millis;
    unsigned int progress;
    unsigned int duration;
    String track_name;
    String album_name;
    String artist_name;
    String track_id;
    bool playing;
} SpotifyPlayback;
SpotifyPlayback playback;

String lastTrack;

StaticJsonDocument<12288> lyricDoc;
const char* p_lyric_start;
const char* p_lyric_next = NULL;
const char* p_lyric_current = NULL;
unsigned int next_lyric_ms;

String spotifyAuth() {
    String oneWayCode = "";

    if(!MDNS.begin(MDNS_HOSTNAME)) {
        Serial.println(F("FATAL: MDNS error"));
        while(1) yield();
    }
    Serial.println(F("mDNS started"));

    server.on("/", []() {
        server.sendHeader("Location", F("https://accounts.spotify.com/authorize/?client_id=" SP_CLIENT_ID \
                                        "&response_type=code&redirect_uri=" SP_REDIRECT_URI \
                                        "&scope=user-read-private%20user-read-currently-playing%20user-read-playback-state"), true);
        server.send ( 302, "text/plain", "");
    });

    // Retrieve auth code returned by Spotify
    server.on ("/callback/", [&oneWayCode](){
        if(!server.hasArg("code")) {
            server.send(500, "text/plain", "BAD ARGS");
        } else {
            oneWayCode = server.arg("code");
            server.send (200, "text/html", F("Spotify authorization complete. You can close this window."));
        }
    });

    server.begin();
    Serial.println(F("HTTP server started"));

    while(oneWayCode == "") {
        server.handleClient();
        MDNS.update();
        yield();
    }
    server.stop();
    MDNS.close();
    return oneWayCode;
}

void getToken(bool refresh, String code) {
    WiFiClientSecure client;
    client.setInsecure();
    const char* host = "accounts.spotify.com";
    const int port = 443;
    String url = "/api/token";
    if (!client.connect(host, port)) {
        Serial.println("connection failed");
        return;
    }



  JsonObject lyric_filter = filter["message"]["body"]["macro_calls"]["track.subtitles.get"].createNestedObject("message");
  lyric_filter["header"]["available"] = true;
  lyric_filter["body"]["subtitle_list"][0]["subtitle"]["subtitle_body"] = true;

  DeserializationError error = deserializeJson(lyricDoc, client, DeserializationOption::Filter(filter), DeserializationOption::NestingLimit(12));
  if (error) {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.f_str());
    return;
  }

  int lyric_available = lyricDoc["message"]["body"]["macro_calls"]["track.subtitles.get"]["message"]["header"]["available"];
  if(lyric_available) {
    p_lyric = lyricDoc["message"]["body"]["macro_calls"]["track.subtitles.get"]["message"]["body"]["subtitle_list"][0]["subtitle"]["subtitle_body"];
  }

  http.end();
}

void saveRefreshToken(String refreshToken) {
  File f = LittleFS.open("/sptoken.txt", "w+");
  if (!f) {
    Serial.println("Failed to open sptoken");
    return;
  }
  f.println(refreshToken);
  f.close();
  Serial.println("Saved sptoken");
}

String loadRefreshToken() {
  File f = LittleFS.open(F("/sptoken.txt"), "r");
  if (!f) {
    Serial.println("Failed to open sptoken");
    return "";
  }
  while(f.available()) {
      //Lets read line by line from the file
      String token = f.readStringUntil('\r');
      Serial.print("Loaded Token: ");
      Serial.println(token);
      f.close();
      return token;
  }
  return "";
}

int code = http.GET();
    Serial.print("Response code: ");
    Serial.println(code);
    if(code == 301) {
        mxmCookie = http.header("Set-Cookie");
        delay(100);
        http.addHeader("Cookie", mxmCookie);
        code = http.GET();
        Serial.print("Response code: ");
        Serial.println(code);
    } else if(code != 200) {
        return;
    }
 Serial.println("connection failed");
        return;
    }

    String codeParam = "code";
    String grantType = "authorization_code";
    if (refresh) {
        grantType = codeParam = "refresh_token";
    }
    String authorization = base64::encode(F(SP_CLIENT_ID ":" SP_CLIENT_SECRET), false);
    // This will send the request to the server
    String content = "grant_type=" + grantType + "&" + codeParam + "=" + code + "&redirect_uri=" SP_REDIRECT_URI;
    String request = String("POST ") + url + " HTTP/1.1\r\n" +
                             "Host: " + host + "\r\n" +
                             "Authorization: Basic " + authorization + "\r\n" +
                             "Content-Length: " + String(content.length()) + "\r\n" +
                             "Content-Type: application/x-www-form-urlencoded\r\n" +
                             "Connection: close\r\n\r\n" +
                             content;
    client.print(request);

    unsigned long req_start = millis();
    while(!client.available()) {
        if(millis() - req_start > REQUEST_TIMEOUT_MS) {
            Serial.println(F("Request Timeout"));
            return;
        }
        delay(10);
    }

    while (client.connected()) {
        String line = client.readStringUntil('\n');
        if (line == "\r") {
                break;
        }
    }

    StaticJsonDocument<32> filter;
    filter["access_token"] = true;
    filter["refresh_token"] = true;
    StaticJsonDocument<512> doc;
    DeserializationError error = deserializeJson(doc, client, DeserializationOption::Filter(filter));
    if (error) {
        Serial.print(F("deserializeJson() failed: "));
        Serial.println(error.f_str());
        return;
    }
    auth.accessToken = (const char*)doc["access_token"];
    auth.refreshToken = (const char*)doc["refresh_token"];
}

int updatePlayback() {
    int ret_code = 0;
    WiFiClientSecure client;
    client.setInsecure();

    String host = "api.spotify.com";
    const int port = 443;
    String url = "/v1/me/player/currently-playing";
    if (!client.connect(host.c_str(), port)) {
        Serial.println(F("Connection failed"));
        return ret_code;
    }

    String request = "GET " + url + " HTTP/1.1\r\n" +
                             "Host: " + host + "\r\n" +
                             "Authorization: Bearer " + auth.accessToken + "\r\n" +
                             "Connection: close\r\n\r\n";
    client.print(request);

    unsigned long req_start = millis();
    while(!client.available()) {
        if(millis() - req_start > REQUEST_TIMEOUT_MS) {
            Serial.println(F("Request Timeout"));
            return ret_code;
        }
        delay(10);
    }

    while (client.connected()) {
        String line = client.readStringUntil('\n');
        if(line.startsWith(F("HTTP/1"))) {
            ret_code = line.substring(9, line.indexOf(' ', 9)).toInt();
        }
        if(line == "\r") {
                break;
        }
    }

    StaticJsonDocument<192> filter;

    JsonObject lyric_filter = filter["message"]["body"]["macro_calls"]["track.subtitles.get"].createNestedObject("message");
    lyric_filter["header"]["available"] = true;
    lyric_filter["body"]["subtitle_list"][0]["subtitle"]["subtitle_body"] = true;

    DeserializationError error = deserializeJson(lyricDoc, client, DeserializationOption::Filter(filter), DeserializationOption::NestingLimit(12));
    if (error) {
        Serial.print(F("deserializeJson() failed: "));
        Serial.println(error.f_str());
        return;
    }

    int lyric_available = lyricDoc["message"]["body"]["macro_calls"]["track.subtitles.get"]["message"]["header"]["available"];
    if(lyric_available) {
        p_lyric_start = lyricDoc["message"]["body"]["macro_calls"]["track.subtitles.get"]["message"]["body"]["subtitle_list"][0]["subtitle"]["subtitle_body"];
        p_lyric_next = p_lyric_start;
    }

    http.end();
}

void saveRefreshToken(String refreshToken) {
    File f = LittleFS.open(F("/sptoken.txt"), "w");
    if (!f) {
        Serial.println(F("Failed to write sptoken"));
        return;
    }
    f.println(refreshToken);
    f.close();
    Serial.println(F("Saved token"));
}

String loadRefreshToken() {
    File f = LittleFS.open(F("/sptoken.txt"), "r");
    if (!f) {
        Serial.println(F("Failed to read sptoken"));
        return "";
    }
    while(f.available()) {
        String token = f.readStringUntil('\r');
        Serial.println(F("Loaded token"));
        f.close();
        return token;
    }
    return "";
}


unsigned int parseInt(const char* ptr) {
  unsigned int ret = 0;
  while(*ptr >= '0' && *ptr <= '9') {
    ret *= 10;
    ret += (*ptr++ - '0');
  }
  return ret;
}

  }
}
