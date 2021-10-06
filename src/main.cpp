#if defined(ESP32)
#include <FirebaseESP32.h>
#include <WiFi.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#include <FirebaseESP8266.h>
#endif

#include "addons/RTDBHelper.h"
#include "microwave.hpp"
#include "secret.hpp"

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

unsigned long dataMillis = 0;
int count = 0;

/* --- microwave functions --- */

// TODO: checar
int led_r;
int led_y;
int led_g;
int door;
int button;
unsigned long time_seconds;

void spin() {
  // ** roda por n segundos
  unsigned long old = millis();
  unsigned long now = millis();
  while (now - old < time_seconds) {
    now = millis();
    if (door == OPEN) {
      break;
    }
    // ** evita bloqueio das funcoes em background na placa
    yield();
  }
  stop();
}

void start() {
  if (door == CLOSED) {
    tone(D3, 500, 250);
    led_y = ON;
    led_g = ON;
    led_r = ON;
    spin();
  }
}

void stop() {
  led_r = OFF;
  led_g = OFF;
  tone(D3, 757, 100);
  tone(D3, 757, 100);
  tone(D3, 757, 100);
}

void open_door() {
  led_y = ON;
  door = OPEN;
}

void close_door() {
  led_y = OFF;
  door = CLOSED;
}

/* --------------------------- */

void read_button() {
  if (door == CLOSED) {
    open_door();
  } else {
    close_door();
  }
}

void setup() {

  /* ESP8266 connection */

  Serial.begin(115200);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(300);
  }

  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();
  Serial.printf("Firebase Client v%s\n\n", FIREBASE_CLIENT_VERSION);

  config.database_url = DATABASE_URL;
  config.signer.tokens.legacy_token = DATABASE_SECRET;

  Firebase.reconnectWiFi(true);
  Firebase.begin(&config, &auth);

  /* Pins */

  // TODO: setar as coisas
  pinMode(D3, OUTPUT);
}

void loop() {
  if (millis() - dataMillis > 125) {
    dataMillis = millis();

    Firebase.getInt(fbdo, "time");
    time_seconds = fbdo.intData() * 1000;

    Firebase.getInt(fbdo, "start");
    if (fbdo.intData() == 1 and time_seconds != 0) {
      start();
    }
    Firebase.setInt(fbdo, "start", 0);
  }
}