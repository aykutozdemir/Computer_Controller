#pragma once

#include <Arduino.h>

class MXRMReceiver {
public:
    // Constructor - takes an interrupt pin number
    MXRMReceiver(uint8_t interruptPin);
    
    // Initialize the receiver
    void begin();
    
    // Read the current value from the receiver
    bool read();
    
    // Check if the value has changed
    bool hasChanged() const;
    
    // Get the unique button code
    uint32_t getButtonCode();
    
    // Check if a new button code was received
    bool isNewButtonCode();

private:
    uint8_t _interruptPin;
    bool _lastValue;
    bool _hasChanged;
    uint32_t _buttonCode;
    bool _newButtonCode;
    
    // Pulse timing measurement
    unsigned long _lastChangeTime;
    unsigned int _pulseCount;
    unsigned long _totalPulseTime;
    
    // Internal methods
    void _update();
}; 