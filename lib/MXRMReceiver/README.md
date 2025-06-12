# MX-RM Receiver Driver

This library provides a simple and efficient way to interface with MX-RM 5V receivers in Arduino projects. The driver handles digital input reading and change detection.

## Features

- Simple initialization and usage
- Digital input reading (HIGH/LOW)
- Change detection
- Update frequency monitoring

## Installation

1. Download the library
2. Place the `MXRMReceiver` folder in your Arduino libraries directory
3. Restart the Arduino IDE

## Usage

```cpp
#include <MXRMReceiver.h>

// Create an instance with the digital pin number
// The second parameter (true) enables voltage divider compensation
MXRMReceiver receiver(2, true);  // Use any digital pin

void setup() {
    // Initialize the receiver
    receiver.begin();
}

void loop() {
    // Read the current value
    bool value = receiver.read();
    
    // Check if the value has changed
    if (receiver.hasChanged()) {
        // Do something when the value changes
        if (value) {
            // Button pressed or signal received
        } else {
            // Button released or no signal
        }
    }
    
    // Get the update frequency
    float frequency = receiver.getUpdateFrequency();
}
```

## Methods

- `MXRMReceiver(uint8_t pin, bool useVoltageDivider = true)` - Constructor
- `void begin()` - Initialize the receiver
- `bool read()` - Read the current value (HIGH/LOW)
- `bool hasChanged()` - Check if the value has changed
- `unsigned long getLastReadTime()` - Get the last read timestamp
- `float getUpdateFrequency()` - Get the update frequency in Hz

## Wiring for 3.3V MCU

When using with a 3.3V MCU, you need to use a voltage divider to safely read the 5V signal:

```
MX-RM Receiver    Voltage Divider    3.3V MCU
    5V ----+----[4.7K]----+----[10K]----GND
           |              |
           |              |
           +--------------+----[Digital Pin]
```

Components needed:
- 1x 4.7K resistor (R1)
- 1x 10K resistor (R2)
- Jumper wires

This voltage divider will convert 5V to approximately 3.3V:
- R1 = 4.7K
- R2 = 10K
- Vout = 5V * (10K / (4.7K + 10K)) ≈ 3.3V

## Notes

- The driver uses digital input, so make sure to connect the receiver to a digital pin
- The MX-RM receiver outputs a digital signal (HIGH/LOW)
- When using with a 3.3V MCU, always use the voltage divider circuit as shown above
- The update frequency is calculated based on the time between reads 