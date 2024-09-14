#include <WiFi.h>
#include <ESPmDNS.h>
#include <WebServer.h>
#include <Adafruit_Fingerprint.h>
#include <HardwareSerial.h>
//#include <softwareSerial.h> // add new version 

#include "configwifi.h"
#include "configlogin.h"

// Inicjalizacja serwera HTTP na porcie 80
WebServer server(80);

// Pin dla przekaźnika
const int relayPin = 10;  //D34 

// Inicjalizacja czujnika linii papilarnych
HardwareSerial mySerial(1);
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);

// Tablica dla nazw użytkowników
String userNames[100];  // Tablica na 100 odcisków palców

// Zmienna do przechowywania stanu logowania
bool isLoggedIn = false;

void setup() {
  Serial.begin(57600);

  // Ustawienie pinu przekaźnika jako wyjście
  pinMode(relayPin, OUTPUT);
  digitalWrite(relayPin, LOW);

  // Inicjalizacja WiFi
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting...");
  }
  Serial.println("Connected to Wi-Fi!");

  if (!MDNS.begin("esp32")) {
    Serial.println("Error setting up MDNS responder!");
    while (1) {
      delay(1000);
    }
  }
  Serial.println("mDNS responder started");

  // Inicjalizacja serwera
  server.on("/", HTTP_GET, handleRoot);
  server.on("/login", HTTP_GET, handleLogin);
  server.on("/login", HTTP_POST, handleLoginPost);
  server.on("/add", HTTP_POST, handleAdd);
  server.on("/remove", HTTP_POST, handleRemove);
  server.on("/block", HTTP_POST, handleBlock);
  server.on("/status", HTTP_GET, handleStatus);
  server.on("/logs", HTTP_GET, handleLogs);
  server.on("/unlock", HTTP_POST, handleUnlock);
  server.on("/list", HTTP_GET, handleList);

  MDNS.addService("http", "tcp", 80);
  server.begin();
  Serial.println("HTTP server started");

  //Inicjalizacja czujnika linii papilarnych
  mySerial.begin(57600, SERIAL_8N1, 40, 41);  // RX, TX pin (dostosuj w zależności od połączenia).    blue == txd0
  finger.begin(57600);

  // if (finger.verifyPassword()) {
  //   Serial.println("Found fingerprint sensor!");
  // } else {
  //   Serial.println("Fingerprint sensor not found!");
  //   while (1) { delay(1); }
  // }
}

void handleRoot() {
  if (!isLoggedIn) {
    server.sendHeader("Location", "/login");
    server.send(302, "text/plain", "");
    return;
  }

  String html = R"rawliteral(
    <!DOCTYPE html>
    <html>
    <head>
      <style>
        body {
          background-color: #333;
          color: #fff;
          font-family: Arial, sans-serif;
          text-align: center;
        }
        h2 {
          color: #4CAF50;
        }
        .container {
          display: flex;
          flex-direction: column;
          align-items: center;
          justify-content: center;
          height: 100vh;
        }
        form {
          margin: 10px;
        }
        input[type="submit"] {
          background-color: #4CAF50;
          border: none;
          color: white;
          padding: 15px 32px;
          text-align: center;
          text-decoration: none;
          display: inline-block;
          font-size: 16px;
          margin: 4px 2px;
          cursor: pointer;
          border-radius: 5px;
          transition: background-color 0.3s;
          width: 120%
        }
        input[type="submit"]:hover {
          background-color: #45a049;
        }
      </style>
    </head>
    <body>
      <div class="container">
        <h2>Fingerprint Control Panel</h2>
        <form action="/logs" method="get">
          <input type="submit" value="View Logs">
        </form>
        <form action="/status" method="get">
          <input type="submit" value="View Status">
        </form>
        <form action="/" method="post">
          <input type="submit" value="Check Fingerprint">
        </form>
        <form action="/add" method="post">
          <input type="submit" value="Add Fingerprint">
        </form>
        <form action="/remove" method="post">
          <input type="submit" value="Remove Fingerprint">
        </form>
        <form action="/block" method="post">
          <input type="submit" value="Block Fingerprint">
        </form>
        <form action="/unlock" method="post">
          <input type="submit" value="Unlock Fingerprint">
        </form>
        <form action="/list" method="get">
          <input type="submit" value="List Fingerprints">
        </form>
      </div>
    </body>
    </html>
  )rawliteral";
  server.send(200, "text/html", html);
}

