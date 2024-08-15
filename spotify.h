#include <LiquidCrystal_I2C.h>
#include <Arduino.h>
#include <WiFi.h>
#include <SpotifyEsp32.h>

char* SSID = 
const char* PASSWORD = 
const char* CLIENT_ID = 
const char* CLIENT_SECRET =
const char* REFRESH_TOKEN =

Spotify sp(CLIENT_ID, CLIENT_SECRET);

LiquidCrystal_I2C lcd(0x27, 20, 4);

const int button1Pin = 2;  
const int button2Pin = 3;  

bool button1State = false;
bool button2State = false;
bool lastButton1State = false;
bool lastButton2State = false;

void setup() {
    lcd.init();
    lcd.backlight(); 

    Serial.begin(115200);
    pinMode(button1Pin, INPUT_PULLUP);
    pinMode(button2Pin, INPUT_PULLUP);

    connect_to_wifi();
    lcd.clear();
    lcd.print("Initializing Spotify");
    sp.begin();
    
    while (!sp.is_auth()) {
        sp.handle_client();
        lcd.setCursor(0, 1);
        lcd.print(".");
        delay(500);
    }
    
    Serial.println("Authenticated");
    lcd.clear();
    lcd.print("Spotify Auth'd");
    delay(2000);
    lcd.clear();
}

void loop() {
    static String lastArtist;
    static String lastTrackname;
    static String lastPlaybackStatus;

    String currentArtist = sp.current_artist_names();
    String currentTrackname = sp.current_track_name();
    String playbackStatus = sp.get_playback_state() ? "Playing" : "Paused";

    if (lastArtist != currentArtist && currentArtist != "Something went wrong" && !currentArtist.isEmpty()) {
        lastArtist = currentArtist;
        Serial.println("Artist: " + lastArtist);
        lcd.setCursor(0, 0);
        lcd.print("Artist: ");
        lcd.print(lastArtist.substring(0, 13));
    }
    
    if (lastTrackname != currentTrackname && currentTrackname != "Something went wrong" && currentTrackname != "null") {
        lastTrackname = currentTrackname;
        Serial.println("Track: " + lastTrackname);
        lcd.setCursor(0, 1);
        lcd.print("Track: ");
        lcd.print(lastTrackname.substring(0, 13)); 
    }

    if (lastPlaybackStatus != playbackStatus) {
        lastPlaybackStatus = playbackStatus;
        Serial.println("Status: " + playbackStatus);
        lcd.setCursor(0, 2);
        lcd.print("Status: ");
        lcd.print(playbackStatus);
    }

    button1State = digitalRead(button1Pin) == LOW;
    if (button1State && !lastButton1State) {
        Serial.println("Button 1 pressed: Toggle Play/Pause");
        sp.toggle_play();
        delay(500); 
    }
    lastButton1State = button1State;

    button2State = digitalRead(button2Pin) == LOW;
    if (button2State && !lastButton2State) {
        Serial.println("Button 2 pressed: Next Track");
        sp.next_track();
        delay(500);
    }
    lastButton2State = button2State;

    delay(1000); 
}

void connect_to_wifi() {
    lcd.clear();
    lcd.print("Connecting to WiFi");
    lcd.setCursor(0, 1);
    lcd.print(SSID);

    WiFi.begin(SSID, PASSWORD);
    Serial.print("Connecting to WiFi...");
    int counter = 0;

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
        lcd.setCursor(counter % 20, 2);
        lcd.print(".");
        counter++;
    }

    Serial.println("\nConnected to WiFi");
    lcd.clear();
    lcd.print("Connected to WiFi");
    lcd.setCursor(0, 1);
    lcd.print(SSID);
    delay(2000);
    lcd.clear();
}
