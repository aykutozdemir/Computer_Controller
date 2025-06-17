/**
 * @file LedController.cpp
 * @brief Implementation of the LED controller class
 */

#include "LedController.h"

LedController::LedController(uint8_t pin)
    : _led(pin), _currentStatus(Status::OFF) {
    if (pin == 0) {
        // Invalid pin number
        return;
    }
    pinMode(pin, OUTPUT); // Set pin as output
}

void LedController::begin() {
    // Initialize LED to off state
    _led.turnOFF();
    applyStatus(); // Apply initial status
}

void LedController::loop() {
    _led.loop(); // Essential for ezLed's blink functionality
}

void LedController::setStatus(Status newStatus) {
    if (_currentStatus != newStatus) {
        _currentStatus = newStatus;
        applyStatus();
    }
}

LedController::Status LedController::getStatus() const {
    return _currentStatus;
}

void LedController::applyStatus() {
    switch (_currentStatus) {
        case Status::OFF:
            _led.turnOFF();
            break;
        case Status::CONNECTING:
            _led.blink(LED_CONNECTING_BLINK_ON_MS, LED_CONNECTING_BLINK_OFF_MS);
            break;
        case Status::CONNECTED:
            _led.turnON();
            break;
        default:
            _led.turnOFF(); // Default to off for unknown states
            break;
    }
}