#include "SimpleBuzzer.h"

SimpleBuzzer::SimpleBuzzer(uint8_t pin) 
    : _pin(pin)
    , _state(State::IDLE)
    , _patternCount(0)
    , _patternIndex(0)
    , _offDuration(0)
    , _timer(0)  // Initialize timer with 0 interval
{
    pinMode(_pin, OUTPUT);
    digitalWrite(_pin, LOW);
}

void SimpleBuzzer::begin() {
    // Ensure pin is properly configured
    pinMode(_pin, OUTPUT);
    digitalWrite(_pin, LOW);
}

void SimpleBuzzer::loop() {
    if (_timer.isReady()) {
        switch (_state) {
            case State::BEEPING:
                onBeepComplete();
                break;
            case State::PATTERN:
                onPatternBeepComplete();
                break;
            default:
                break;
        }
    }
}

void SimpleBuzzer::beep(uint16_t duration_ms) {
    if (_state == State::IDLE) {  // Only start new beep if not already active
        digitalWrite(_pin, HIGH);
        _timer.setInterval(duration_ms);
        _timer.reset();
        _state = State::BEEPING;
    }
}

void SimpleBuzzer::beepPattern(uint8_t count, uint16_t on_ms, uint16_t off_ms) {
    if (_state == State::IDLE) {  // Only start new pattern if not already active
        _patternCount = count;
        _patternIndex = 0;
        _offDuration = off_ms;
        digitalWrite(_pin, HIGH);
        _timer.setInterval(on_ms);
        _timer.reset();
        _state = State::PATTERN;
    }
}

void SimpleBuzzer::onBeepComplete() {
    digitalWrite(_pin, LOW);
    _state = State::IDLE;
}

void SimpleBuzzer::onPatternBeepComplete() {
    digitalWrite(_pin, LOW);
    _patternIndex++;
    
    if (_patternIndex >= _patternCount) {
        _state = State::IDLE;
    } else {
        _timer.setInterval(_offDuration);
        _timer.reset();
        // Schedule the next beep
        _state = State::PATTERN;
        digitalWrite(_pin, HIGH);
    }
}

bool SimpleBuzzer::isActive() const {
    return _state != State::IDLE;
} 