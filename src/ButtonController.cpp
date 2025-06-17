#include "ButtonController.h"

ButtonController::ButtonController(uint8_t pin, SimpleBuzzer& b)
    : button(pin, INPUT_PULLUP), buzzer(b), 
      pressStartTime(0), currentState(State::NO_PRESS),
      beepTimer(BUTTON_PRESS_BEEP_INTERVAL_MS) {
    // ezButton constructor handles pinMode with INPUT_PULLUP
    button.setDebounceTime(DEBOUNCE_TIME);
    button.setCountMode(COUNT_FALLING); // Count when button goes from HIGH to LOW
}

void ButtonController::begin() {
    // No initialization needed
}

void ButtonController::loop() {
    button.loop(); // Essential for ezButton to update its internal state

    if (button.getState() == LOW) { // Button is pressed
        if (pressStartTime == 0) { // Just pressed
            pressStartTime = millis();
            buzzer.beep(BUTTON_PRESS_BEEP_DURATION_MS);
            currentState = State::SHORT_PRESS;
        } else { // Still pressed
            unsigned long duration = millis() - pressStartTime;
            
            // Update state based on duration
            if (duration >= VERY_LONG_PRESS_DURATION) {
                currentState = State::VERY_LONG_PRESS;
            } else if (duration >= LONG_PRESS_DURATION) {
                currentState = State::LONG_PRESS;
            }

            // Periodic beep
            if (beepTimer.isReady()) {
                buzzer.beep(BUTTON_PRESS_BEEP_DURATION_MS);
            }
        }
    } else if (pressStartTime != 0) { // Button was just released
        unsigned long duration = millis() - pressStartTime;
        
        // Set final state based on duration
        if (duration >= VERY_LONG_PRESS_DURATION) {
            currentState = State::VERY_LONG_PRESS;
        } else if (duration >= LONG_PRESS_DURATION) {
            currentState = State::LONG_PRESS;
        } else if (duration >= SHORT_PRESS_DURATION) {
            currentState = State::SHORT_PRESS;
        } else {
            currentState = State::NO_PRESS;
        }
        
        pressStartTime = 0;
    } else {
        currentState = State::NO_PRESS;
    }
}

ButtonController::State ButtonController::state() const {
    return currentState;
}

bool ButtonController::isPressing() const {
    return currentState != State::NO_PRESS;
}