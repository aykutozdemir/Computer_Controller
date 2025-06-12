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

private:
    void onBeepComplete();
    void onPatternBeepComplete();

    uint8_t _pin;
    State _state;
    SimpleTimer<unsigned long> _timer;
    uint8_t _patternCount;
    uint8_t _patternIndex;
    uint16_t _offDuration;
};

#endif // SIMPLE_BUZZER_H 