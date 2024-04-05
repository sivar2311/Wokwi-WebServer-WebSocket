/*
   This ESP32 code is created by esp32io.com

   This ESP32 code is released in the public domain

   For more detail (instruction and wiring diagram), visit https://esp32io.com/tutorials/esp32-led-matrix-via-web
*/

#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <WebSocketsServer.h>

#include "index.h"

#include <MD_Parola.h>
#include <MD_MAX72xx.h>

#define HARDWARE_TYPE MD_MAX72XX::FC16_HW
#define MAX_DEVICES 4  // 4 blocks
#define CS_PIN 21      // ESP32 pin GPIO21

const char* ssid = "Wokwi-GUEST";     // CHANGE IT
const char* password = "";  // CHANGE IT

AsyncWebServer server(80);
WebSocketsServer webSocket = WebSocketsServer(81);  // WebSocket server on port 81

// create an instance of the MD_Parola class
MD_Parola ledMatrix = MD_Parola(HARDWARE_TYPE, CS_PIN, MAX_DEVICES);

void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
  switch (type) {
    case WStype_DISCONNECTED:
      Serial.printf("[%u] Disconnected!\n", num);
      break;
    case WStype_CONNECTED:
      {
        IPAddress ip = webSocket.remoteIP(num);
        Serial.printf("[%u] Connected from %d.%d.%d.%d\n", num, ip[0], ip[1], ip[2], ip[3]);
      }
      break;
    case WStype_TEXT:
      String text = String((char*)payload);
      Serial.println("Received: " + text);
      ledMatrix.displayClear();  // clear led matrix display
      ledMatrix.displayScroll(text.c_str(), PA_CENTER, PA_SCROLL_LEFT, 100);
      webSocket.sendTXT(num, "Displayed: " + String((char*)payload) + "on LED Matrix");
      break;
  }
}

void setup() {
  Serial.begin(9600);
  ledMatrix.begin();          // initialize the object
  ledMatrix.setIntensity(0);  // set the brightness of the LED matrix display (from 0 to 15)
  ledMatrix.displayClear();   // clear led matrix display

  ledMatrix.displayScroll("esp32io.com", PA_CENTER, PA_SCROLL_LEFT, 100);

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");

  // Initialize WebSocket server
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);

  // Serve a basic HTML page with JavaScript to create the WebSocket connection
  server.on("/", HTTP_GET, [](AsyncWebServerRequest * request) {
    Serial.println("Web Server: received a web page request");
    String html = HTML_CONTENT;  // Use the HTML content from the index.h file
    request->send(200, "text/html", html);
  });

  server.begin();
  // Serial.print("ESP32 Web Server's IP address: ");
  // Serial.println(WiFi.localIP());
  Serial.println("Open http://localhost:8080 in your browser!");
}

void loop() {
  webSocket.loop();

  if (ledMatrix.displayAnimate()) {
    ledMatrix.displayReset();
  }
  delay(1);
}