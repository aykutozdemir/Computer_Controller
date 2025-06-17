#include "MXRMReceiver.h"

// Initialize static member
MXRMReceiver* MXRMReceiver::_isrInstance = nullptr;

MXRMReceiver::MXRMReceiver(uint8_t interruptPin) :
    _interruptPin(interruptPin),
    _buttonCode(0),
    _newButtonCodeAvailable(false),
    _timingCount(0),
    _lastInterruptTime_us(0),
    _lastDecodedCode(0),
    _lastDecodeTimeMs(0)
{
    // Set the static instance pointer. Assumes only one instance.
    // For multiple instances, ISR would need a way to identify which instance.
    if (_isrInstance == nullptr) {
        _isrInstance = this;
    }
}

void MXRMReceiver::begin()
{
    // Configure pin as input with pull-up
    Serial.printf("MXRMReceiver: Initializing RF receiver on interrupt pin %d.\n", _interruptPin);
    pinMode(_interruptPin, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(_interruptPin), ISR_Handler, CHANGE);
    _lastInterruptTime_us = micros(); // Initialize last interrupt time
}

void IRAM_ATTR MXRMReceiver::ISR_Handler() {
    if (_isrInstance) {
        _isrInstance->handleInterrupt();
    }
}

void IRAM_ATTR MXRMReceiver::handleInterrupt() {
    unsigned long now_us = micros();
    unsigned int duration = now_us - _lastInterruptTime_us;
    _lastInterruptTime_us = now_us;

    // If the gap since the previous edge is larger than the frame timeout we
    // consider this to be the start of a new transmission.  We therefore
    // clear the buffer and ignore this (very long) duration so that the first
    // stored timing will belong to the actual data frame.
    if (duration > PACKET_TIMEOUT_US) {
        _timingCount = 0;
        return; // wait for next edge to start capturing a fresh packet
    }

    if (_timingCount < MAX_TIMINGS_BUFFER_SIZE) {
        // Store the inter-edge duration
        _timings[_timingCount++] = duration;
    } else {
        // Buffer full, likely noise or very long signal. Reset.
        _timingCount = 0; 
    }
}

bool MXRMReceiver::read() {
    unsigned int localTimingCount = 0;
    unsigned int localTimings[MAX_TIMINGS_BUFFER_SIZE];

    // Check if ISR has captured anything or if too much time passed (packet end)
    noInterrupts();
    bool potentialPacket = (_timingCount > 0 && (micros() - _lastInterruptTime_us > PACKET_TIMEOUT_US));
    unsigned int capturedCountForLog = _timingCount; // Capture for logging before reset
    if (potentialPacket) {
        localTimingCount = _timingCount;
        if (localTimingCount > MAX_TIMINGS_BUFFER_SIZE) localTimingCount = MAX_TIMINGS_BUFFER_SIZE; // Cap
        memcpy(localTimings, (void*)_timings, localTimingCount * sizeof(unsigned int));
        _timingCount = 0; // Reset ISR buffer
        // Reset the baseline timestamp so that the duration of the first pulse
        // in the next packet is measured correctly and does not include the idle
        // time since the last transmission.  This prevents the first timing
        // value from becoming an erroneously large number which would later
        // cause decodePulses() to reject the packet.
        _lastInterruptTime_us = micros();
    }
    interrupts();

    if (potentialPacket && capturedCountForLog > 0) {
        Serial.printf("MXRMReceiver::read() - Potential packet. ISR captured %u timings. Processing...\n", capturedCountForLog);
    }

    if (potentialPacket && localTimingCount > (REQUIRED_DATA_BITS / 2)) { // Need at least some pulses
        if (decodePulses(localTimings, localTimingCount)) {
            // Debounce logic
            unsigned long currentTimeMs = millis();
            if (_buttonCode != _lastDecodedCode || (currentTimeMs - _lastDecodeTimeMs > MIN_REPEAT_DELAY_MS)) {
                _newButtonCodeAvailable = true;
                _lastDecodedCode = _buttonCode;
                _lastDecodeTimeMs = currentTimeMs;
            } else {
                // Same code received too soon, ignore for now
                _newButtonCodeAvailable = false; 
            }
        } else {
            // Serial.println("MXRMReceiver::read() - Decode failed."); // Already logged in decodePulses
            _newButtonCodeAvailable = false;
        }
    }
    // else if (potentialPacket) { Serial.printf("MXRMReceiver::read() - Not enough pulses for full decode: %u\n", localTimingCount); }
    return _newButtonCodeAvailable;
}

