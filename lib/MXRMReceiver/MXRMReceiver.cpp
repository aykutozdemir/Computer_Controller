#include "MXRMReceiver.h"

MXRMReceiver::MXRMReceiver(uint8_t interruptPin) :
    _interruptPin(interruptPin),
    _lastValue(false),
    _hasChanged(false),
    _buttonCode(0),
    _newButtonCode(false),
    _lastChangeTime(0),
    _pulseCount(0),
    _totalPulseTime(0)
{
}

void MXRMReceiver::begin()
{
    // Configure pin as input with pull-up
    pinMode(_interruptPin, INPUT_PULLUP);
}

bool MXRMReceiver::read()
{
    _update();
    return _lastValue;
}

bool MXRMReceiver::hasChanged() const
{
    return _hasChanged;
}

uint32_t MXRMReceiver::getButtonCode()
{
    _newButtonCode = false;
    return _buttonCode;
}

bool MXRMReceiver::isNewButtonCode()
{
    return _newButtonCode;
}

void MXRMReceiver::_update()
{
    bool currentValue = digitalRead(_interruptPin);
    unsigned long currentTime = millis();
    
    if (currentValue != _lastValue) {
        unsigned long pulseTime = currentTime - _lastChangeTime;
        _lastChangeTime = currentTime;
        
        if (pulseTime > 100) {  // Ignore noise
            _pulseCount++;
            _totalPulseTime += pulseTime;
            
            // If we've received enough pulses, calculate the button code
            if (_pulseCount >= 24) {  // EV1527 protocol uses 24 bits
                _buttonCode = _totalPulseTime / _pulseCount;  // Simple encoding for now
                _newButtonCode = true;
                _pulseCount = 0;
                _totalPulseTime = 0;
            }
        }
    }
    
    _hasChanged = (currentValue != _lastValue);
    _lastValue = currentValue;
}