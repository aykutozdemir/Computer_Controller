#include "SimpleBuzzer.h"

SimpleBuzzer::SimpleBuzzer(uint8_t pin) 
    : _pin(pin)
    , _state(State::IDLE)    
    , _timer(0)  // Initialize timer with 0 interval
    , _isEnabled(true) // Default to enabled
    , _patternTotalBeeps(0)
    , _patternBeepsCompleted(0)
    , _patternOnMs(0)
    , _patternOffMs(0)
    , _isPatternPhaseOn(false)
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
    if (!_isEnabled && _state != State::IDLE) { // If disabled while active, stop it
        digitalWrite(_pin, LOW);
        _state = State::IDLE;
        return; 
    }

    if (_timer.isReady()) {
        switch (_state) {
            case State::BEEPING:
                digitalWrite(_pin, LOW); // Directly handle simple beep completion
                _state = State::IDLE;
                break;
            case State::PATTERN:
                handlePatternTransition(); // Use new handler for pattern state
                break;
            default:
                break;
        }
    }
}

void SimpleBuzzer::beep(uint16_t duration_ms) {
    if (!_isEnabled) return;
    if (_state == State::IDLE) {  // Only start new beep if not already active
        digitalWrite(_pin, HIGH);
        _timer.setInterval(duration_ms);
        _timer.reset();
        _state = State::BEEPING;
    }
}

void SimpleBuzzer::beepPattern(uint8_t count, uint16_t on_ms, uint16_t off_ms) {
    if (!_isEnabled || count == 0) return;
    if (_state == State::IDLE) {  // Only start new pattern if not already active
        _patternTotalBeeps = count;
        _patternBeepsCompleted = 0;
        _patternOnMs = on_ms;
        _patternOffMs = off_ms;

        // Start the first beep
        digitalWrite(_pin, HIGH);
        _timer.setInterval(_patternOnMs);
        _timer.reset();
        _state = State::PATTERN;
        _isPatternPhaseOn = true; // Current phase is ON
    }
}

void SimpleBuzzer::handlePatternTransition() {
    if (_isPatternPhaseOn) { // The ON phase of a beep has just finished
        digitalWrite(_pin, LOW);    // Turn buzzer OFF
        _patternBeepsCompleted++;   // This beep is now fully completed

        if (_patternBeepsCompleted >= _patternTotalBeeps) { // All beeps in the pattern are done
            _state = State::IDLE;
            // No more actions needed
        } else { // Not all beeps are done, so start the OFF phase
            _isPatternPhaseOn = false; // Transition to OFF phase
            _timer.setInterval(_patternOffMs);
            _timer.reset();
            // Buzzer is already LOW, state remains PATTERN
        }
    } else { // The OFF phase (pause) between beeps has just finished
        // Start the next ON phase
        _isPatternPhaseOn = true; // Transition to ON phase
        digitalWrite(_pin, HIGH);   // Turn buzzer ON
        _timer.setInterval(_patternOnMs);
        _timer.reset();
        // State remains PATTERN
    }
}

bool SimpleBuzzer::isActive() const {
    return _state != State::IDLE;
}

void SimpleBuzzer::setEnabled(bool enabled) {
    _isEnabled = enabled;
}

bool SimpleBuzzer::isEnabled() const {
    return _isEnabled;
}