bool MXRMReceiver::decodePulses(const unsigned int* capturedTimings, unsigned int count) {
    Serial.printf("decodePulses: Received %u timings. Required minimum for EV1527-like is ~50.\n", count);
    // Print all captured timings for debugging
    Serial.print("Raw timings (us): ");
    for (unsigned int k = 0; k < count; k++) {
        Serial.print(capturedTimings[k]);
        Serial.print(" ");
    }
    Serial.println();

    if (count < (2 + REQUIRED_DATA_BITS * 2)) { // 2 for sync, 2 per data bit
        Serial.printf("decodePulses: Insufficient pulse count (%u) for %d data bits.\n", count, REQUIRED_DATA_BITS);
        return false; 
    }

    // ------------------------------------------------------------------
    // Locate the sync pulse (1T high followed by ~30T low) inside the
    // captured timings instead of assuming it starts at index 0.  Depending
    // on when the ISR started recording we might begin with the low part of
    // the sync pulse or even somewhere inside the data stream.
    // ------------------------------------------------------------------

    int syncIndex = -1;
    for (unsigned int i = 0; i + 1 < count; ++i) {
        if ((capturedTimings[i]   >= MIN_SYNC_HIGH_US && capturedTimings[i]   <= MAX_SYNC_HIGH_US) &&
            (capturedTimings[i+1] >= MIN_SYNC_LOW_US  && capturedTimings[i+1] <= MAX_SYNC_LOW_US)) {
            syncIndex = i;
            break;
        }
    }

    if (syncIndex == -1) {
        Serial.println("decodePulses: Failed to locate sync pattern in captured data.");
        return false;
    }

    unsigned int t_unit_high = capturedTimings[syncIndex];
    unsigned int sync_low_duration = capturedTimings[syncIndex + 1];
    Serial.printf("decodePulses: Sync found at index %d (High: %u us, Low: %u us).\n", syncIndex, t_unit_high, sync_low_duration);

    // Sync pulse found. Now decode data bits starting after the low part of
    // the sync pulse.
    uint32_t decodedValue = 0;
    int bitsDecoded = 0;

    // Start decoding from the element two positions after syncIndex
    for (unsigned int i = syncIndex + 2; i + 1 < count && bitsDecoded < REQUIRED_DATA_BITS; i += 2) {
        if (i + 1 >= count) { // Ensure we have a pair of pulses
            Serial.printf("decodePulses: Ran out of pulses at index %u while decoding data bits.\n", i);
            return false;
        }
        unsigned int pulse_high = capturedTimings[i];
        unsigned int pulse_low = capturedTimings[i+1];

        bool isBit0 = (pulse_high >= MIN_DATA_SHORT_US && pulse_high <= MAX_DATA_SHORT_US) &&
                      (pulse_low  >= MIN_DATA_LONG_US  && pulse_low  <= MAX_DATA_LONG_US);
        bool isBit1 = (pulse_high >= MIN_DATA_LONG_US  && pulse_high <= MAX_DATA_LONG_US) &&
                      (pulse_low  >= MIN_DATA_SHORT_US && pulse_low  <= MAX_DATA_SHORT_US);

        if (isBit0) {
            decodedValue = (decodedValue << 1) | 0;
            bitsDecoded++;
        } else if (isBit1) {
            decodedValue = (decodedValue << 1) | 1;
            bitsDecoded++;
            // Serial.printf("Bit %d: 1 (H: %u, L: %u)\n", bitsDecoded, pulse_high, pulse_low);
        } else {
            Serial.printf("decodePulses: Invalid bit pattern for bit %d. High: %u us (Exp Short: %u-%u, Exp Long: %u-%u), Low: %u us (Exp Short: %u-%u, Exp Long: %u-%u)\n",
                bitsDecoded + 1, pulse_high, MIN_DATA_SHORT_US, MAX_DATA_SHORT_US, MIN_DATA_LONG_US, MAX_DATA_LONG_US,
                pulse_low, MIN_DATA_SHORT_US, MAX_DATA_SHORT_US, MIN_DATA_LONG_US, MAX_DATA_LONG_US);
            return false; 
        }
    }

    if (bitsDecoded == REQUIRED_DATA_BITS) {
        _buttonCode = decodedValue;
        Serial.printf("decodePulses: Successfully decoded %d bits. Code: 0x%X\n", bitsDecoded, _buttonCode);
        return true;
    }
    Serial.printf("decodePulses: Failed to decode required %d bits. Decoded %d bits.\n", REQUIRED_DATA_BITS, bitsDecoded);
    return false;
}


uint32_t MXRMReceiver::getButtonCode()
{
    _newButtonCodeAvailable = false; // Clear the flag once the code is read
    return _buttonCode;
}

bool MXRMReceiver::isNewButtonCode() const
{
    return _newButtonCodeAvailable;
}