void handleLogin() {
  String html = R"rawliteral(
    <!DOCTYPE html>
    <html>
    <head>
      <style>
        body {
          background-color: #333;
          color: #fff;
          font-family: Arial, sans-serif;
          text-align: center;
        }
        h2 {
          color: #4CAF50;
        }
        .container {
          display: flex;
          flex-direction: column;
          align-items: center;
          justify-content: center;
          height: 100vh;
        }
        form {
          margin: 10px;
        }
        label, input {
          display: block;
          margin: 10px 0;
        }
        input[type="text"],
        input[type="password"] {
          padding: 10px;
          font-size: 16px;
          border-radius: 5px;
          border: 1px solid #ccc;
        }
        input[type="submit"] {
          padding: 10px;
          font-size: 16px;
          border-radius: 5px;
          border: none;
          background-color: #4CAF50;
          color: white;
          cursor: pointer;
          transition: background-color 0.1s;
          margin: auto;
        }
        input[type="submit"]:hover {
          background-color: #45a049;
        }
      </style>
    </head>
    <body>
      <div class="container">
        <h2>Login</h2>
        <form action="/login" method="post">
          <label for="username">Username:</label>
          <input type="text" id="username" name="username" required><br>
          <label for="password">Password:</label>
          <input type="password" id="password" name="password" required><br>
          <input type="submit" value="Login">
        </form>
      </div>
    </body>
    </html>
  )rawliteral";
  server.send(200, "text/html", html);
}

// Funkcja obsługująca logowanie (POST)
void handleLoginPost() {
  String username = server.arg("username");
  String password = server.arg("password");

  if (username == LOGIN_USERNAME && password == LOGIN_PASSWORD) {
    isLoggedIn = true;
    server.sendHeader("Location", "/");
    server.send(302, "text/plain", "");
  } else {
    server.send(401, "text/html", "Unauthorized: Incorrect credentials");
  }
}

void handleAdd() {
  if (!isLoggedIn) {
    server.sendHeader("Location", "/login");
    server.send(302, "text/plain", "");
    return;
  }

  if (server.method() == HTTP_POST) {
    // Odczytaj dane z formularza
    String userName = server.arg("username");
    int userID = server.arg("id").toInt();

    // Sprawdź, czy ID jest prawidłowe
    if (userID < 0 || userID >= 100) {
      server.send(400, "text/html", "Invalid ID. Must be between 0 and 99.");
      return;
    }

    int id = enrollFingerprint();
    if (id >= 0) {
      userNames[userID] = userName;  // Przypisz nazwę użytkownika do podanego ID
      Serial.println("Fingerprint added with ID: " + String(userID));
      server.send(200, "text/html", "Fingerprint added successfully with ID: " + String(userID));
    } else {
      server.send(500, "text/html", "Failed to add fingerprint.");
    }
  } else {
    String html = R"rawliteral(
      <!DOCTYPE html>
      <html>
      <head>
        <style>
          body {
            background-color: #333;
            color: #fff;
            font-family: Arial, sans-serif;
            text-align: center;
          }
          h2 {
            color: #4CAF50;
          }
          .container {
            display: flex;
            flex-direction: column;
            align-items: center;
            justify-content: center;
            height: 100vh;
          }
          form {
            margin: 10px;
          }
          input[type="text"],
          input[type="number"],
          input[type="submit"] {
            padding: 10px;
            font-size: 16px;
            border-radius: 5px;
            border: 1px solid #ccc;
          }
          input[type="submit"] {
            background-color: #4CAF50;
            color: white;
            border: none;
            cursor: pointer;
            transition: background-color 0.3s;
          }
          input[type="submit"]:hover {
            background-color: #45a049;
          }
        </style>
      </head>
      <body>
        <div class="container">
          <h2>Add Fingerprint</h2>
          <form action="/add" method="post">
            <label for="id">ID:</label>
            <input type="number" id="id" name="id" min="0" max="99" required><br>
            <label for="username">Username:</label>
            <input type="text" id="username" name="username" required><br>
            <input type="submit" value="Start Enrollment">
          </form>
        </div>
      </body>
      </html>
    )rawliteral";
    server.send(200, "text/html", html);
  }
}

