
#define DISABLE_PLAYER
#define DISABLE_ALBUM
#define DISABLE_ARTIST
#define DISABLE_AUDIOBOOKS
#define DISABLE_CATEGORIES
#define DISABLE_CHAPTERS
#define DISABLE_EPISODES
#define DISABLE_GENRES
#define DISABLE_MARKETS
#define DISABLE_PLAYLISTS
#define DISABLE_SEARCH
#define DISABLE_SHOWS
#define DISABLE_TRACKS
#define DISABLE_USER
#define DISABLE_SIMPLIFIED
#define DISABLE_WEB_SERVER

#include <LiquidCrystal_I2C.h>
#include <Arduino.h>
#include <WiFi.h>
#include <SpotifyEsp32.h>



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
