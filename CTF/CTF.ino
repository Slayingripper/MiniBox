#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiUdp.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "Challenges.h"  // Include the challenges header file

const char* spinnerFrames[] = {
  "[      ]", "[*     ]", "[**    ]", "[***   ]", 
  "[ ***  ]", "[  *** ]", "[   ***]", "[    **]",
  "[     *]"
};

const int buzzerPin = D5;
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define CUBE_SIZE 20
#define ROTATE_SPEED 10

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// WiFi and server settings
const char *ssid = "MiniBox";
char  password[9]= "12345678";
ESP8266WebServer server(80);
DNSServer dnsServer;
WiFiUDP udp;
unsigned int localUdpPort = 1337;
unsigned int hintPort = 808;

// Define the Super Mario mushroom sound frequency and duration for the first 3 seconds
const int melody[] = {660, 660, 660, 510, 660, 770};
const int noteDurations[] = {4, 4, 4, 8, 4, 4};

const int smelody[] = {
  784, 784, 784, 659, 784, 587, 523  // G, G, G, E, G, D, C
};

const int snoteDurations[] = {
  4, 4, 4, 8, 4, 4, 2  // Note durations
};

// Define the pins for the rotary encoder and button
const int builtInLed = LED_BUILTIN;  // Built-in LED pin
const int encoderPinA = D3;
const int encoderPinB = D6;
const int buttonPin = A0;
const int apiLedPin = D7; // LED controlled via API
const int errorLedPin = D7; // LED for errors or non-existent URLs

// Variables for encoder state and position
int lastEncoderStateA = LOW;
int encoderStateA = LOW;
int encoderPosition = 0;
bool buttonPressed = false;

ChallengeLevel currentLevel = EASY;  // Default difficulty

// Username and password for basic authentication
char http_username[6] = "admin";
char http_password[9] = "password";

void setup() {
  // Initialize serial communication
  Serial.begin(115200);
  delay(1000);
   // Check if the button is pressed before creating the AP
  buttonPressed = analogRead(buttonPin) == LOW; // Button pressed when LOW
  // Initialize the display
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;);
  }
  display.setRotation(2);
  animateSpinner();
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor((SCREEN_WIDTH - 64) / 2, SCREEN_HEIGHT / 2 - 10);
  display.println("MiniBox");
  display.display();
  delay(2000);

  // Set up pins for LEDs, encoder, and button
  pinMode(buzzerPin, OUTPUT);
  pinMode(encoderPinA, INPUT);
  pinMode(encoderPinB, INPUT);
  pinMode(buttonPin, INPUT_PULLUP);  // Enable internal pull-up resistor
  // Display a message on the OLED screen and ask user to hold the button if they want to change the difficulty to Hard
  
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("Hold the button to change difficulty");
  display.display();
  // Wait for 2 seconds to allow the user to press the button
  delay(2000);
  checkButtonPress();
  if (buttonPressed) {
    Serial.println("Button is pressed - Hard mode activated");
    //turn on the led to indicate the user to press the button
    digitalWrite(builtInLed, HIGH);
    //turn on api LED on pin d7 to indicate the user to press the button
    digitalWrite(apiLedPin, HIGH);
    currentLevel = HARD;
    //play the melody
    for (int i = 0; i < sizeof(smelody) / sizeof(smelody[0]); i++) {
      int toneDuration = 1000 / snoteDurations[i];
      tone(buzzerPin, smelody[i], toneDuration); // Use buzzerPin for buzzer sound
      delay(toneDuration * 1.30); // Pause between notes
      noTone(buzzerPin); // Stop tone
    }
    

  } 
  animateCube();
 
   //show a animation for those 2 seconds

  // Initialize WiFi access point
   if (WiFi.softAP(ssid, password)) {
    Serial.println("Access Point Started");
    Serial.print("IP Address: ");
    Serial.println(WiFi.softAPIP());
  } else {
    Serial.println("Failed to start Access Point");
  }

  dnsServer.start(53, "*", WiFi.softAPIP());
  
  udp.begin(localUdpPort);
  setupServer(currentLevel);
  displayInfo();
}

void loop() {
     dnsServer.processNextRequest();
     server.handleClient();
     checkUdpPacket();
  broadcastFlag();
  updateEncoder();
  checkButtonPress();
}


