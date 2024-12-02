import requests
import hashlib
import socket
import time
import re
import binascii
from scapy.all import sniff, UDP, IP
import sys

def challenge_1():
    url = 'http://192.168.4.1/api/led/on'
    response = requests.get(url)
    print('Response Status Code:', response.status_code)
    print('Response Text:', response.text)
    try:
        data = response.json()
        flag = data.get('flag', 'Flag not found')
        print('Challenge 1 Flag:', flag)
    except requests.exceptions.JSONDecodeError:
        print('Failed to decode JSON. The response may not be in JSON format.')


def challenge_2():
    # Challenge 2: Access /api/clients and extract the flag.
    url = 'http://192.168.4.1/api/clients'
    response = requests.get(url)
    data = response.json()
    flag = data.get('flag', 'Flag not found')
    print('Challenge 2 Flag:', flag)

def challenge_3():
    # Challenge 3: Access /api/scan and extract the flag.
    url = 'http://192.168.4.1/api/scan'
    response = requests.get(url)
    data = response.json()
    flag = data.get('flag', 'Flag not found')
    print('Challenge 3 Flag:', flag)

def challenge_4():
    # Challenge 4: Access /api/buzzer/play and extract the flag.
    url = 'http://192.168.4.1/api/buzzer/play'
    response = requests.get(url)
    data = response.json()
    flag = data.get('flag', 'Flag not found')
    print('Challenge 4 Flag:', flag)

def challenge_5():
    # Challenge 5: Hash Validation Challenge at /api/hashiklis.
    url = 'http://192.168.4.1/api/hashiklis'
    response = requests.get(url)
    data = response.json()
    message = data.get('message', '')
    print('Challenge 5 Message:', message)
    # Generate an MD5 hash of any string, e.g., "test"
    test_string = 'test'
    md5_hash = hashlib.md5(test_string.encode()).hexdigest()
    # Send the hash back
    response = requests.post(url, data={'plain': md5_hash})
    data = response.json()
    flag = data.get('flag', 'Flag not found')
    print('Challenge 5 Flag:', flag)

def challenge_6():
    # Challenge 6: Access /TimeNewRoman, get ciphered flag, and decipher it.
    url = 'http://192.168.4.1/TimeNewRoman'
    response = requests.get(url)
    data = response.json()
    ciphered_flag = data.get('ciphered_flag', '')
    print('Challenge 6 Ciphered Flag:', ciphered_flag)
    # Decipher using a Caesar cipher with a shift of 3
    def caesar_decipher(text, shift):
        result = ''
        for c in text:
            if c.isalpha():
                shift_base = 65 if c.isupper() else 97
                result += chr((ord(c) - shift_base - shift) % 26 + shift_base)
            else:
                result += c
        return result
    flag = caesar_decipher(ciphered_flag, 3)
    print('Challenge 6 Flag:', flag)

def challenge_7():
    # Challenge 7: Hidden UDP Broadcast
    # Capture UDP packets on port 1337
    def packet_callback(packet):
        if UDP in packet and packet[UDP].sport == 1337:
            data = packet[UDP].payload.load.decode()
            print('Challenge 7 Flag:', data)
            # Stop sniffing
            sys.exit()

    print('Challenge 7: Listening for UDP broadcast on port 1337...')
    sniff(filter='udp port 1337', prn=packet_callback, timeout=10)

def challenge_8():
    # Challenge 8: User-Agent Header Challenge at /specialAgent
    url = 'http://192.168.4.1/specialAgent'
    headers = {'User-Agent': 'secret-agent'}
    response = requests.get(url, headers=headers)
    data = response.json()
    flag = data.get('flag', 'Flag not found')
    print('Challenge 8 Flag:', flag)

def challenge_9():
    # Challenge 9: Cookie Challenge at /cookieMonster
    url = 'http://192.168.4.1/cookieMonster'
    # First, get the cookie from the server
    response = requests.get(url)
    # Modify the cookie
    cookies = {'auth': 'letmein'}
    response = requests.get(url, cookies=cookies)
    data = response.json()
    flag = data.get('flag', 'Flag not found')
    print('Challenge 9 Flag:', flag)

def challenge_10():
    # Challenge 10: Host Header Challenge at /hostSecret
    url = 'http://192.168.4.1/hostSecret'
    headers = {'Host': 'secret.local'}
    response = requests.get(url, headers=headers)
    data = response.json()
    flag = data.get('flag', 'Flag not found')
    print('Challenge 10 Flag:', flag)

def challenge_11():
    # Challenge 11: Timing Attack Challenge at /timingAttack
    url = 'http://192.168.4.1/timingAttack'
    charset = 'CTF{}ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789_'
    discovered = ''
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
        print('Discovered so far:', discovered)
        if next_char == '}':
            break
    flag = discovered
    print('Challenge 11 Flag:', flag)

def challenge_12():
    # Challenge 12: Hex Encoding Challenge at /superSecret
    url = 'http://192.168.4.1/superSecret'
    response = requests.get(url)
    data = response.json()
    hex_data = data.get('data', '')
    # Decode the hex data
    flag = bytes.fromhex(hex_data).decode()
    print('Challenge 12 Flag:', flag)

def challenge_13():
    # Challenge 13: CAPTCHA Challenge at /captcha
    url = 'http://192.168.4.1/captcha'
    response = requests.get(url)
    data = response.json()
    question = data.get('question', '')
    print('Challenge 13 Question:', question)
    # Extract numbers and operator from the question
    match = re.match(r'What is (\d+) (\+|\-|\*|/) (\d+)\?', question)
    if match:
        num1 = int(match.group(1))
        op = match.group(2)
        num2 = int(match.group(3))
        if op == '+':
            answer = num1 + num2
        elif op == '-':
            answer = num1 - num2
        elif op == '*':
            answer = num1 * num2
        elif op == '/':
            answer = num1 / num2
        else:
            answer = 0
    else:
        answer = 0
    # Submit the answer
    response = requests.post(url, data={'answer': str(answer)})
    data = response.json()
    flag = data.get('flag', 'Flag not found')
    print('Challenge 13 Flag:', flag)

def challenge_14():
    # Challenge 14: UDP Packet Challenge
    # Send a specific UDP packet and receive the flag
    target_ip = '192.168.4.1'
    target_port = 1337
    message = 'pleaseSendFlag'
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.sendto(message.encode(), (target_ip, target_port))
    # Set timeout
    sock.settimeout(5)
    try:
        data, addr = sock.recvfrom(1024)
        flag = data.decode()
        print('Challenge 14 Flag:', flag)
    except socket.timeout:
        print('Challenge 14: No response received.')
    sock.close()

def main():
    challenge_1()
    challenge_2()
    challenge_3()
    challenge_4()
    challenge_5()
    challenge_6()
    challenge_7()
    challenge_8()
    challenge_9()
    challenge_10()
    challenge_11()
    challenge_12()
    challenge_13()
    challenge_14()
    print("All challenges completed.")

if __name__ == '__main__':
    main()
