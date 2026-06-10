# The audio recorder based on ESP32 and INMP441 (I2S) with saving WAV-files to MicroSD

![image](https://github.com/user-attachments/assets/2c3cde85-9fc2-4634-b6c5-806b858d65eb)

A simple dictaphone (voice recorder) firmware for ESP32(S3). After power-on or reset the board records a fixed-length audio clip from an I2S MEMS microphone (INMP441) and saves it as a standard 16-bit PCM WAV file to a MicroSD card. The 32-bit samples coming from the microphone are converted to 16-bit on the fly with an adjustable volume scaling factor, so the resulting files can be played on any computer or phone without extra processing.

The project is organized as a [PlatformIO](https://platformio.org/) project. The original Arduino IDE sketch is kept in the [`arduino/`](arduino/) folder.

## Project structure

```
├── platformio.ini                  # PlatformIO configuration (ESP32-S3, Arduino framework)
├── src/
│   └── main.cpp                    # Main firmware source (PlatformIO)
└── arduino/
    └── SimpleESP32Dictaphone/
        └── SimpleESP32Dictaphone.ino   # Legacy Arduino IDE sketch
```

## Need components:
1) ESP32(S3) board
2) INMP441 (or other I2S microphone)
3) MicroSD HC card

Optional: adapter for MicroSD card OR use ESP32S3 with internal microsd slot (in my case)

## Features
- Support INMP441 or other I2S microphone
- Support MicroSD HC Cards (via SD_MMC, 1-bit mode)
- Audio Recording after start ESP32 or reset button press on ESP32
- Name of files into card use randomly for several recordings (`audio_XXXXX.wav`)
- Format audio: WAV (PCM, mono, 16-bit, 16 kHz by default)
- 32-bit → 16-bit sample conversion with volume scaling
- Possibility to choose: record duration, volume level, buffer size and sample rate (see constants in code)

## Configuration

All settings are constants at the top of [`src/main.cpp`](src/main.cpp):

| Constant | Default | Description |
| ----------- | ----------- | ----------- |
| `SAMPLE_RATE` | `16000` | Sampling rate, Hz |
| `DURATION_SEC` | `10` | Recording duration, seconds |
| `VOLUME_SCALE` | `0.2` | Volume scaling (0.0 — silence, 1.0 — original level) |
| `BUFFER_SIZE` | `1024` | I2S read buffer size, samples |
| `SD_MMC_CMD/CLK/D0` | `38/39/40` | MicroSD (SD_MMC) pins |
| `I2S_MIC_*` | `15/16/17` | I2S microphone pins (WS/SCK/SD) |

## Getting started (PlatformIO)

1) Install [PlatformIO](https://platformio.org/install) (VS Code extension or CLI)
2) Connect microphone and microsd to GPIO (see tables below)
3) Connect your ESP32 Board to your computer
4) Build and upload:
```
pio run -t upload
```
5) (Optional) Open the serial monitor to watch the recording progress:
```
pio device monitor
```

The default environment targets ESP32-S3 (`esp32-s3-devkitc-1`). For another board, change `board` in [`platformio.ini`](platformio.ini) (a commented-out example for classic ESP32 is included).

## Getting started (Arduino IDE, legacy)

0) Setting up your Arduino IDE for ESP32 Boards
1) Open `arduino/SimpleESP32Dictaphone/SimpleESP32Dictaphone.ino`
2) Connect microphone and microsd to GPIO
3) Connect your ESP32 Board to your computer
4) Upload this code to your ESP32

## How to connect microphone

| ESP32(S3) Board | INMP441 |
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

## How it works

1. On boot the firmware mounts the MicroSD card over SD_MMC (1-bit mode) and installs the I2S driver in master RX mode.
2. A random file name `audio_XXXXX.wav` is generated, so several recordings can be made by pressing reset without overwriting previous files.
3. A WAV header is written, then samples are read from the microphone in chunks, converted from 32-bit to 16-bit with volume scaling, and streamed to the file.
4. After `DURATION_SEC` seconds the WAV header is updated with the final file size and the file is closed.

[Demonstration on YouTube](https://youtu.be/J6_7y23uscI?si=-QVYGFy7MUHNssFf)
