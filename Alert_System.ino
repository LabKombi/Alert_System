#include "Arduino.h"
#include "DFRobotDFPlayerMini.h"
#include <ESP8266WiFi.h>
#include <Firebase_ESP_Client.h>
#include <SoftwareSerial.h>

// DFPlayer setup
SoftwareSerial softSerial(4, 5);  // RX, TX
#define FPSerial softSerial
DFRobotDFPlayerMini myDFPlayer;

// Firebase setup
FirebaseData firebaseData;
FirebaseAuth auth;
FirebaseConfig config;

#define WIFI_SSID "JTI-3.11"             // Masukkan SSID WiFi Anda
#define WIFI_PASSWORD "" // Masukkan Password WiFi Anda
#define FIREBASE_HOST "https://drowsines-e2e79-default-rtdb.firebaseio.com/"
#define FIREBASE_AUTH "NUIz8GXU4AD8qaPqAycd049ex8TTNm0qZ1pYwZ4Y"

// State variables
String currentStatus = "";  // Status saat ini
String previousStatus = ""; // Status sebelumnya
bool isPlaying = false;     // Apakah sedang memutar
int currentTrack = 0;       // Track yang sedang dimainkan

void setup() {
  // Serial setup
  Serial.begin(115200);
  FPSerial.begin(9600); // Serial untuk DFPlayer Mini

  // WiFi setup
  Serial.print("Connecting to Wi-Fi");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected to Wi-Fi");

  // Firebase setup
  config.host = FIREBASE_HOST;
  config.signer.tokens.legacy_token = FIREBASE_AUTH;

  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);

  // DFPlayer setup
  Serial.println(F("Initializing DFPlayer... (May take 3~5 seconds)"));
  if (!myDFPlayer.begin(FPSerial, true, true)) {
    Serial.println(F("Unable to begin DFPlayer. Please check connections."));
    while (true); // Hentikan jika DFPlayer tidak terdeteksi
  }
  myDFPlayer.volume(5); // Set volume awal
  Serial.println(F("DFPlayer Mini is ready."));
}

void loop() {
  // Ambil data dari Firebase Realtime Database
  if (Firebase.RTDB.getString(&firebaseData, "/pengendara")) { // Path ke database Firebase
    if (firebaseData.dataType() == "string") {
      currentStatus = firebaseData.stringData();
      Serial.print(F("Status: "));
      Serial.println(currentStatus);

      if (currentStatus != previousStatus) { // Jika status berubah
        previousStatus = currentStatus;
        handleStatusChange(); // Tangani perubahan status
      }
    } else {
      Serial.println(F("Data type mismatch."));
    }
  } else {
    Serial.print(F("Failed to get data from Firebase: "));
    Serial.println(firebaseData.errorReason());
  }

  // Periksa apakah track selesai
  if (isPlaying && myDFPlayer.available() && myDFPlayer.readType() == DFPlayerPlayFinished) {
    Serial.println(F("Track finished. Restarting..."));
    myDFPlayer.play(currentTrack); // Mulai ulang track yang sama
  }

  delay(500); // Interval membaca Firebase
}

void handleStatusChange() {
  if (currentStatus == "sleepy") {
    Serial.println(F("Playing track 1..."));
    currentTrack = 1;         // Set track 1
    myDFPlayer.play(1);       // Memutar track 1
    isPlaying = true;         // Tandai sedang memutar
  } else if (currentStatus == "tired") {
    Serial.println(F("Playing track 2..."));
    currentTrack = 2;         // Set track 2
    myDFPlayer.play(2);       // Memutar track 2
    isPlaying = true;         // Tandai sedang memutar
  } else if (currentStatus == "not sleepy") {
    Serial.println(F("Stopping playback..."));
    myDFPlayer.stop();        // Berhenti memutar
    isPlaying = false;        // Tandai tidak memutar
    currentTrack = 0;         // Reset track
  } else {
    Serial.println(F("Unknown status received from Firebase."));
  }
}
