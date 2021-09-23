#if defined(ESP32)
#include <FirebaseESP32.h>
#include <WiFi.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#include <FirebaseESP8266.h>
#endif

#include "addons/RTDBHelper.h"
#include "secret.hpp"

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

unsigned long dataMillis = 0;
int count = 0;

void setup() {

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

  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(D1, INPUT_PULLUP);
}

void loop() {
  if (millis() - dataMillis > 125) {
    dataMillis = millis();

    Firebase.setInt(fbdo, "light", analogRead(A0));
    Firebase.setInt(fbdo, "count", count++);
    Firebase.setInt(fbdo, "button", digitalRead(D1));

    Firebase.getInt(fbdo, "led");
    int led = fbdo.intData();
    digitalWrite(LED_BUILTIN, !led);

    Serial.printf("Led: %d \n", led);
    Serial.printf("LDR: %d \n", analogRead(A0));
  }
}