void setupServer(ChallengeLevel level) {
  server.onNotFound(handleNotFound);
  server.on("/", HTTP_GET, [](){
    if (!server.authenticate(http_username, http_password)) {
      return server.requestAuthentication();
    }
    handleRoot();
  });

  if (level == EASY) {
    // Easy level API endpoints
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

    // Add API endpoint to play a buzzer tone
    server.on("/api/buzzer/play", HTTP_GET, [](){
      if (!server.authenticate(http_username, http_password)) {
        return server.requestAuthentication();
      }
      handlePlayBuzzer();
    });
  } else if (level == HARD) {
    // Hard level with randomized username and password
    generateRandomUsername(http_username, 5);
    generateRandomPassword(http_password, 8);
  }
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
  html += "<p><a href=\"/api/buzzer/play\"><button>Play Buzzer</button></a></p>"; // Button to play buzzer
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
  if (digitalRead(switchPin) == LOW) {
    return EASY;
  } else {
    // Add more conditions or checks for different challenge levels
    return HARD;
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

  //show difficulty  bar depending on the encoder position

  
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


void checkButtonPress() {
  buttonPressed = (analogRead(buttonPin) < 100);
  //only pick up first 1 press at a time
  delay(500);
  if (buttonPressed) {
    Serial.println("Button is pressed");
    
  }
}


void updateEncoder() {
  encoderStateA = digitalRead(encoderPinA);
  
  if (encoderStateA != lastEncoderStateA) {
    if (digitalRead(encoderPinB) != encoderStateA) {
      encoderPosition++;
    } else {
      encoderPosition--;
    }
    lastEncoderStateA = encoderStateA;

    // Update difficulty based on encoder position
    if (encoderPosition > 1) encoderPosition = 0;
    if (encoderPosition < 0) encoderPosition = 1;
    displayInfo();
  }
}


// Function to animate the cube
void animateCube() {
  float angleX = 0;
  float angleY = 0;
  int xCenter = SCREEN_WIDTH / 2;
  int yCenter = SCREEN_HEIGHT / 2;
  int zOffset = SCREEN_HEIGHT / 4;
  
  unsigned long startTime = millis();
  unsigned long animationDuration = 5000; // Duration in milliseconds (10 seconds)

  while (millis() - startTime < animationDuration) {
    drawCube(xCenter, yCenter, zOffset, angleX, angleY);
    angleX += 0.01; // Rotate the cube around X axis
    angleY += 0.01; // Rotate the cube around Y axis
    
    // Play the "woosh" sound every time the cube rotates
    playWooshSound();
  }
}

// Function to draw a cube on the display
void drawCube(int xCenter, int yCenter, int zOffset, float angleX, float angleY) {
  display.clearDisplay();
  
  // Define the cube's vertices relative to the center
  float vertices[8][3] = {
    {-CUBE_SIZE, -CUBE_SIZE, -CUBE_SIZE},
    { CUBE_SIZE, -CUBE_SIZE, -CUBE_SIZE},
    { CUBE_SIZE,  CUBE_SIZE, -CUBE_SIZE},
    {-CUBE_SIZE,  CUBE_SIZE, -CUBE_SIZE},
    {-CUBE_SIZE, -CUBE_SIZE,  CUBE_SIZE},
    { CUBE_SIZE, -CUBE_SIZE,  CUBE_SIZE},
    { CUBE_SIZE,  CUBE_SIZE,  CUBE_SIZE},
    {-CUBE_SIZE,  CUBE_SIZE,  CUBE_SIZE}
  };

  // Rotate vertices around the X and Y axes
  for (int i = 0; i < 8; ++i) {
    float x = vertices[i][0];
    float y = vertices[i][1];
    float z = vertices[i][2];

    // Rotate around X axis
    float newY = y * cos(angleX) - z * sin(angleX);
    float newZ = y * sin(angleX) + z * cos(angleX);
    y = newY;
    z = newZ;

    // Rotate around Y axis
    float newX = x * cos(angleY) - z * sin(angleY);
    newZ = x * sin(angleY) + z * cos(angleY);
    x = newX;
    z = newZ;

    // Translate to center
    vertices[i][0] = x + xCenter;
    vertices[i][1] = y + yCenter;
    vertices[i][2] = z + zOffset;
  }

  // Draw the edges of the cube
  int edges[12][2] = {
    {0, 1}, {1, 2}, {2, 3}, {3, 0}, // Bottom face
    {4, 5}, {5, 6}, {6, 7}, {7, 4}, // Top face
    {0, 4}, {1, 5}, {2, 6}, {3, 7}  // Vertical edges
  };

  for (int i = 0; i < 12; ++i) {
    int start = edges[i][0];
    int end = edges[i][1];
    display.drawLine(
      vertices[start][0], vertices[start][1],
      vertices[end][0], vertices[end][1],
      SSD1306_WHITE
    );
  }

  display.display();
}

void playWooshSound() {
  int frequency = 80; // Start frequency for the "woosh"
  int duration = 10; // Duration of each part of the sound

  for (int i = 0; i < 3; i++) {
    tone(buzzerPin, frequency, duration); // Play tone
    delay(duration); // Wait for the tone to play
    noTone(buzzerPin); // Stop tone
    frequency += 1000; // Increase frequency for rising effect
  }
}

void handlePlayBuzzer() {
  // Play a simple tone on the buzzer
  int frequency = 1000;  // Frequency in Hertz
  int duration = 500;    // Duration in milliseconds
  
  tone(buzzerPin, frequency, duration);  // Play the tone
  delay(duration);  // Wait for the tone to finish
  noTone(buzzerPin);  // Stop the tone

  // Send a JSON response to the client
  server.send(200, "application/json", "{\"message\":\"Buzzer played\"}");
}