// Funkcja wyświetlająca listę odcisków palców
void handleList() {
  if (!isLoggedIn) {
    server.sendHeader("Location", "/login");
    server.send(302, "text/plain", "");
    return;
  }

  String html = "<!DOCTYPE html><html><body><h2>List of Fingerprints</h2><ul>";

  for (int i = 0; i < 100; i++) {
    if (userNames[i] != "") {
      html += "<li>ID " + String(i) + ": " + userNames[i] + "</li>";
    }
  }

  html += "</ul></body></html>";
  server.send(200, "text/html", html);
}

// Funkcja usuwania odcisku palca
void handleRemove() {
  if (!isLoggedIn) {
    server.sendHeader("Location", "/login");
    server.send(302, "text/plain", "");
    return;
  }

  String html = R"rawliteral(
    <!DOCTYPE html>
    <html>
    <body>
    <h2>Remove Fingerprint</h2>
    <form action="/remove" method="post">
      <input type="number" name="id" min="0" max="99" placeholder="ID to Remove">
      <input type="submit" value="Remove">
    </form>
    </body>
    </html>
  )rawliteral";
  server.send(200, "text/html", html);

  // Tu możesz dodać kod do usuwania odcisku palca na podstawie ID
}

// Funkcja blokowania odcisku palca
void handleBlock() {
  if (!isLoggedIn) {
    server.sendHeader("Location", "/login");
    server.send(302, "text/plain", "");
    return;
  }

  String html = R"rawliteral(
    <!DOCTYPE html>
    <html>
    <body>
    <h2>Block Fingerprint</h2>
    <form action="/block" method="post">
      <input type="number" name="id" min="0" max="99" placeholder="ID to Block">
      <input type="submit" value="Block">
    </form>
    </body>
    </html>
  )rawliteral";
  server.send(200, "text/html", html);

  // Tu możesz dodać kod do blokowania odcisku palca na podstawie ID
}

// Funkcja wyświetlająca status
void handleStatus() {
  if (!isLoggedIn) {
    server.sendHeader("Location", "/login");
    server.send(302, "text/plain", "");
    return;
  }

  String status = "Status:\n";
  for (int i = 0; i < 100; i++) {
    if (userNames[i] != "") {
      status += "ID " + String(i) + ": " + userNames[i] + "\n";
    }
  }
  server.send(200, "text/plain", status);
}

// Funkcja obsługująca żądanie logów
void handleLogs() {
  if (!isLoggedIn) {
    server.sendHeader("Location", "/login");
    server.send(302, "text/plain", "");
    return;
  }

  String logData = "Here are the log messages:\n";
  logData += "Last action: Fingerprint added\n"; // Przykładowe logi
  // Możesz tutaj dodać więcej danych logujących

  server.send(200, "text/plain", logData);
}

