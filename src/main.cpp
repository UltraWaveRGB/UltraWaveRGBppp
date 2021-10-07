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

int led_r = D0;
int led_y = D2;
int led_g = D1;
int buzzer = D3;
int button = D4;
int door_is_open;
int power;
unsigned long time_mseconds;

void spin() {
  // ** roda por n segundos
  unsigned long old = millis();
  unsigned long now = millis();
  unsigned int time_remain = time_mseconds / 1000;
  while (now - old < time_mseconds) {
    now = millis();
    if (door_is_open == OPEN) {
      break;
    }
    // FIXME: envio de timer para firebase
    unsigned int current_left = (time_mseconds - (now - old)) / 1000;
    if (current_left != time_remain) {
      time_remain = current_left;
      Serial.printf("Time remaining: %d \n", time_remain);
      // Firebase.setInt(fbdo, "time", time_remain);
    }
    // ** evita bloqueio das funcoes em background na placa
    yield();
  }
  stop();
}

void start() {
  if (door_is_open == CLOSED) {
    tone(buzzer, 500, 250);
    digitalWrite(led_g, ON);
    digitalWrite(led_r, power);
    spin();
  }
}

void stop() {
  Firebase.setInt(fbdo, "time", 0);
  digitalWrite(led_r, OFF);
  digitalWrite(led_g, OFF);
  tone(buzzer, 757, 200);
}

void open_door_is_open() {
  digitalWrite(led_y, ON);
  door_is_open = OPEN;
}

void close_door_is_open() {
  digitalWrite(led_y, OFF);
  door_is_open = CLOSED;
}

/* --------------------------- */

void read_button() {
  if (door_is_open == CLOSED) {
    open_door_is_open();
  } else {
    close_door_is_open();
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

  pinMode(buzzer, OUTPUT);
  pinMode(led_r, OUTPUT);
  pinMode(led_g, OUTPUT);
  pinMode(led_y, OUTPUT);
}

void loop() {
  if (millis() - dataMillis > 125) {
    dataMillis = millis();

    Firebase.getInt(fbdo, "time");
    time_mseconds = fbdo.intData() * 1000;

    Firebase.getInt(fbdo, "start");
    if (fbdo.intData() == 1 and time_mseconds != 0) {
      start();
      Firebase.setInt(fbdo, "start", 0);
    }

    Firebase.getInt(fbdo, "power");
    power = fbdo.intData();
    if (power > MAX_BRIGHTNESS) {
      power = MAX_BRIGHTNESS;
    }
    Serial.printf("power: %d \n", power);
  }
}