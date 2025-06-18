#include "PowerResetController.h"

PowerResetController::PowerResetController() : 
    powerButton(POWER_BUTTON_PIN, INPUT_PULLUP),
    resetButton(RESET_BUTTON_PIN, INPUT_PULLUP) {
    // Set debounce time for both buttons
    powerButton.setDebounceTime(DEBOUNCE_TIME);
    resetButton.setDebounceTime(DEBOUNCE_TIME);
    
    // Set count mode for both buttons
    powerButton.setCountMode(COUNT_FALLING);
    resetButton.setCountMode(COUNT_FALLING);
}

void PowerResetController::begin() {
    // Nothing to initialize
}

void PowerResetController::loop() {
    powerButton.loop();
    resetButton.loop();
}

bool PowerResetController::isPowerPressed() const {
    return powerButton.isPressed();
}

bool PowerResetController::isResetPressed() const {
    return resetButton.isPressed();
} 
