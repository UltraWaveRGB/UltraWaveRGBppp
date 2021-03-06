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
unsigned long time_now = 0;
unsigned long time_past = 0;
unsigned long timer = 0;

int led_r = D0;
int led_y = D2;
int led_g = D1;
int buzzer = D3;
int button = D4;
int execution_has_finished = FALSE;

int state = OFF_DOOR_CLOSED;
int power;

int is_door_open() { return !digitalRead(button); }

int stop_button_was_pressed() {
  Firebase.getInt(fbdo, "stop_button_was_pressed");
  int pressed = fbdo.intData();
  if (pressed == TRUE) {
    Firebase.setIntAsync(fbdo, "stop_button_was_pressed", FALSE);
    time_past = millis();
    return TRUE;
  }
  return FALSE;
}

int start_button_was_pressed() {
  Firebase.getInt(fbdo, "start_button_was_pressed");
  int pressed = fbdo.intData();
  if (pressed == TRUE) {
    Firebase.getInt(fbdo, "timer");
    timer = fbdo.intData();
    Firebase.setInt(fbdo, "start_button_was_pressed", FALSE);
    time_past = millis();
    return TRUE;
  }
  return FALSE;
}

int execution_finished() {
  if (timer == 0) {
    execution_has_finished = TRUE;
    return TRUE;
  }
  return FALSE;
}

void update_timer() {
  if (timer > 0) {
    timer--;
    Firebase.setIntAsync(fbdo, "timer", timer);
    Serial.printf("timer: %ld\n", timer);
  }
}

void update_power() {
  Firebase.getInt(fbdo, "power");
  power = fbdo.intData();
  if (power > MAX_POWER) {
    power = MAX_POWER;
  }
}

void ring_buzzer() {
  Serial.println("Execution Finished - Ringing buzzer");
  execution_has_finished = FALSE;
  for (int i = 0; i < NUMBER_OF_TONES; i++) {
    if (is_door_open() == TRUE) {
      digitalWrite(led_y, ON);
    } else {
      digitalWrite(led_y, OFF);
    }
    if (start_button_was_pressed() == TRUE && is_door_open() == FALSE) {
      state = ON_DOOR_CLOSED;
      Serial.println("State changed to: ON_DOOR_CLOSED");
      return;
    }
    tone(buzzer, 880);
    delay(TONE_DURATION);
    if (is_door_open() == TRUE) {
      digitalWrite(led_y, ON);
    } else {
      digitalWrite(led_y, OFF);
    }
    noTone(buzzer);
    delay(INTERVAL_BETWEEN_TONES);
    if (start_button_was_pressed() == TRUE && is_door_open() == FALSE) {
      state = ON_DOOR_CLOSED;
      Serial.println("State changed to: ON_DOOR_CLOSED");
      return;
    }
  }
}

void setup() {

  Serial.begin(115200);

  /* Wi-fi Connection */

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi... \n");
  while (WiFi.status() != WL_CONNECTED) {
    delay(2000);
    Serial.print("Connecting to Wi-Fi... \n");
  }
  Serial.println("Connected to Wi-Fi !");
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  /* Firebase Connection */

  Serial.printf("Firebase Client v%s\n\n", FIREBASE_CLIENT_VERSION);
  config.database_url = DATABASE_URL;
  config.signer.tokens.legacy_token = DATABASE_SECRET;
  Firebase.reconnectWiFi(true);
  Firebase.begin(&config, &auth);

  /* Reseting Database to Default Values */

  Firebase.setInt(fbdo, "timer", 0);
  Firebase.setInt(fbdo, "door_is_open", FALSE);
  Firebase.setInt(fbdo, "start_button_was_pressed", FALSE);
  Firebase.setInt(fbdo, "stop_button_was_pressed", FALSE);
  Firebase.setInt(fbdo, "power", MAX_POWER);

  /* Pins */

  pinMode(buzzer, OUTPUT);
  pinMode(led_r, OUTPUT);
  pinMode(led_g, OUTPUT);
  pinMode(led_y, OUTPUT);
  pinMode(button, INPUT_PULLUP);

  Serial.println("Project Started!");
}

