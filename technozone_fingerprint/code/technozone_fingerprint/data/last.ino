#include <WiFi.h>
#include <ESPmDNS.h>
#include <WebServer.h>

#include "configwifi.h"   // file with info to wifi connected 
#include "configlogin.h"   // file with a login and password to log in to the page 

WebServer server(80);   // create serwer on port 80

// Function to login to the main page 
void handleRoot() {
  if (!server.hasArg("username") || !server.hasArg("password")) {
    // If login data is empty send the info to login 
    String loginPage = R"rawliteral(
      <!DOCTYPE html>
      <html>
      <body>
      // ToDo: chage log in and password mutherfucker if i dont do this kill me plis 
      <form action="/" method="post">
        <label for="username">Username:</label><br>
        <input type="text" id="username" name="username"><br>
        <label for="password">Password:</label><br>
        <input type="password" id="password" name="password"><br><br>
        <input type="submit" value="Submit">
      </form>
      </body>
      </html>
    )rawliteral";
    server.send(200, "text/html", loginPage);
  } else {
    // Sprawdź dane logowania
    String username = server.arg("username");
    String password = server.arg("password");
    if (username == LOGIN_USERNAME && password == LOGIN_PASSWORD) {
      // If data to login is correct send info 
      IPAddress ip = WiFi.localIP();
      String ipStr = String(ip[0]) + '.' + String(ip[1]) + '.' + String(ip[2]) + '.' + String(ip[3]);
      String content = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<!DOCTYPE HTML>\r\n<html>Hello from ESP32 at ";
      content += ipStr;
      content += "</html>\r\n\r\n";
      server.send(200, "text/html", content);
    } else {
      // If data to login is bad send this popup 
      server.send(401, "text/html", "Unauthorized: Incorrect credentials don't try again if you do this i will find you and kill");
    }
  }
}

void setup() {
  Serial.begin(115200);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Laczenie...");
  }
  Serial.println("Polaczone z Wi-Fi!");

  if (!MDNS.begin("esp32")) {
    Serial.println("Error setting up MDNS responder!");
    while (1) {
      delay(1000);
    }
  }
  Serial.println("mDNS responder started");

  server.on("/", HTTP_GET, handleRoot);
  server.on("/", HTTP_POST, handleRoot);

  MDNS.addService("http", "tcp", 80);

  server.begin();
  Serial.println("TCP server started");
}

void loop() {
  server.handleClient();
}