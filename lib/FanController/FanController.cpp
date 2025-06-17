#include "FanController.h"

// Define log tag
static const char* TAG_FAN_CTRL = "FanCtrl";

// Static member for ISR
static FanController* _instance = nullptr;

FanController::FanController(uint8_t pwmPin, uint8_t tachPin, uint32_t pwmFrequency,
                           uint8_t pwmResolutionBits, uint8_t pulsesPerRevolution, uint32_t rpmUpdateInterval)
    : _pwmPin(pwmPin),
      _tachPin(tachPin),
      _pwmFrequency(pwmFrequency),
      _pwmResolutionBits(pwmResolutionBits),
      _pulsesPerRevolution(pulsesPerRevolution),
      _rpmUpdateInterval(rpmUpdateInterval) {
    _maxDutyCycle = (1 << _pwmResolutionBits) - 1;
    _instance = this;
}

void FanController::begin() {
    // Configure PWM pin
    pinMode(_pwmPin, OUTPUT);
    
    // Configure PWM frequency and resolution
    analogWriteFrequency(_pwmFrequency);
    analogWriteResolution(_pwmResolutionBits);
    
    // Configure tachometer pin with interrupt
    pinMode(_tachPin, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(_tachPin), tachISR, RISING);
    
    // Set initial state to off
    turnOff();
    ESP_LOGI(TAG_FAN_CTRL, "Fan Controller initialized on pin %d (PWM), tach on pin %d, freq %lu Hz, res %d bits. Max duty: %lu",
             _pwmPin, _tachPin, _pwmFrequency, _pwmResolutionBits, _maxDutyCycle);
}

void IRAM_ATTR FanController::tachISR() {
    if (_instance) {
        _instance->_pulseCount++;
    }
}

void FanController::updateRPM() {
    uint32_t currentTime = millis();
    if (currentTime - _lastRPMUpdate >= _rpmUpdateInterval) {
        uint32_t pulseDiff = _pulseCount - _lastPulseCount;
        _lastPulseCount = _pulseCount;
        
        // Calculate RPM: (pulses * 60 * 1000) / (pulses_per_rev * interval_ms)
        _currentRPM = (pulseDiff * 60 * 1000) / (_pulsesPerRevolution * _rpmUpdateInterval);
        _lastRPMUpdate = currentTime;
        
        ESP_LOGD(TAG_FAN_CTRL, "Fan RPM: %u", _currentRPM);
    }
}

void FanController::setSpeed(uint8_t percentage) {
    if (percentage > 100) {
        percentage = 100;
        ESP_LOGW(TAG_FAN_CTRL, "Fan speed percentage capped at 100%%.");
    }

    _currentSpeed = percentage;
    
    if (percentage > 0) {
        // Map percentage (0-100) to PWM duty cycle (0-_maxDutyCycle)
        uint32_t dutyCycle = map(percentage, 0, 100, 0, _maxDutyCycle);
        
        // Set PWM duty cycle using analogWrite
        analogWrite(_pwmPin, dutyCycle);
        _isOn = true;
        
        ESP_LOGD(TAG_FAN_CTRL, "Set fan speed to %u%% (Duty Cycle: %lu / %lu)", percentage, dutyCycle, _maxDutyCycle);
    } else {
        // Turn off the fan if speed is 0
        turnOff();
    }
}

void FanController::turnOn() {
    setSpeed(100);
}

void FanController::turnOff() {
    // Turn off PWM output
    analogWrite(_pwmPin, 0);
    _isOn = false;
    _currentSpeed = 0;
    ESP_LOGD(TAG_FAN_CTRL, "Fan turned off");
}

void FanController::loop() {
    updateRPM();
}

uint8_t FanController::getSpeed() const {
    return _currentSpeed;
}

bool FanController::isEnabled() const {
    return _isOn;
}

uint16_t FanController::getRPM() const {
    return _currentRPM;
}

uint32_t FanController::getPulseCount() const {
    return _pulseCount;
}