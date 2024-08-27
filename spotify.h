

#define DISABLE_AUDIOBOOKS
#define DISABLE_CATEGORIES
#define DISABLE_CHAPTERS
#define DISABLE_EPISODES
#define DISABLE_GENRES
#define DISABLE_MARKETS
#define DISABLE_PLAYLISTS
#define DISABLE_SEARCH
#define DISABLE_SHOWS
#define DISABLE_USER
#define DISABLE_SIMPLIFIED

#include <LiquidCrystal_I2C.h>
#include <Arduino.h>
#include <WiFi.h>
#include <SpotifyEsp32.h>
#include <BleKeyboard.h>

char* SSID = "Acanac18757";
const char* PASSWORD = "1237nzokkg8r";
const char* CLIENT_ID = "0fd6fefc1023457e8ebc913581318957";
const char* CLIENT_SECRET = "a537a116bddd404bb9ea10f727b8a552";
const char* REFRESH_TOKEN = "AQDYaQotym381ZQY0wQUHFMbRA7t_ElWhfJvd726zK_A9u4FrQRHCj-GxH1HVJ8bmU22n1kK88lqFk8H_TKqkhsMZRgkFcoUcTmShyA4gcfBrzh5URlINB_QGMKAjO8m-lU";

Spotify sp(CLIENT_ID, CLIENT_SECRET);

LiquidCrystal_I2C lcd(0x27, 20, 4);

void setup() {

    lcd.init();
    lcd.backlight(); 

    
    Serial.begin(115200);
    connect_to_wifi();

     
    
    sp.begin();
    while(!sp.is_auth()){
        sp.handle_client();
    }
    Serial.println("Authenticated");
}

void loop() {
    static String lastArtist;
    static String lastTrackname;
    
    String currentArtist = sp.current_artist_names();
    String currentTrackname = sp.current_track_name();
    
    if (lastArtist != currentArtist && currentArtist != "Something went wrong" && !currentArtist.isEmpty()) {
        lastArtist = currentArtist;
        Serial.println("Artist: " + lastArtist);
        lcd.setCursor(0,0);
        lcd.print(lastArtist);
    }
    
    if (lastTrackname != currentTrackname && currentTrackname != "Something went wrong" && currentTrackname != "null") {
        lastTrackname = currentTrackname;
        Serial.println("Track: " + lastTrackname);
        lcd.setCursor(0,2);
        lcd.print(lastTrackname);
    }
    if(bleKeyboard.isConnected()) {
    Serial.println("Sent");
    bleKeyboard.print("Request");

    delay(1000);

    Serial.println("Sending Enter key...");
    bleKeyboard.write(KEY_RETURN);

    delay(1000);

    Serial.println("Sending Play/Pause media key...");
    bleKeyboard.write(KEY_MEDIA_PLAY_PAUSE);

    delay(1000);

}

void connect_to_wifi(){
    WiFi.begin(SSID, PASSWORD);
    Serial.print("Connecting to WiFi...");
    lcd.print("Connecting to ");
    lcd.setCursor(0,2);
    lcd.print(SSID);
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.print(".");
    }
    Serial.printf("\nConnected to WiFi\n");
    lcd.clear();
    lcd.print("Connected to Wifi!");
    delay(500);
    lcd.clear();
}
