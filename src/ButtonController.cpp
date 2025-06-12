#include "ButtonController.h"

ButtonController::ButtonController(uint8_t pin, SimpleBuzzer& b)
    : button(pin, INPUT_PULLUP), buzzer(b), pressStartTime(0), currentState(State::NO_PRESS) {
    // ezButton constructor handles pinMode with INPUT_PULLUP
    button.setDebounceTime(DEBOUNCE_TIME);
    button.setCountMode(COUNT_FALLING); // Count when button goes from HIGH to LOW
}

void ButtonController::begin() {
    // Initialization logic for the button controller, if any, can go here.
    // For now, it's empty as per the original header.
}

void ButtonController::loop() {
    button.loop(); // Essential for ezButton to update its internal state

    if (button.isPressed()) {
        if (currentState == State::NO_PRESS) {
            // Button was just pressed
            pressStartTime = millis();
            currentState = State::SHORT_PRESS; // Tentatively set to short press
            // buzzer.beep(); // Optional: beep on initial press
        } else {
            // Button is being held
            unsigned long duration = millis() - pressStartTime;
            if (duration >= VERY_LONG_PRESS_DURATION) {
                if (currentState != State::VERY_LONG_PRESS) {
                    currentState = State::VERY_LONG_PRESS;
                    // buzzer.beep(200); // Optional: different beep for very long press
                }
            } else if (duration >= LONG_PRESS_DURATION) {
                if (currentState != State::LONG_PRESS && currentState != State::VERY_LONG_PRESS) {
                    currentState = State::LONG_PRESS;
                    // buzzer.beep(100); // Optional: different beep for long press
                }
            }
        }
    } else if (button.isReleased()) {
        if (currentState != State::NO_PRESS) {
            // Button was just released
            unsigned long duration = millis() - pressStartTime;
            if (duration < SHORT_PRESS_DURATION) { // Filter out very short, possibly noise presses
                currentState = State::NO_PRESS;
            }
            // The state (SHORT, LONG, VERY_LONG) is already set by the time it's released,
            // unless it was too short.
            // If you need to act *on release* based on duration, you can do it here.
            // For now, we just reset to NO_PRESS if it was a valid press.
            // If it was too short, it's already NO_PRESS.
            // If it was a valid press, it will be reset on the next loop if not pressed.
        }
    } else {
        // Button is not pressed and not just released
        if (currentState != State::NO_PRESS) {
             // Reset state if it was previously pressed but now is not
            currentState = State::NO_PRESS;
            pressStartTime = 0;
        }
    }
}

ButtonController::State ButtonController::state() const {
    return currentState;
}

bool ButtonController::isPressing() const {
    return currentState != State::NO_PRESS;
}