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

int led_r = D0;
int led_y = D2;
int led_g = D1;
int buzzer = D3;
int button = D4;

int state = OFF_DOOR_CLOSED;
int timer = 0;
int power;

int is_door_open(){
  return digitalRead(button);
}

int stop_button_was_pressed(){
  // TODO: if the variable in the database that tells if the stop button was pressed is true
  // set that variable in the database as false
  // wait that variable to be false
  // return true
  return false;
}

int start_button_was_pressed(){

  // TODO: if the variable in the database that tells if the start button was pressed is true
  // set that variable in the database as false
  // wait that variable to be false
  // return true

  // TODO: Set the timer variable as equal to the value in the database
  return false;
}

int execution_finished(){
  if (timer == 0) {
    return true;
  }
  return false;
}

void update_timer(){
  //TODO: Decrease the "timer" variable here and update the database
  if (timer > 0){
    timer = timer - 1;
  }
}

void update_power(){
  //TODO: Set the "power" variable to be the same as in the database
}

void buzz_one_time(){
  //TODO: Buzz one time 
}

void buzz_execution_finished(){
  //TODO: Buzz a few times to represent the execution finished
}

void setup() {

    Serial.begin(115200);

    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    Serial.print("Connecting to Wi-Fi");
    while (WiFi.status() != WL_CONNECTED)
    {
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
  pinMode(button, INPUT_PULLUP);

  /* Firebase variables */

  Firebase.setInt(fbdo, "state", 0);
  Firebase.setInt(fbdo, "time", 0);

  Serial.println("Start!");
}

void loop() {

  switch (state) 
  {
    case ON_DOOR_CLOSED:

      digitalWrite(led_y, ON);
      digitalWrite(led_r, ON);
      digitalWrite(led_g, ON);

      update_timer();
      update_power();

      if (is_door_open() == true) {
        state = PAUSED_DOOR_OPEN;
        Serial.println("State changed to: PAUSED_DOOR_OPEN");
      }

      if (stop_button_was_pressed() == true) {
        state = PAUSED_DOOR_CLOSED;
        buzz_one_time();
        Serial.println("State changed to: PAUSED_DOOR_CLOSED");
      }

      if (execution_finished() == true) {
        state = OFF_DOOR_CLOSED;
        buzz_execution_finished();
        Serial.println("State changed to: OFF_DOOR_CLOSED");
      }

      break;

    case PAUSED_DOOR_OPEN:

      digitalWrite(led_y, ON);
      digitalWrite(led_r, OFF);
      digitalWrite(led_g, OFF);

      if (is_door_open() == false) {
        state = PAUSED_DOOR_CLOSED;
        Serial.println("State changed to: PAUSED_DOOR_CLOSED");
      }

      if (stop_button_was_pressed() == true) {
        state == OFF_DOOR_OPEN;
        Serial.println("State changed to: OFF_DOOR_OPEN");
      }

      break;

    case PAUSED_DOOR_CLOSED:

      digitalWrite(led_y, ON);
      digitalWrite(led_r, OFF);
      digitalWrite(led_g, OFF);

      if (is_door_open() == true) {
        state = PAUSED_DOOR_OPEN;
        Serial.println("State changed to: PAUSED_DOOR_OPEN");
      }

      if (stop_button_was_pressed() == true) {
        state == OFF_DOOR_CLOSED;
        Serial.println("State changed to: OFF_DOOR_CLOSED");
      }

      if (start_button_was_pressed() == true) {
        state == ON_DOOR_CLOSED;
        Serial.println("State changed to: ON_DOOR_CLOSED");
      }

      break;

    case OFF_DOOR_OPEN:

      digitalWrite(led_y, ON);
      digitalWrite(led_r, OFF);
      digitalWrite(led_g, OFF);

      if (is_door_open() == false) {
        state = OFF_DOOR_CLOSED;
        Serial.println("State changed to: OFF_DOOR_CLOSED");
      }

      break;
    
    case OFF_DOOR_CLOSED:

      digitalWrite(led_y, OFF);
      digitalWrite(led_r, OFF);
      digitalWrite(led_g, OFF);

      if (is_door_open() == true) {
        state = OFF_DOOR_OPEN;
        Serial.println("State changed to: OFF_DOOR_OPEN");
      }

      if (start_button_was_pressed() == true) {
        state == ON_DOOR_CLOSED;
        Serial.println("State changed to: ON_DOOR_CLOSED");
      }

      break;
  }
}