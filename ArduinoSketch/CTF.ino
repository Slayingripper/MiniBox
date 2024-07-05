#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiUdp.h>
#include "Challenges.h"  // Include the challenges header file

const char *ssid = "ESP8266_Honeypot";  // Set your SSID
char password[9];  // WiFi password, up to 8 characters plus null terminator

// Set LED, switch, and buzzer pins
const int builtInLed = LED_BUILTIN;  // Built-in LED pin
const int apiLedPin = D3; // LED controlled via API
const int errorLedPin = D7; // LED for errors or non-existent URLs

ESP8266WebServer server(80);
DNSServer dnsServer;
WiFiUDP udp;

unsigned int localUdpPort = 1337;  // local port to listen on
unsigned int hintPort = 808;  // port to send hint message

// Define the Super Mario mushroom sound frequency and duration for the first 3 seconds
const int melody[] = {660, 660, 660, 510, 660, 770};
const int noteDurations[] = {4, 4, 4, 8, 4, 4};

// 3-way switch pin
const int switchPin = D1;

void setup() {
  // Initialize serial communication
  Serial.begin(115200);
  delay(10);

  // Set up LED pins
  pinMode(builtInLed, OUTPUT);
  pinMode(apiLedPin, OUTPUT);
  pinMode(errorLedPin, OUTPUT);

  digitalWrite(builtInLed, LOW); // Turn off built-in LED (assuming active low)
  digitalWrite(apiLedPin, LOW); // Ensure API LED is off initially
  digitalWrite(errorLedPin, LOW); // Ensure error LED is off initially

  // Set up switch pin
  pinMode(switchPin, INPUT);

  // Generate password based on switch state
  if (digitalRead(switchPin) == HIGH) {
    strncpy(password, "12345678", 9); // Default password for easier access
  } else {
    generateRandomPassword(password, 8); // Harder challenge with random password
  }

  // Start the access point
  Serial.println("Setting up AP...");
  WiFi.softAP(ssid, password);

  // Start the DNS server
  dnsServer.start(53, "*", WiFi.softAPIP());

  // Print the IP address and password
  Serial.print("AP IP address: ");
  Serial.println(WiFi.softAPIP());
  Serial.print("AP Password: ");
  Serial.println(password);

  // Set up the web server based on challenge level
  ChallengeLevel currentLevel = getCurrentChallengeLevel(switchPin);
  setupServer(currentLevel);

  // Start the UDP server
  udp.begin(localUdpPort);
}

void loop() {
  dnsServer.processNextRequest();
  server.handleClient();
  checkUdpPacket();
}

void setupServer(ChallengeLevel level) {
  // Set up the web server based on challenge level
  server.on("/", handleRoot);

  switch (level) {
    case EASY:
      // Allow easy access to API endpoints
      server.on("/api/led/on", HTTP_GET, handleLedOn);
      server.on("/api/led/off", HTTP_GET, handleLedOff);
      server.on("/api/clients", HTTP_GET, handleClientList);
      server.on("/api/scan", HTTP_GET, handleScanNetworks);
      server.on("/api/password", HTTP_GET, handleShowPassword);
      server.on("/api/test/buzzer", HTTP_GET, handleTestBuzzer);
      break;

    case MEDIUM:
      // Add some restrictions or additional challenge
      server.on("/api/led/on", HTTP_GET, handleChallengeLedOn);
      server.on("/api/led/off", HTTP_GET, handleChallengeLedOff);
      server.on("/api/clients", HTTP_GET, handleChallengeClientList);
      server.on("/api/scan", HTTP_GET, handleChallengeScanNetworks);
      server.on("/api/password", HTTP_GET, handleChallengeShowPassword);
      server.on("/api/test/buzzer", HTTP_GET, handleTestBuzzer); // Allow buzzer testing
      break;

    case HARD:
      // Implement more complex challenges or stricter access control
      server.on("/api/led/on", HTTP_GET, handleChallengeLedOn);
      server.on("/api/led/off", HTTP_GET, handleChallengeLedOff);
      server.on("/api/clients", HTTP_GET, handleChallengeClientList);
      server.on("/api/scan", HTTP_GET, handleChallengeScanNetworks);
      server.on("/api/password", HTTP_GET, handleChallengeShowPassword);
      server.on("/api/test/buzzer", HTTP_GET, handleTestBuzzer); // Allow buzzer testing
      break;

    default:
      // Default to easy level if switch position is unrecognized
      server.on("/api/led/on", HTTP_GET, handleLedOn);
      server.on("/api/led/off", HTTP_GET, handleLedOff);
      server.on("/api/clients", HTTP_GET, handleClientList);
      server.on("/api/scan", HTTP_GET, handleScanNetworks);
      server.on("/api/password", HTTP_GET, handleShowPassword);
      server.on("/api/test/buzzer", HTTP_GET, handleTestBuzzer);
      break;
  }

  server.onNotFound(handleNotFound);
  server.begin();
  Serial.println("Server started");
}

