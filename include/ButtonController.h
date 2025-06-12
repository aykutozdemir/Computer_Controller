#ifndef BUTTON_CONTROLLER_H
#define BUTTON_CONTROLLER_H

#include <Arduino.h>
#include <ezButton.h>
#include "SimpleBuzzer.h"

class ButtonController {
public:
    // Button press states
    enum class State {
        NO_PRESS,
        SHORT_PRESS,
        LONG_PRESS,
        VERY_LONG_PRESS
    };

    // Press duration thresholds (in milliseconds)
    static constexpr uint16_t SHORT_PRESS_DURATION = 50;    // Minimum duration for a valid press
    static constexpr uint16_t LONG_PRESS_DURATION = 1000;   // 1 second for long press
    static constexpr uint16_t VERY_LONG_PRESS_DURATION = 3000; // 3 seconds for very long press
    static constexpr uint16_t DEBOUNCE_TIME = 50;           // Debounce time in milliseconds

    // Constructor with explicit pull-up mode
    ButtonController(uint8_t pin) : button(pin, INPUT_PULLUP), buzzer(27) {
        // Configure button with pull-up
        pinMode(pin, INPUT_PULLUP);
        button.setDebounceTime(DEBOUNCE_TIME);
        button.setCountMode(COUNT_FALLING);  // Count when button goes from HIGH to LOW
        pressStartTime = 0;
        currentState = State::NO_PRESS;
    }

    void begin() {
        // Initialize buzzer
        buzzer.begin();
        buzzer.beepPattern(3, 100, 100);
    }

    void loop();
    
    // State queries
    State state() const;
    bool isPressing() const;
    bool isPressingRaw() const;

private:
    ezButton button;
    SimpleBuzzer buzzer;
    uint16_t pressStartTime;
    State currentState;
};

#endif // BUTTON_CONTROLLER_H 