void loop() {

  switch (state) {
  case ON_DOOR_CLOSED:

    digitalWrite(led_y, ON);
    digitalWrite(led_r, power);
    digitalWrite(led_g, ON);

    time_now = time_past - millis();

    if (time_now >= 1000) {
      time_past = millis();
      update_timer();
      update_power();
    }

    if (is_door_open() == TRUE) {
      state = PAUSED_DOOR_OPEN;
      Firebase.setIntAsync(fbdo, "door_is_open", TRUE);
      Serial.println("State changed to: PAUSED_DOOR_OPEN");
      break;
    }

    if (stop_button_was_pressed() == TRUE) {
      state = PAUSED_DOOR_CLOSED;
      Serial.println("State changed to: PAUSED_DOOR_CLOSED");
      break;
    }

    if (execution_finished() == TRUE) {
      state = OFF_DOOR_CLOSED;
      Serial.println("State changed to: OFF_DOOR_CLOSED");
      break;
    }

    break;

  case PAUSED_DOOR_OPEN:

    digitalWrite(led_y, ON);
    digitalWrite(led_r, OFF);
    digitalWrite(led_g, OFF);

    if (is_door_open() == FALSE) {
      state = PAUSED_DOOR_CLOSED;
      Firebase.setIntAsync(fbdo, "door_is_open", FALSE);
      Serial.println("State changed to: PAUSED_DOOR_CLOSED");
      break;
    }

    if (stop_button_was_pressed() == TRUE) {
      state = OFF_DOOR_OPEN;
      Firebase.setIntAsync(fbdo, "timer", 0);
      Serial.println("State changed to: OFF_DOOR_OPEN");
      break;
    }

    break;

  case PAUSED_DOOR_CLOSED:

    digitalWrite(led_y, ON);
    digitalWrite(led_r, OFF);
    digitalWrite(led_g, OFF);

    if (is_door_open() == TRUE) {
      state = PAUSED_DOOR_OPEN;
      Firebase.setIntAsync(fbdo, "door_is_open", TRUE);
      Serial.println("State changed to: PAUSED_DOOR_OPEN");
      break;
    }

    if (stop_button_was_pressed() == TRUE) {
      state = OFF_DOOR_CLOSED;
      Firebase.setIntAsync(fbdo, "timer", 0);
      Serial.println("State changed to: OFF_DOOR_CLOSED");
      break;
    }

    if (start_button_was_pressed() == TRUE) {
      state = ON_DOOR_CLOSED;
      Serial.println("State changed to: ON_DOOR_CLOSED");
      break;
    }

    break;

  case OFF_DOOR_OPEN:

    digitalWrite(led_y, ON);
    digitalWrite(led_r, OFF);
    digitalWrite(led_g, OFF);

    if (is_door_open() == FALSE) {
      state = OFF_DOOR_CLOSED;
      Firebase.setIntAsync(fbdo, "door_is_open", FALSE);
      Serial.println("State changed to: OFF_DOOR_CLOSED");
      break;
    }

    break;

  case OFF_DOOR_CLOSED:

    digitalWrite(led_y, OFF);
    digitalWrite(led_r, OFF);
    digitalWrite(led_g, OFF);

    if (execution_has_finished == TRUE) {
      ring_buzzer();
    }

    if (is_door_open() == TRUE) {
      state = OFF_DOOR_OPEN;
      Firebase.setIntAsync(fbdo, "door_is_open", TRUE);
      Serial.println("State changed to: OFF_DOOR_OPEN");
      break;
    }

    if (start_button_was_pressed() == TRUE) {
      state = ON_DOOR_CLOSED;
      Serial.println("State changed to: ON_DOOR_CLOSED");
      break;
    }

    break;
  }
}