void handleRoot() {
  String html = "<!DOCTYPE HTML><html><h1>ESP8266 Honeypot</h1>";
  html += "<p><a href=\"/api/led/on\"><button>Turn On LED</button></a></p>";
  html += "<p><a href=\"/api/led/off\"><button>Turn Off LED</button></a></p>";
  html += "<p><a href=\"/api/clients\"><button>Show Connected Clients</button></a></p>";
  html += "<p><a href=\"/api/scan\"><button>Scan Wireless Networks</button></a></p>";
  html += "<p><a href=\"/api/password\"><button>Show Current Password</button></a></p>";
  html += "<p><a href=\"/api/test/buzzer\"><button>Test Buzzer</button></a></p>"; // Button to test buzzer
  html += "</html>";
  server.send(200, "text/html", html);
}

void handleLedOn() {
  digitalWrite(apiLedPin, HIGH); // Turn the API LED on
  server.send(200, "application/json", "{\"message\":\"API LED is ON\"}");
}

void handleLedOff() {
  digitalWrite(apiLedPin, LOW); // Turn the API LED off
  server.send(200, "application/json", "{\"message\":\"API LED is OFF\"}");
}

void handleClientList() {
  digitalWrite(builtInLed, LOW); // Flash built-in LED when a client connects
  delay(100);
  digitalWrite(builtInLed, HIGH);

  int clientCount = WiFi.softAPgetStationNum();
  String response = "{\"client_count\":" + String(clientCount) + ",\"clients\":[";
  struct station_info *stat_info = wifi_softap_get_station_info();
  while (stat_info != NULL) {
    response += "{\"mac\":\"";
    for (int i = 0; i < 6; ++i) {
      response += String(stat_info->bssid[i], 16);
      if (i < 5) response += ":";
    }
    response += "\"}";
    stat_info = STAILQ_NEXT(stat_info, next);
    if (stat_info != NULL) response += ",";
  }
  response += "]}";
  server.send(200, "application/json", response);
}

void handleScanNetworks() {
  digitalWrite(errorLedPin, LOW); // Flash error LED when scanning networks
  delay(100);
  digitalWrite(errorLedPin, HIGH);

  int n = WiFi.scanNetworks();
  String response = "{\"network_count\":" + String(n) + ",\"networks\":[";
  for (int i = 0; i < n; ++i) {
    response += "{\"ssid\":\"" + WiFi.SSID(i) + "\",\"channel\":" + String(WiFi.channel(i)) + "}";
    if (i < n - 1) response += ",";
  }
  response += "]}";
  server.send(200, "application/json", response);
}

void handleShowPassword() {
  String response = "{\"password\":\"" + String(password) + "\"}";
  server.send(200, "application/json", response);
}

void handleChallengeLedOn() {
  // Implement challenge logic or restrict access further
  server.send(403, "application/json", "{\"message\":\"Access Denied\"}");
}

