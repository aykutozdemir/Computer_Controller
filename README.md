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

# Computer Controller

A comprehensive ESP32-based computer controller with RF remote control, environmental monitoring, and web interface capabilities.

## Features

- **Power Management**: Remote power on/off and reset functionality
- **RF Remote Control**: Learn and use RF remote controls for power management
- **Environmental Monitoring**: Temperature and humidity sensing
- **GPU Fan Control**: Automatic and manual GPU fan speed control
- **Web Interface**: Built-in web server for remote access
- **Telegram Bot**: Remote control via Telegram messages
- **MQTT Integration**: IoT connectivity and status reporting
- **Child Lock**: Safety feature to prevent accidental activation
- **Audible Feedback**: Buzzer for user notifications

## RF Stability Improvements

The system includes several advanced features to improve RF signal stability and reduce false positives:

### Signal Validation
- **Multi-Signal Validation**: Requires 2 identical signals before accepting a code
- **Signal Strength Tracking**: Monitors signal strength to filter weak signals
- **Noise Filtering**: 100ms delay between signals to filter out noise spikes
- **Bit Length Validation**: Validates signal bit length (12-32 bits)
- **Delay Validation**: Checks signal timing parameters for validity

### Configuration Parameters
```cpp
// RF Configuration in Globals.h
static constexpr uint8_t RF_RECEIVE_TOLERANCE = 60;      // Reduced from 80 for better quality
static constexpr uint16_t RF_MIN_SIGNAL_STRENGTH = 3;    // Minimum signal strength required
static constexpr uint16_t RF_SIGNAL_VALIDATION_COUNT = 2; // Identical signals required
static constexpr uint16_t RF_NOISE_FILTER_DELAY = 100;   // Noise filtering delay (ms)
static constexpr uint16_t RF_SIGNAL_TIMEOUT = 5000;      // Signal validation timeout (ms)
```

### Monitoring and Debugging
Use the `rf stats` command to monitor RF stability:
```
rf stats
```
This shows:
- Total signals received
- Valid signals count
- Noise signals count
- Success rate percentage
- Current signal strength
- Signal validation status

### Troubleshooting RF Issues

1. **High Noise Count**: If you see many noise signals, try:
   - Moving the RF receiver away from electrical interference
   - Using a different GPIO pin for the RF receiver
   - Adding a ferrite bead to the RF receiver power line

2. **Low Success Rate**: If success rate is below 80%:
   - Check RF receiver positioning and antenna
   - Verify remote control battery
   - Adjust `RF_RECEIVE_TOLERANCE` (try 50-70)
   - Increase `RF_MIN_SIGNAL_STRENGTH` to 4-5

3. **False Triggers**: If you get unwanted activations:
   - Increase `RF_SIGNAL_VALIDATION_COUNT` to 3
   - Increase `RF_NOISE_FILTER_DELAY` to 150-200ms
   - Check for nearby RF sources (WiFi, Bluetooth, etc.)

4. **No Signal Detection**: If signals aren't being detected:
   - Decrease `RF_RECEIVE_TOLERANCE` to 70-80
   - Decrease `RF_MIN_SIGNAL_STRENGTH` to 2
   - Verify RF receiver is properly connected to GPIO 35
   - Check remote control frequency compatibility

### Hardware Recommendations

- **RF Receiver**: Use a 433MHz superheterodyne receiver module
- **Antenna**: Add a 17.3cm wire antenna for better reception
- **Shielding**: Keep RF receiver away from switching power supplies
- **Power Supply**: Use a clean 3.3V power supply for the RF receiver
- **Grounding**: Ensure proper grounding to reduce noise

### Advanced Configuration

For environments with high RF interference, you can adjust these parameters in `Globals.h`:

```cpp
// More strict settings for noisy environments
static constexpr uint8_t RF_RECEIVE_TOLERANCE = 50;      // Stricter tolerance
static constexpr uint16_t RF_MIN_SIGNAL_STRENGTH = 4;    // Higher strength requirement
static constexpr uint16_t RF_SIGNAL_VALIDATION_COUNT = 3; // More validation signals
static constexpr uint16_t RF_NOISE_FILTER_DELAY = 200;   // Longer noise filter
```

For environments with weak signals:
```cpp
// More lenient settings for weak signals
static constexpr uint8_t RF_RECEIVE_TOLERANCE = 70;      // More lenient tolerance
static constexpr uint16_t RF_MIN_SIGNAL_STRENGTH = 2;    // Lower strength requirement
static constexpr uint16_t RF_SIGNAL_VALIDATION_COUNT = 1; // Single signal validation
static constexpr uint16_t RF_NOISE_FILTER_DELAY = 50;    // Shorter noise filter
```

## Installation 