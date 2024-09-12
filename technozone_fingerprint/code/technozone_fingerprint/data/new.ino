#include <WiFi.h>
#include <ESPmDNS.h>
#include <WebServer.h>
// #include <Adafruit_Fingerprint.h> // Tymczasowo wykomentowane
// #include <HardwareSerial.h>       // Tymczasowo wykomentowane
#include "configwifi.h"
#include "configlogin.h"

// Inicjalizacja serwera HTTP na porcie 80
WebServer server(80);

// Pin dla przekaźnika
const int relayPin = 5;

// Inicjalizacja czujnika linii papilarnych
// HardwareSerial mySerial(1);
// Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);

// Tablica dla nazw użytkowników
String userNames[100];  // Tablica na 100 odcisków palców

// Zmienna do przechowywania stanu logowania
bool isLoggedIn = false;

void setup() {
  Serial.begin(115200);

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

  MDNS.addService("http", "tcp", 80);
  server.begin();
  Serial.println("HTTP server started");

  // Inicjalizacja czujnika linii papilarnych
  // mySerial.begin(57600, SERIAL_8N1, 16, 17);  // RX, TX pin (dostosuj w zależności od połączenia)
  // finger.begin(57600);

  // Tymczasowo sprawdzenie, czy czujnik jest prawidłowo skonfigurowany
  // if (finger.verifyPassword()) {
  //   Serial.println("Found fingerprint sensor!");
  // } else {
  //   Serial.println("Fingerprint sensor not found!");
  //   while (1) { delay(1); }
  // }
}

// Funkcja obsługująca żądania GET na stronie głównej
void handleRoot() {
  if (!isLoggedIn) {
    server.sendHeader("Location", "/login");
    server.send(302, "text/plain", "");
    return;
  }

  String html = R"rawliteral(
    <!DOCTYPE html>
    <html>
    <body>
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
    <form action="/status" method="get">
      <input type="submit" value="View Status">
    </form>
    </body>
    </html>
  )rawliteral";
  server.send(200, "text/html", html);
}

// Funkcja obsługująca stronę logowania
void handleLogin() {
  String html = R"rawliteral(
    <!DOCTYPE html>
    <html>
    <body>
    <h2>Login</h2>
    <form action="/login" method="post">
      <label for="username">Username:</label><br>
      <input type="text" id="username" name="username"><br>
      <label for="password">Password:</label><br>
      <input type="password" id="password" name="password"><br><br>
      <input type="submit" value="Login">
    </form>
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

// Funkcja dodawania odcisku palca
void handleAdd() {
  if (!isLoggedIn) {
    server.sendHeader("Location", "/login");
    server.send(302, "text/plain", "");
    return;
  }

  String html = R"rawliteral(
    <!DOCTYPE html>
    <html>
    <body>
    <h2>Add Fingerprint</h2>
    <form action="/add" method="post">
      <input type="submit" value="Start Enrollment">
    </form>
    </body>
    </html>
  )rawliteral";
  server.send(200, "text/html", html);

  // Tymczasowo komentujemy wywołanie funkcji enrollFingerprint
  // int id = enrollFingerprint();
  // if (id >= 0) {
  //   userNames[id] = "User" + String(id);  // Przypisz nazwę użytkownika, np. "User1"
  //   Serial.println("Fingerprint added with ID: " + String(id));
  // } else {
  //   Serial.println("Failed to add fingerprint.");
  // }
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

void loop() {
  server.handleClient();
}