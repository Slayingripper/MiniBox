#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiUdp.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "Challenges.h"  // Include the challenges header file

const char* spinnerFrames[] = {
  "[      ]",   // Frame 0
  "[*     ]",   // Frame 1
  "[**    ]",   // Frame 2
  "[***   ]",   // Frame 3
  "[ ***  ]",   // Frame 4
  "[  *** ]",   // Frame 5
  "[   ***]",   // Frame 6
  "[    **]",   // Frame 7
  "[     *]"    // Frame 8
};

const int buzzerPin = D5;
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

const char *ssid = "MiniBox";  // Set your SSID
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
const int switchPin = D6;

ChallengeLevel currentLevel;

// Username and password for basic authentication
char http_username[6] = "admin"; // Mutable character array for username
char http_password[9]; // Mutable character array for password


void setup() {
  // Initialize serial communication
  Serial.begin(115200);
  delay(1000);
  
  // Initialize the display 
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3C for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }
  animateSpinner();
  display.clearDisplay();
  //display welcome message
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  //center the text
  display.setCursor((SCREEN_WIDTH - 64) / 2, SCREEN_HEIGHT / 2 - 10);
  // make text bigger
  display.setTextSize(2);
  display.println("MiniBox");
  display.display();
  delay(2000);
  // Set up LED pins
  pinMode(builtInLed, OUTPUT);
  pinMode(apiLedPin, OUTPUT); // Ensure API LED pin is set as output
  pinMode(errorLedPin, OUTPUT);
  pinMode(buzzerPin, OUTPUT); // Set the buzzer pin as output

  digitalWrite(builtInLed, HIGH); // Turn off built-in LED (assuming active low)
  digitalWrite(apiLedPin, HIGH); // Ensure API LED is off initially
  digitalWrite(errorLedPin, HIGH); // Ensure error LED is off initially

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
  currentLevel = getCurrentChallengeLevel(switchPin);
  setupServer(currentLevel);

  // Start the UDP server
  udp.begin(localUdpPort);
  // display initial information
  displayInfo();
}

void loop() {
  dnsServer.processNextRequest();
  server.handleClient();
  checkUdpPacket();
  broadcastFlag();
  displayInfo();
  
}

void setupServer(ChallengeLevel level) {
  // Set up the web server based on challenge level
  server.on("/", HTTP_GET, [](){
    if (!server.authenticate(http_username, http_password)) {
      return server.requestAuthentication();
    }
    handleRoot();
  });

  switch (level) {
    case EASY:
      // Allow easy access to API endpoints
      server.on("/api/led/on", HTTP_GET, [](){
        if (!server.authenticate(http_username, http_password)) {
          return server.requestAuthentication();
        }
        handleLedOn();
      });
      server.on("/api/led/off", HTTP_GET, [](){
        if (!server.authenticate(http_username, http_password)) {
          return server.requestAuthentication();
        }
        handleLedOff();
      });
      server.on("/api/clients", HTTP_GET, [](){
        if (!server.authenticate(http_username, http_password)) {
          return server.requestAuthentication();
        }
        handleClientList();
      });
      server.on("/api/scan", HTTP_GET, [](){
        if (!server.authenticate(http_username, http_password)) {
          return server.requestAuthentication();
        }
        handleScanNetworks();
      });
      server.on("/api/password", HTTP_GET, [](){
        if (!server.authenticate(http_username, http_password)) {
          return server.requestAuthentication();
        }
        handleShowPassword();
      });
      server.on("/api/test/buzzer", HTTP_GET, [](){
        if (!server.authenticate(http_username, http_password)) {
          return server.requestAuthentication();
        }
        handleTestBuzzer();
      });
      break;

    case MEDIUM:
      // Randomize password
      generateRandomPassword(password, 8);
      break;

    case HARD:
      // Randomize username and password
      generateRandomUsername(http_username, 5); // Generate a random username with 5 characters
      generateRandomPassword(password, 8); // Generate a random password with 8 characters
      break;

    default:
      break;
  }

  // Set up other routes as needed

  server.onNotFound(handleNotFound);
  server.begin();
  Serial.println("Server started");
}



void handleRoot() {
  String html = "<!DOCTYPE HTML><html><h1>MiniBox CTF Trainer</h1>";
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
  digitalWrite(apiLedPin, LOW); // Turn the API LED on
  server.send(200, "application/json", "{\"message\":\"API LED is ON\"}");
}

