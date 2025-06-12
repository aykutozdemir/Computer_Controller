#include "../include/ButtonController.h"

void ButtonController::loop()
{
    button.loop();
    buzzer.loop();  // Update buzzer state

    if (button.isPressed())
    {
        // Button was just pressed, record the time and reset state
        pressStartTime = millis() & 0xFFFF;
        currentState = State::NO_PRESS;
        if (!buzzer.isActive()) {  // Only start new beep if not already active
            buzzer.beep(50); // Short beep on press
        }
    }
    else if (button.isReleased())
    {
        // Button was just released, calculate the press duration and determine type
        const uint16_t currentTime = millis() & 0xFFFF;
        const uint16_t pressedDuration = currentTime - pressStartTime;

        currentState = pressedDuration >= VERY_LONG_PRESS_DURATION ? State::VERY_LONG_PRESS
                    : pressedDuration >= LONG_PRESS_DURATION    ? State::LONG_PRESS
                    : pressedDuration >= SHORT_PRESS_DURATION   ? State::SHORT_PRESS
                    : State::NO_PRESS;

        // Different beep patterns for different press durations
        if (!buzzer.isActive()) {  // Only start new pattern if not already active
            switch (currentState) {
                case State::VERY_LONG_PRESS:
                    buzzer.beepPattern(3, 200, 100); // Three long beeps
                    break;
                case State::LONG_PRESS:
                    buzzer.beepPattern(2, 150, 100); // Two medium beeps
                    break;
                case State::SHORT_PRESS:
                    buzzer.beep(100); // Single short beep
                    break;
                default:
                    break;
            }
        }
    }
    else
    {
        // No change in button state
        currentState = State::NO_PRESS;
    }
}

ButtonController::State ButtonController::state() const
{
    return currentState;
}

bool ButtonController::isPressing() const
{
    return button.getState() == LOW;
}

bool ButtonController::isPressingRaw() const
{
    return button.getStateRaw() == LOW;
} 