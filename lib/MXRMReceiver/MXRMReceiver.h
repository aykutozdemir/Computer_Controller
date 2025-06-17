#pragma once

#include <Arduino.h>

class MXRMReceiver {
public:
    // Constructor - takes an interrupt pin number
    MXRMReceiver(uint8_t interruptPin);
    
    // Initialize the receiver
    void begin(); // Attaches interrupt
    
    // Attempts to decode any packet data captured by the ISR.
    // Returns true if a new, valid button code is available after processing.
    bool read(); 
    
    // Get the unique button code. Clears the new button code flag.
    uint32_t getButtonCode();
    
    // Check if a new button code was received and is ready to be read by getButtonCode()
    bool isNewButtonCode() const;

private:
    // Static ISR handler and instance pointer
    static void IRAM_ATTR ISR_Handler();
    static MXRMReceiver* _isrInstance;

    // Instance-specific interrupt handling logic
    void IRAM_ATTR handleInterrupt();

    uint8_t _interruptPin;
    uint32_t _buttonCode;
    bool _newButtonCodeAvailable; // True if a new, valid, debounced code is ready
    
    // ISR pulse capture
    volatile unsigned int _timings[100]; // Buffer for pulse durations
    volatile unsigned int _timingCount;  // Number of pulses captured
    volatile unsigned long _lastInterruptTime_us;

    // EV1527-like protocol decoding parameters (typical values, may need tuning)
    // A "unit" or "T" is typically 250-500 microseconds.
    // --- Reverted to more standard EV1527-like timings ---
    static const unsigned int MIN_SYNC_HIGH_US = 200;    // Sync: 1T high
    static const unsigned int MAX_SYNC_HIGH_US = 600;
    static const unsigned int MIN_SYNC_LOW_US = 6000;   // Sync: ~30T low (e.g. 30 * 250us = 7500us)
    static const unsigned int MAX_SYNC_LOW_US = 12000;  // Adjusted slightly from a common 10ms to catch up to 12ms

    static const unsigned int MIN_DATA_SHORT_US = 200;   // Data bit pulse: 1T
    static const unsigned int MAX_DATA_SHORT_US = 600;
    static const unsigned int MIN_DATA_LONG_US = 700;    // Data bit pulse: 3T
    static const unsigned int MAX_DATA_LONG_US = 1500;
    // --- End standard timings ---

    static const unsigned int MAX_TIMINGS_BUFFER_SIZE = sizeof(_timings) / sizeof(_timings[0]);
    static const int REQUIRED_DATA_BITS = 24; // This might also need to change (e.g., 16, 20)
    static const unsigned int PACKET_TIMEOUT_US = 20000; // Max time between pulses before reset
    
    // Debouncing for successfully decoded packets
    uint32_t _lastDecodedCode;
    unsigned long _lastDecodeTimeMs;
    static const unsigned long MIN_REPEAT_DELAY_MS = 300; // Min delay to process same code again

    bool decodePulses(const unsigned int* capturedTimings, unsigned int count);
}; 