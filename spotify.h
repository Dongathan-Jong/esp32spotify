
#include <LiquidCrystal_I2C.h>
#include <Arduino.h>
#include <WiFi.h>
#include <SpotifyEsp32.h>


bool newSong = false;

Spotify sp(CLIENT_ID, CLIENT_SECRET);

LiquidCrystal_I2C lcd(0x27, 20, 4);
String lastArtist;
String lastTrackname;
String currentArtist;
String currentTrackname;

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
    
    
    currentArtist = sp.current_artist_names();
    currentTrackname = sp.current_track_name();
    
    if (lastArtist != currentArtist && currentArtist != "Something went wrong" && !currentArtist.isEmpty()) {
        newSong = true;
        lastArtist = currentArtist;
        
        refreshSong();
        
        Serial.println("New Artist");
    }

        
    
    if (lastTrackname != currentTrackname && currentTrackname != "Something went wrong" && currentTrackname != "null") {
        newSong = true;
        
        lastTrackname = currentTrackname;
        refreshSong();
        
        Serial.println("New Song");
    }
}

void refreshSong()
{
  if(newSong)
  {
        lcd.clear();
        Serial.println("Artist: " + lastArtist);
        Serial.println("Song: " + lastTrackname);
        if(lastArtist.length() > 19)
        {
          lcd.setCursor(0,0);
          lcd.print(lastArtist.substring(0,19));
          lcd.setCursor(0,1);
          lcd.print(lastArtist.substring(20,40));
        }
        else{
          lcd.setCursor(0,0);
          lcd.print(lastArtist);
        }
        if(lastTrackname.length() > 19)
        {
          lcd.setCursor(0,2);
          lcd.print(lastTrackname.substring(0,19));
          lcd.setCursor(0,3);
          lcd.print(lastTrackname.substring(20,40));
        }
        else{
          lcd.setCursor(0,2);
          lcd.print(lastTrackname);
        }
  }
        newSong = false;
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