void handleLedOff() {
  digitalWrite(apiLedPin, HIGH); // Turn the API LED off
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
    response += "{\"ssid\":\"" + WiFi.SSID(i) + "\",\"rssi\":" + String(WiFi.RSSI(i)) + "}";
    if (i < n - 1) response += ",";
  }
  response += "]}";
  server.send(200, "application/json", response);
}

void handleShowPassword() {
  String response = "{\"password\":\"" + String(password) + "\"}";
  server.send(200, "application/json", response);
}

void handleError() {
  server.send(403, "application/json", "{\"message\":\"Access Denied\"}");
}

void handleChallengeLedOff() {
  server.send(403, "application/json", "{\"message\":\"Access Denied\"}");
}

void handleChallengeClientList() {
  server.send(403, "application/json", "{\"message\":\"Access Denied\"}");
}

void handleChallengeScanNetworks() {
  server.send(403, "application/json", "{\"message\":\"Access Denied\"}");
}

void handleChallengeShowPassword() {
  server.send(403, "application/json", "{\"message\":\"Access Denied\"}");
}

void handleTestBuzzer() {
  // Play Super Mario mushroom sound for the first 3 seconds
  unsigned long startTime = millis();
  Serial.println("Starting buzzer test...");
  for (int i = 0; i < sizeof(melody) / sizeof(melody[0]); i++) {
    // Check if more than 3 seconds have passed
    if (millis() - startTime > 3000) {
      break;
    }
    int toneDuration = 1000 / noteDurations[i];
    Serial.print("Playing tone: ");
    Serial.print(melody[i]);
    Serial.print(" for duration: ");
    Serial.println(toneDuration);
    tone(buzzerPin, melody[i], toneDuration); // Use buzzerPin for buzzer sound
    delay(toneDuration * 1.30); // Pause between notes
    noTone(buzzerPin); // Stop tone
  }
  Serial.println("Buzzer test completed.");

  server.send(200, "application/json", "{\"message\":\"Buzzer tested\"}");
}

void handleNotFound() {
  digitalWrite(errorLedPin, LOW); // Flash error LED for not found
  delay(100);
  digitalWrite(errorLedPin, HIGH);

  server.send(404, "application/json", "{\"message\":\"Not Found\"}");
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

void generateRandomUsername(char *username, int length) {
  const char alphanum[] = "abcdefghijklmnopqrstuvwxyz0123456789";
  for (int i = 0; i < length; i++) {
    username[i] = alphanum[random(36)];
  }
  username[length] = '\0';  // Null-terminate the string
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

void displayInfo() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  
  display.setCursor(0, 0);
  display.print("Clients: ");
  display.print(WiFi.softAPgetStationNum());

  //display IP address and ap password
  display.setCursor(0, 20);
  display.print("AP IP: ");
  display.println(WiFi.softAPIP());
  display.setCursor(0, 30);
  display.print("Password: ");
  display.println(password);

  
  display.setCursor(0, 10);
  display.print("Difficulty: ");
  switch (currentLevel) {
    case EASY:
      display.print("Easy");
      break;
    case MEDIUM:
      display.print("Medium");
      break;
    case HARD:
      display.print("Hard");
      break;
    default:
      display.print("Unknown");
      break;
  }
  //display the username and password
  display.setCursor(0, 40);
  display.print("Username: ");
  display.println(http_username);
  display.setCursor(0, 50);
  display.print("Password: ");
  display.println(http_password);

  
  display.display();
}

void animateSpinner() {

  const int numFrames = sizeof(spinnerFrames) / sizeof(spinnerFrames[0]);
  const int frameDelay = 200;  // Adjust delay as needed for animation speed

  for (int i = 0; i < numFrames; i++) {
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.println("Booting...");

    display.setCursor(30, 30);  // Adjust position for the spinner
    display.println(spinnerFrames[i]);
    display.display();

    delay(frameDelay);
  }
}

//broadcast a udp packet with the flag to the network
void broadcastFlag() {
  //broadcast the flag to the network
  udp.beginPacket("CTF{This_Is_A_Flag}", 1337);
  udp.write("CTF{This_Is_A_Flag}");
  udp.endPacket();
}

