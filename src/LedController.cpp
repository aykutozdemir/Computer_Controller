/**
 * @file LedController.cpp
 * @brief Implementation of the LED controller class
 */

#include "LedController.h"

LedController::LedController(uint8_t pin)
    : _led(pin, CTRL_CATHODE), _currentStatus(Status::OFF) {
    if (pin == 0) {
        // Invalid pin number
        return;
    }
    pinMode(pin, OUTPUT); // Set pin as output
}

void LedController::begin() {
    Serial.println("LED Controller: Initializing...");
    // Initialize LED to off state
    _led.turnOFF();
    applyStatus(); // Apply initial status
    Serial.println("LED Controller: Initialization complete");
}

void LedController::loop() {
    static uint32_t lastDebugTime = 0;
    uint32_t now = millis();
    
    // Debug logging every 5 seconds, but only when something changed
    if (now - lastDebugTime > 5000) {
        static Status lastStatus = Status::OFF;
        static uint8_t lastEzState = 0;

        Status currentStatus = _currentStatus;
        uint8_t ezState = _led.getState();

        if (currentStatus != lastStatus || ezState != lastEzState) {
            Serial.printf("LED Controller: status=%d, ezLED state=%d\n", (int)currentStatus, ezState);
            lastStatus = currentStatus;
            lastEzState = ezState;
        }
        lastDebugTime = now;
    }
    
    _led.loop(); // Essential for ezLed's blink functionality
}

void LedController::setStatus(Status newStatus) {
    if (_currentStatus != newStatus) {
        Serial.printf("LED Status changing from %d to %d\n", (int)_currentStatus, (int)newStatus);
        _currentStatus = newStatus;
        applyStatus();
    }
}

LedController::Status LedController::getStatus() const {
    return _currentStatus;
}

void LedController::applyStatus() {
    Serial.printf("Applying LED status: %d\n", (int)_currentStatus);
    switch (_currentStatus) {
        case Status::OFF:
            Serial.println("LED: Turning OFF");
            _led.turnOFF();
            break;
        case Status::CONNECTING:
            Serial.printf("LED: Starting blink pattern (%dms on, %dms off)\n", 
                         LED_CONNECTING_BLINK_ON_MS, LED_CONNECTING_BLINK_OFF_MS);
            _led.blink(LED_CONNECTING_BLINK_ON_MS, LED_CONNECTING_BLINK_OFF_MS);
            break;
        case Status::CONNECTED:
            Serial.println("LED: Turning ON");
            _led.turnON();
            break;
        default:
            Serial.printf("LED: Unknown status %d, turning OFF\n", (int)_currentStatus);
            _led.turnOFF(); // Default to off for unknown states
            break;
    }
}