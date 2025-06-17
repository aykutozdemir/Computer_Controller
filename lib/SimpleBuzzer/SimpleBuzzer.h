#ifndef SIMPLE_BUZZER_H
#define SIMPLE_BUZZER_H

#include <Arduino.h>
#include <SimpleTimer.h>

class SimpleBuzzer {
public:
    enum class State {
        IDLE,
        BEEPING,
        PATTERN
    };

    SimpleBuzzer(uint8_t pin);
    void begin();
    void loop();
    void beep(uint16_t duration_ms = 100);
    void beepPattern(uint8_t count = 3, uint16_t on_ms = 100, uint16_t off_ms = 100);
    bool isActive() const;
    void setEnabled(bool enabled);
    bool isEnabled() const;

private:
    void onBeepComplete();
    void onPatternBeepComplete();
    void handlePatternTransition(); // New private method

    uint8_t _pin;
    State _state;
    SimpleTimer<unsigned long> _timer;
    uint8_t _patternTotalBeeps;      // Total number of beeps in the current pattern
    uint8_t _patternBeepsCompleted;  // Number of beeps already completed in the current pattern
    uint16_t _patternOnMs;           // Duration for the ON part of a pattern beep
    uint16_t _patternOffMs;          // Duration for the OFF part of a pattern beep
    bool _isEnabled;
    bool _isPatternPhaseOn;        // True if the current phase of the pattern is ON, false if OFF
};

#endif // SIMPLE_BUZZER_H 