void handleChallengeLedOff() {
  // Implement challenge logic or restrict access further
  server.send(403, "application/json", "{\"message\":\"Access Denied\"}");
}

void handleChallengeClientList() {
  // Implement challenge logic or restrict access further
  server.send(403, "application/json", "{\"message\":\"Access Denied\"}");
}

void handleChallengeScanNetworks() {
  // Implement challenge logic or restrict access further
  server.send(403, "application/json", "{\"message\":\"Access Denied\"}");
}

void handleChallengeShowPassword() {
  // Implement challenge logic or restrict access further
  server.send(403, "application/json", "{\"message\":\"Access Denied\"}");
}

void handleTestBuzzer() {
  // Play Super Mario mushroom sound for the first 3 seconds
  unsigned long startTime = millis();
  for (int i = 0; i < sizeof(melody) / sizeof(melody[0]); i++) {
    // Check if more than 3 seconds have passed
    if (millis() - startTime > 3000) {
      break;
    }
    int toneDuration = 1000 / noteDurations[i];
    tone(apiLedPin, melody[i], toneDuration); // Use apiLedPin for buzzer sound
    delay(toneDuration * 1.30); // Pause between notes
    noTone(apiLedPin); // Stop tone
  }
  
  server.send(200, "application/json", "{\"message\":\"Buzzer tested\"}");
}

void handleNotFound() {
  digitalWrite(errorLedPin, LOW); // Flash error LED for not found
  delay(100);
  digitalWrite(errorLedPin, HIGH);

  server.send(404, "application/json", "{\"message\":\"Not Found\"}");
  logRequest();
}

void logRequest() {
  Serial.print("Request from ");
  Serial.print(server.client().remoteIP());
  Serial.print(": ");
  Serial.println(server.uri());
}

void checkUdpPacket() {
  int packetSize = udp.parsePacket();
  if (packetSize) {
    Serial.print("Received packet of size ");
    Serial.println(packetSize);
    Serial.print("From ");
    IPAddress remoteIp = udp.remoteIP();
    Serial.print(remoteIp);
    Serial.print(", port ");
    Serial.println(udp.remotePort());

    // Read the packet into a buffer
    char incomingPacket[255];
    int len = udp.read(incomingPacket, 255);
    if (len > 0) {
      incomingPacket[len] = 0;
    }
    Serial.println("Contents:");
    Serial.println(incomingPacket);

    // Check if the packet has the structure of an MQTT message
    // Here you can add your custom logic to parse and handle the packet
    if (len >= 2 && incomingPacket[0] == 0x30) { // MQTT CONNECT message
      Serial.println("Received MQTT CONNECT message");
      // Handle the MQTT message accordingly
      // Example: turn on buzzer for testing
      handleTestBuzzer();
    } else {
      Serial.println("Unknown packet structure. Sending hint message...");
      sendHintMessage(remoteIp);
    }
  }
}

void sendHintMessage(IPAddress clientIp) {
  String hintMessage = "Unknown packet structure. Hint: Send an MQTT-like packet.";
  udp.beginPacket(clientIp, hintPort);
  udp.write(hintMessage.c_str());
  udp.endPacket();
  Serial.println("Hint message sent.");
}

void generateRandomPassword(char *password, int length) {
  const char alphanum[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
  bool hasNumber = false;

  for (int i = 0; i < length; i++) {
    char c = alphanum[random(62)];
    if (isdigit(c)) {
      hasNumber = true;
    }
    password[i] = c;
  }

  // Ensure at least one number
  if (!hasNumber) {
    password[random(length)] = '0' + random(10);
  }

  password[length] = '\0';  // Null-terminate the string
}

ChallengeLevel getCurrentChallengeLevel(int switchPin) {
  // Determine the current challenge level based on switch position
  if (digitalRead(switchPin) == HIGH) {
    return EASY;
  } else {
    // Add more conditions or checks for different challenge levels
    // For simplicity, assume MEDIUM level when switch is LOW
    return MEDIUM;
  }
}
