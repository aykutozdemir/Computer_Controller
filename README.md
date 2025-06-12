# ESP32 Project 707

This is a PlatformIO project for ESP32 using the ESPDUINO-32 hardware configuration.

## Project Structure

```
.
├── platformio.ini    # Project configuration file
├── src/             # Source files
│   └── main.cpp     # Main source file
└── README.md        # This file
```

## Hardware Requirements

- ESP32 Development Board
- USB cable for programming and power

## Features

- Basic LED blinking example
- Serial communication at 115200 baud rate
- Uses built-in LED on GPIO 2

## Building and Uploading

1. Make sure you have PlatformIO installed
2. Connect your ESP32 board via USB
3. Run the following commands:
   ```bash
   pio run        # Build the project
   pio run -t upload  # Upload to ESP32
   pio device monitor  # Monitor serial output
   ```

## Configuration

The project is configured for:
- CPU Frequency: 240MHz
- Flash Size: 4MB
- Upload Speed: 921600 baud
- Monitor Speed: 115200 baud 