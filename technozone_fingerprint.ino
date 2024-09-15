#include <SoftwareSerial.h>
#include <Adafruit_Fingerprint.h>
#include <Arduino.h>

// pin #2 is IN from sensor (GREEN wire)
// pin #3 is OUT from Arduino (WHITE wire)
SoftwareSerial mySerial(2, 3);
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);

const int unlockPin = 7;  // Pin to control the lock (e.g., relay)
String userNames[64]; 

void setup() {
  Serial.begin(9600);
  Serial.println("Fingerprint sensor ready");

  pinMode(unlockPin, OUTPUT);
  digitalWrite(unlockPin, LOW);  // Zamek zamknięty

  // Ustawienie prędkości dla czujnika
  finger.begin(57600);

  if (finger.verifyPassword()) {
    Serial.println("Found fingerprint sensor!");
  } else {
    Serial.println("Did not find fingerprint sensor :(");
    while (1);  // Zatrzymaj program, jeśli czujnik nie zostanie znaleziony
  }

  Serial.println("Waiting for valid finger...");
}

void loop() {
  // Sprawdzaj cały czas odcisk palca
  int fingerprintID = getFingerprintIDez();
  if (fingerprintID != -1) {
    Serial.print("Access granted for ID #");
    Serial.println(fingerprintID);

    // Otwórz zamek
    digitalWrite(unlockPin, HIGH);  // Zamek otwarty
    delay(5000);  // Otwórz na 5 sekund
    digitalWrite(unlockPin, LOW);   // Zamek zamknięty
  }

  // Sprawdź polecenia wprowadzone w Serial Monitor
  if (Serial.available()) {
    String command = Serial.readStringUntil('\n');

    if (command.startsWith("add")) {
      addFingerprint();
    } else if (command.startsWith("print")) {
      printFingerprintDatabase();
    } else if (command.startsWith("remove")) {
      int idToRemove = command.substring(7).toInt();
      removeFingerprint(idToRemove);
    }
  }

  delay(50);
}

// Funkcja do pobierania ID odcisku
int getFingerprintIDez() {
  uint8_t p = finger.getImage();
  if (p != FINGERPRINT_OK)  return -1;

  p = finger.image2Tz();
  if (p != FINGERPRINT_OK)  return -1;

  p = finger.fingerFastSearch();
  if (p != FINGERPRINT_OK)  return -1;

  // Znaleziono pasujący odcisk palca
  return finger.fingerID;
}

// Funkcja dodająca nowy odcisk palca
void addFingerprint() {
  int id = findEmptyID();  // Znajdź najmniejszy dostępny ID większy od 0
  if (id == -1) {
    Serial.println("No available space to add new fingerprint.");
    return;
  }

  Serial.print("Placing finger to register for ID #");
  Serial.println(id);

  while (getFingerprintEnroll(id) != FINGERPRINT_OK);

  // nowe moze zepsuje kod 

  //   // Po poprawnym dodaniu odcisku, zapytaj o nazwę użytkownika
  Serial.println("Fingerprint added successfully!");
  Serial.print("Enter user name for ID #");
  Serial.println(id);

    // Czekaj na wpisanie nazwy użytkownika
  while (!Serial.available());
  userNames[id] = Serial.readStringUntil('\n');
  Serial.print("User name '");
  Serial.print(userNames[id]);
  Serial.println("' saved.");
}

// Funkcja rejestracji nowego odcisku palca
uint8_t getFingerprintEnroll(int id) {
  int p;
  Serial.println("Place finger on the sensor");

  // Pobierz pierwszy obraz
  while ((p = finger.getImage()) != FINGERPRINT_OK) {
    delay(1000);
  }

  // Konwertuj obraz na zestaw cech
  p = finger.image2Tz(1);
  if (p != FINGERPRINT_OK) return p;

  Serial.println("Remove finger");
  delay(2000);

  Serial.println("Place the same finger again");

  // Pobierz drugi obraz i przetwórz
  while ((p = finger.getImage()) != FINGERPRINT_OK) {
    delay(1000);
  }

  // Konwertuj obraz na zestaw cech (2)
  p = finger.image2Tz(2);
  if (p != FINGERPRINT_OK) return p;

  // Tworzenie modelu odcisku palca
  p = finger.createModel();
  if (p != FINGERPRINT_OK) return p;

  // Zapisz odcisk palca pod danym ID
  p = finger.storeModel(id);
  if (p == FINGERPRINT_OK) {
    Serial.println("Fingerprint stored successfully!");
  } else {
    Serial.println("Error storing fingerprint.");
  }

  return p;
}

// Funkcja znajdująca najmniejszy dostępny ID
int findEmptyID() {
  for (int id = 1; id < 127; id++) {  // Zakres ID w zależności od czujnika
    if (finger.loadModel(id) != FINGERPRINT_OK) {
      return id;  // Znaleziono wolny ID
    }
  }
  return -1;  // Brak wolnych miejsc
}

// Funkcja wyświetlająca bazę odcisków
void printFingerprintDatabase() {
  Serial.println("Fingerprint database:");

  for (int id = 1; id < 127; id++) {
    if (finger.loadModel(id) == FINGERPRINT_OK) {
      Serial.print("ID #");
      Serial.println(id);
      // nowe moze zepsuje kod 
        if (userNames[id] != "") {
        Serial.print(" (User: ");
        Serial.print(userNames[id]);
        Serial.println(")");
      } else {
        Serial.println();
      }
    }
  }
}

// Funkcja usuwająca odcisk po ID
void removeFingerprint(int id) {
  if (id < 1 || id > 127) {
    Serial.println("Invalid ID");
    return;
  }

  if (finger.deleteModel(id) == FINGERPRINT_OK) {
    Serial.print("Successfully deleted fingerprint with ID #");
    Serial.println(id);
    userNames[id] = ""; 
  } else {
    Serial.println("Failed to delete fingerprint.");
  }
}