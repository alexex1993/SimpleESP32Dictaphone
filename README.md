# The audio recorder based on ESP32 and IMP441 (I2S) with saving WAV-files to MicroSD

![image](https://github.com/user-attachments/assets/2c3cde85-9fc2-4634-b6c5-806b858d65eb)

##Need components:
1) ESP32(S3) board
2) SSD1306 OLED Display
3) MicroSD HC card

Optional: adapter for MicroSD card OR use ESP32S3 with internal microsd slot (in my case)

## Features
- Support IMP441 or other I2S microphone
- Support MicroSD HC Cards
- Audio Recording after start ESP32 or reset button press on ESP32
- Name of files into card use randomly for several recordings
- Format audio: wav raw
- Possibility to choose: record duration, volume level, buffer size and sample rate (show constants in code)

## Getting started

0) Setting up your Arduino IDE for ESP32 Boards
1) Connect microphone and microsd to GPIO
2) Connect your ESP32 Board to your computer
3) Upload this code to your ESP32

## How to connect microphone

| ESP32(S3) Board | IMP441 |
| ----------- | ----------- |
| GND   | GND   |
| 3V3 | VCC   |
| GND | L/R |
| D15    | WS   |
| D16    | SCK  |
| D17    | SD   |

## How to connect MicroSD
```
#define SD_MMC_CMD 38 // CMD pin for SD_MMC
#define SD_MMC_CLK 39 // CLK pin for SD_MMC
#define SD_MMC_D0  40 // D0 pin for SD_MMC
```
Read examples for `SD_MMC.h` library and compare with your MicroSD adapter

[Demonstration on YouTube](https://youtu.be/J6_7y23uscI?si=-QVYGFy7MUHNssFf)
