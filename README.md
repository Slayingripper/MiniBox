# MiniBox CTF Trainer

This project is a Capture the Flag (CTF) training platform designed to run on an ESP8266 microcontroller. It offers a variety of challenges, including web-based puzzles, cryptographic tasks, and wireless network-related challenges. The purpose of this project is to provide an interactive way to practice CTF skills in a small, embedded environment.

## Features

- **LED Control**: Turn the onboard LED on or off via web API.
- **Client Management**: Display the number of clients connected to the ESP8266 access point.
- **Wireless Scanning**: Scan for available wireless networks.
- **Buzzer Control**: Play a melody on a connected buzzer.
- **Caesar Cipher Challenge**: Respond with a Caesar ciphered flag when accessing a specific endpoint.
- **HTTP Authentication**: Optionally secure API endpoints with basic HTTP authentication.
- **Dynamic Challenge Levels**: Switch between easy and hard difficulty levels.

## Hardware Requirements

- ESP8266 (e.g., NodeMCU or Wemos D1 Mini)
- Buzzer (optional, for the sound challenge)
- LEDs (optional, for the LED control challenge)

## Software Requirements

- Arduino IDE
- ESP8266 Board Package for Arduino IDE
- Libraries:
  - `ESP8266WiFi.h`
  - `ESP8266WebServer.h`

## Installation

1. **Clone the Repository**:
   ```sh
   git clone https://github.com/yourusername/MiniBox-CTF-Trainer.git
   cd MiniBox-CTF-Trainer
   ```

2. Open the Project:
3. Open the CTF.ino file in the Arduino IDE.

4. Install Required Libraries:
Make sure the ESP8266 board package and necessary libraries are installed in your Arduino IDE.

Upload the Code:
Connect your ESP8266 to your computer and upload the code.

5. Connect to the ESP8266:

    After uploading, the ESP8266 will create a Wi-Fi access point named MiniBox-CTF.
    Connect to the access point using the password yourpassword.

Access the CTF Platform:

    Open a web browser and navigate to http://192.168.4.1 to access the CTF interface.

## API Endpoints
/

    Description: Displays the main CTF page with hints and ASCII art.
    Method: GET

## /api/led/on

    Description: Turns on the onboard LED.
    Method: GET

## /api/led/off

    Description: Turns off the onboard LED.
    Method: GET

## /api/clients

    Description: Shows the number of connected clients and their MAC addresses.
    Method: GET

## /api/scan

    Description: Scans for available Wi-Fi networks.
    Method: GET

## /api/buzzer/play

    Description: Plays a melody on the buzzer.
    Method: GET

## /submitFlag

    Description: Submits a CTF flag for validation.
    Method: POST
    Request: The flag should be sent in the request body as {"flag": "FLAG_VALUE"}.

## /TimeNewRoman

    Description: Responds with a Caesar cipher of a flag.
    Method: GET

## Challenge Levels

    Easy Mode: All API endpoints are accessible without authentication.
    Hard Mode: Randomized HTTP Basic Authentication is required to access the API endpoints.

## Usage Example

    Turn on the LED:
        Make a GET request to /api/led/on.

    Submit a Flag:
        Make a POST request to /submitFlag with a JSON body:

        json

        {
          "flag": "FLAG_123"
        }

    Caesar Cipher Challenge:
        Access the /TimeNewRoman endpoint to receive a Caesar ciphered flag.

Hints

    LED Challenge: Control the LED to solve the challenge.
    Client Count: Check how many clients are connected. Hidden clues might be embedded in the response.
    Buzzer Challenge: Listen closely to the melody played by the buzzer. There might be a clue!
    Caesar Cipher: Try to decode the Caesar cipher to reveal the hidden flag.

Contributing

Contributions are welcome! Please fork the repository and submit a pull request with your changes.


### TODO

- [x] Add README.md
- [ ] Add more challenges
- [ ] Improve the web interface
- [ ] Add a scoring system
- [ ] refactor the code


