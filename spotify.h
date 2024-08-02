#include <I2CIO.h>
#include <LCD.h>
#include <LiquidCrystal_I2C.h>


#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <time.h>
#include <LittleFS.h>
#include <ESP8266mDNS.h>
#include <ESP8266WebServer.h>
#include <ArduinoJson.h>
#include <ESP8266HTTPClient.h>

#include <Wire.h> 


#include "secrets.h"

#define PLAYBACK_REFRSH_INTERVAL    5000
#define PLAYBACK_REFRESH_MARGIN     3000
#define PLAYBACK_RETRY_INTERVAL     250

LiquidCrystal_I2C lcd(0x27,2,1,0,4,5,6,7,3,POSITIVE);
ESP8266WebServer server(80);



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
        // Redirect: save the cookies and send them back.
        mxmCookie = http.header("Set-Cookie");
        delay(100);
        http.addHeader("Cookie", mxmCookie);
        code = http.GET();
        Serial.print("Response code: ");
        Serial.println(code);
    } else if(code != 200) {
        return;
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