void handleUnlock() {
  if (!isLoggedIn) {
    server.sendHeader("Location", "/login");
    server.send(302, "text/plain", "");
    return;
  }

  if (server.method() == HTTP_POST) {
    // Odczytaj ID z formularza
    int userID = server.arg("id").toInt();

    // Sprawdź, czy ID jest prawidłowe
    if (userID < 0 || userID >= 100) {
      server.send(400, "text/html", "Invalid ID. Must be between 0 and 99.");
      return;
    }

    // Logika odblokowywania odcisku palca (tu możesz dodać rzeczywistą logikę, jeśli to konieczne)
    Serial.println("Unlocking fingerprint with ID: " + String(userID));
    server.send(200, "text/html", "Fingerprint with ID " + String(userID) + " unlocked.");
  } else {
    String html = R"rawliteral(
      <!DOCTYPE html>
      <html>
      <head>
        <style>
          body {
            background-color: #333;
            color: #fff;
            font-family: Arial, sans-serif;
            text-align: center;
          }
          h2 {
            color: #4CAF50;
          }
          .container {
            display: flex;
            flex-direction: column;
            align-items: center;
            justify-content: center;
            height: 100vh;
          }
          form {
            margin: 10px;
          }
          input[type="number"],
          input[type="submit"] {
            padding: 10px;
            font-size: 16px;
            border-radius: 5px;
            border: 1px solid #ccc;
          }
          input[type="submit"] {
            background-color: #4CAF50;
            color: white;
            border: none;
            cursor: pointer;
            transition: background-color 0.3s;
          }
          input[type="submit"]:hover {
            background-color: #45a049;
          }
        </style>
      </head>
      <body>
        <div class="container">
          <h2>Unlock Fingerprint</h2>
          <form action="/unlock" method="post">
            <label for="id">ID:</label>
            <input type="number" id="id" name="id" min="0" max="99" required><br>
            <input type="submit" value="Unlock">
          </form>
        </div>
      </body>
      </html>
    )rawliteral";
    server.send(200, "text/html", html);
  }
}

// Funkcja do rejestrowania odcisku palca
int enrollFingerprint() {
  int p = -1;
  Serial.println("Place your finger on the sensor.");
  
  while (p == -1) {
    int id = finger.getImage();
    if (id == FINGERPRINT_OK) {
      Serial.println("Image taken.");
      id = finger.image2Tz();
      if (id == FINGERPRINT_OK) {
        Serial.println("Image converted.");
        id = finger.createModel();
        if (id == FINGERPRINT_OK) {
          Serial.println("Model created.");
          uint16_t fingerprintID = 1;  // Przykładowe ID, dostosuj jak potrzebujesz
          id = finger.storeModel(fingerprintID);
          if (id == FINGERPRINT_OK) {
            Serial.println("Model stored.");
            return fingerprintID;
          } else {
            Serial.println("Failed to store model.");
            return -1;
          }
        } else {
          Serial.println("Failed to create model.");
          return -1;
        }
      } else {
        Serial.println("Failed to convert image.");
        return -1;
      }
    } else {
      Serial.println("Failed to take image.");
      return -1;
    }
  }
  return -1;  // Zwraca -1, jeśli operacja nie zakończyła się sukcesem
}

// Funkcja do odblokowywania odcisku palca
int unlockFingerprint() {
  Serial.println("Place your finger on the sensor to unlock.");
  
  int id = -1;
  while (id == -1) {
    int result = finger.getImage();
    if (result == FINGERPRINT_OK) {
      Serial.println("Image taken.");
      result = finger.image2Tz();
      if (result == FINGERPRINT_OK) {
        Serial.println("Image converted.");
        result = finger.fingerSearch();
        if (result == FINGERPRINT_OK) {
          id = finger.fingerID; // ID odcisku palca
          Serial.println("Fingerprint matched with ID: " + String(id));
          return id; // Odcisk odblokowany, zwróć ID
        } else {
          Serial.println("Fingerprint not found.");
        }
      } else {
        Serial.println("Failed to convert image.");
      }
    } else {
      Serial.println("Failed to take image.");
    }
  }
  return -1; // Zwraca -1, jeśli operacja nie zakończyła się sukcesem
}

void loop() {
  server.handleClient();
}