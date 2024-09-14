//#include <Arduino.h>
#include <Adafruit_Fingerprint.h>

#include "defines.h"
#include "led_interrupt.h"

SoftwareSerial mySerial(SERIAL_PINS);
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);

int get_finger();
void scan_fingerprint();
bool try_add_new_finger(const int &id);
void(* resetFunc) (void) = 0;

void setup()
{
    delay(1000);

    // add comment why this number here 
    Serial.begin(115200);

    pinMode(LED_RED, OUTPUT);
    pinMode(LED_GREEN, OUTPUT);
    pinMode(LED_BLUE, OUTPUT);
    pinMode(BUTTON_PIN, INPUT_PULLUP);
    delay(200);
    
    // When waiting for conneciton then light red
    update_led_status(HOLD, RED, 200);
    //update_led_status(FADE, WHITE, 400);
    PRINTLN("[setup] Establishing connection to fingerprint sensor...");

    // Why this number and not some define
    finger.begin(57600);
    delay(200);         //TODO: WAITING LIBRARY
    if (finger.verifyPassword())
    {
        update_led_status(HOLD, GREEN, 300);
        PRINTLN("[setup] Sensor connected!");
        delay(350);
    } else {
        PRINTLN("[setup] FINGERPRINT SENSOR CONNECTION ERROR");
        for (;;) {
            update_led_status(HOLD, RED, 2000);
            delay(2500);
        }
    }
}

void loop()
{
    if (!digitalRead(BUTTON_PIN))
    {
      PRINTLN("[loop] Key inserted");
      for (;;)
      {
          add_fingerprint();
      }
      PRINTLN("[loop] Fingerprint added");
    } else {
      PRINTLN("[loop] Key not inserted");
      scan_fingerprint();
      delay(400);
    }
}

int get_finger() {
  PRINTLN("[get_finger] Searching for empty slot");
  // why those numbers for for loop
  for (int i = 50; i < 128; ++i) {
      uint8_t status_code = finger.loadModel(i);
      switch (status_code) {
        case FINGERPRINT_OK:
          break;
        case FINGERPRINT_PACKETRECIEVEERR:
          break;
        default:
          PRINT("[get_finger] Empty slot found #");
          PRINTLN(i);
          return i;
      }
    }
    PRINTLN("[get_finger] Error empty slot has not been found");
    return 0;
}

void add_fingerprint() {
    //TODO macros for update_led_status
    update_led_status(HOLD, BLUE, 1000);
    delay(900);
    int index = get_finger();
    if(try_add_new_finger(index))
    {
      PRINTLN("[add_fingerprint] Resseting via function pointer");
      resetFunc();
    }
}

bool try_add_new_finger(const int &id) {
  // Why -1? Define or something
  int status_code = -1;
  update_led_status(BLINK, WHITE, 1000000);
  PRINTLN("[try_add_new_finger] Put finger on sensor");
  while (status_code != FINGERPRINT_OK)
  {
    status_code = finger.getImage();
    switch (status_code) {
    case FINGERPRINT_OK:
      update_led_status(HOLD, GREEN, 2000);
      PRINTLN("[try_add_new_finger] Finger found");
      break;
    default:
      return false;
    }
  }
  delay(2000);
  status_code = finger.image2Tz(1);
  if (status_code != FINGERPRINT_OK)
  {
    PRINTLN("[try_add_new_finger] image2Tz failed");
    return false;
  }

  status_code = 0;
  update_led_status(BLINK, WHITE, 1000000);
  PRINTLN("[try_add_new_finger] Put finger again");
  while (status_code != FINGERPRINT_NOFINGER)
  {
    status_code = finger.getImage();
  }

  status_code = -1;
  while (status_code != FINGERPRINT_OK)
  {
    status_code = finger.getImage();
    if(status_code == FINGERPRINT_OK)
        update_led_status(HOLD, GREEN, 2000);
  }
  PRINTLN("[try_add_new_finger] Finger found");
  delay(2000);

  status_code = finger.image2Tz(2);
  if (status_code != FINGERPRINT_OK)
  {
    PRINTLN("[try_add_new_finger] image2Tz failed");
    return false;
  }

  status_code = finger.createModel();
  if (status_code != FINGERPRINT_OK)
  {
    return false;
  }
  PRINTLN("[try_add_new_finger] Model created");
  status_code = finger.storeModel(id);
  if (status_code != FINGERPRINT_OK)
  {
    return false;
  }
  PRINTLN("[try_add_new_finger] Finger model stored");
  return true;
}

void scan_fingerprint() {
  uint8_t status_code = finger.getImage();
  PRINTLN("[scan_fingerprint] Checking if finger on sensor");
  if (status_code != FINGERPRINT_OK)
  {
    return;
  }
  PRINTLN("[scan_fingerprint] Finger found");
  status_code = finger.image2Tz();
  if (status_code != FINGERPRINT_OK)
  {
    PRINTLN("[scan_fingerprint] image2Tz failed");
    return;
  }

  status_code = finger.fingerSearch();
  if (status_code != FINGERPRINT_OK)
  {
    PRINTLN("[scan_fingerprint] Finger has not been found in database");
    return;
  }
  PRINTLN("[scan_fingerprint] Finger has been found in database");
  PRINTLN("[scan_fingerprint] Relay open");
  digitalWrite(BUTTON_PIN, LOW);
  delay(2000);
  PRINTLN("[scan_fingerprint] Relay closed");
  digitalWrite(BUTTON_PIN, HIGH);
}