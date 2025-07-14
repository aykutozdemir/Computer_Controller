#include "RCSwitchReceiver.h"

RCSwitchReceiver::RCSwitchReceiver(uint8_t interruptPin)
    : interruptPin(interruptPin) {
    // Initialize signal validation buffer
    for (int i = 0; i < RF_SIGNAL_VALIDATION_COUNT; i++) {
        signalValidationBuffer[i] = 0;
    }
}

void RCSwitchReceiver::begin() {
    // The RCSwitch library expects Interrupt numbers rather than raw GPIOs
    rcSwitch.enableReceive(digitalPinToInterrupt(interruptPin));
    rcSwitch.setReceiveTolerance(RF_RECEIVE_TOLERANCE);
    
    // Configure pin with internal pull-up for better noise immunity
    pinMode(interruptPin, INPUT_PULLUP);
}

bool RCSwitchReceiver::read() {
    if (rcSwitch.available()) {
        totalSignalsReceived++;
        unsigned long received = rcSwitch.getReceivedValue();
        unsigned int receivedLength = rcSwitch.getReceivedBitlength();
        unsigned int receivedDelay = rcSwitch.getReceivedDelay();
        
        rcSwitch.resetAvailable();
        
        // Debug output for troubleshooting
        ESP_LOGD("RCSwitchReceiver", "Signal received: 0x%lX, length: %u, delay: %u", 
                 received, receivedLength, receivedDelay);
        
        if (received == 0) {
            // 0 usually indicates an error in reception – ignore
            noiseSignalsCount++;
            ESP_LOGD("RCSwitchReceiver", "Ignoring zero value signal");
            return false;
        }

        unsigned long now = millis();
        
        // Check if we should enable fallback mode (no valid signals for 15 seconds)
        if (!fallbackMode && (now - lastValidSignalTime) > 15000) {
            fallbackMode = true;
            ESP_LOGW("RCSwitchReceiver", "Enabling fallback mode for better signal reception");
        }

        if (fallbackMode) {
            // Fallback mode: accept almost any signal
            ESP_LOGD("RCSwitchReceiver", "Fallback mode: accepting signal");
            if (received != lastCode || (now - lastReportTime) > RF_REPEAT_DELAY) {
                lastCode = received;
                lastReportTime = now;
                lastValidSignalTime = now;
                newCodeAvailable = true;
                signalValidated = true;
                validSignalsCount++;
                ESP_LOGI("RCSwitchReceiver", "Valid signal in fallback mode: 0x%lX", received);
                return true;
            }
        } else {
            // Normal mode: very lenient signal quality checks
            if (receivedLength < 4 || receivedLength > 128) {
                // Very wide range - accept almost any length
                noiseSignalsCount++;
                ESP_LOGD("RCSwitchReceiver", "Signal length out of range: %u", receivedLength);
                return false;
            }
            
            if (receivedDelay < 10 || receivedDelay > 50000) {
                // Very wide range - accept almost any delay
                noiseSignalsCount++;
                ESP_LOGD("RCSwitchReceiver", "Signal delay out of range: %u", receivedDelay);
                return false;
            }

            // Minimal noise filtering - only ignore extremely rapid signals
            if ((now - lastSignalTime) < RF_NOISE_FILTER_DELAY) {
                noiseSignalsCount++;
                ESP_LOGD("RCSwitchReceiver", "Signal too rapid, filtering out");
                return false;
            }
            lastSignalTime = now;

            // Simplified signal validation - just store the signal
            signalValidationBuffer[signalValidationIndex] = received;
            signalValidationIndex = (signalValidationIndex + 1) % RF_SIGNAL_VALIDATION_COUNT;
            
            // Since validation count is 1, we accept the signal immediately
            signalStrength++;
            
            // Debounce identical codes that arrive in rapid succession
            if (received != lastCode || (now - lastReportTime) > RF_REPEAT_DELAY) {
                lastCode = received;
                lastReportTime = now;
                lastValidSignalTime = now;
                newCodeAvailable = true;
                signalValidated = true;
                signalStrength = 0; // Reset for next signal
                validSignalsCount++;
                ESP_LOGI("RCSwitchReceiver", "Valid signal accepted: 0x%lX", received);
                return true;
            }
        }
    }
    return false;
}

uint32_t RCSwitchReceiver::getButtonCode() {
    newCodeAvailable = false;
    return lastCode;
}

void RCSwitchReceiver::getSignalStats(uint32_t& totalSignals, uint32_t& validSignals, uint32_t& noiseSignals) const {
    totalSignals = totalSignalsReceived;
    validSignals = validSignalsCount;
    noiseSignals = noiseSignalsCount;
}

void RCSwitchReceiver::resetSignalStats() {
    totalSignalsReceived = 0;
    validSignalsCount = 0;
    noiseSignalsCount = 0;
    signalStrength = 0;
    signalValidated = false;
    fallbackMode = false;
    lastValidSignalTime = 0;
    
    // Reset validation buffer
    for (int i = 0; i < RF_SIGNAL_VALIDATION_COUNT; i++) {
        signalValidationBuffer[i] = 0;
    }
    signalValidationIndex = 0;
}

void RCSwitchReceiver::setFallbackMode(bool enabled) {
    fallbackMode = enabled;
    if (enabled) {
        ESP_LOGI("RCSwitchReceiver", "Fallback mode enabled for better signal reception");
    } else {
        ESP_LOGI("RCSwitchReceiver", "Fallback mode disabled");
    }
} 