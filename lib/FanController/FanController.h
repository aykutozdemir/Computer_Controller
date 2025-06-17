#ifndef FAN_CONTROLLER_H
#define FAN_CONTROLLER_H

#include <Arduino.h>

/**
 * @class FanController
 * @brief Controls a fan using PWM control for both on/off and speed control
 * 
 * This class provides functionality to control a fan using PWM control.
 * It supports speed control from 0% to 100% and basic on/off control.
 * The PWM pin is used for both power control and speed control.
 * RPM measurement is supported through a tachometer input pin.
 */
class FanController {
public:
    /**
     * @brief Construct a new Fan Controller
     * @param pwmPin The GPIO pin number for PWM control
     * @param tachPin The GPIO pin number for tachometer input
     * @param pwmFrequency PWM frequency in Hz (typically 25000 for fans)
     * @param pwmResolutionBits PWM resolution in bits (typically 8)
     * @param pulsesPerRevolution Number of pulses per revolution (typically 2 for PC fans)
     * @param rpmUpdateInterval Interval in milliseconds to update RPM reading (typically 1000)
     */
    FanController(uint8_t pwmPin, uint8_t tachPin, uint32_t pwmFrequency = 25000,
                  uint8_t pwmResolutionBits = 8, uint8_t pulsesPerRevolution = 2, uint32_t rpmUpdateInterval = 1000);

    /**
     * @brief Initialize the fan controller
     * Must be called before using other methods
     */
    void begin();

    /**
     * @brief Set the fan speed
     * @param percentage Speed from 0 to 100
     */
    void setSpeed(uint8_t percentage);

    /**
     * @brief Turn the fan on at full speed
     */
    void turnOn();

    /**
     * @brief Turn the fan off
     */
    void turnOff();

    /**
     * @brief Update the fan state and RPM measurement
     * Should be called regularly in the main loop
     */
    void loop();

    /**
     * @brief Get the current fan speed
     * @return Current speed (0-100)
     */
    uint8_t getSpeed() const;

    /**
     * @brief Check if the fan is enabled
     * @return true if fan speed is greater than 0
     */
    bool isEnabled() const;

    /**
     * @brief Get the current fan RPM
     * @return Current RPM (0 if fan is off or no pulses detected)
     */
    uint16_t getRPM() const;

    /**
     * @brief Get the number of pulses counted in the last measurement period
     * @return Number of pulses
     */
    uint32_t getPulseCount() const;

private:
    uint8_t _pwmPin;          ///< PWM output pin for speed control
    uint8_t _tachPin;         ///< Tachometer input pin for RPM measurement
    uint32_t _pwmFrequency;   ///< PWM frequency in Hz
    uint8_t _pwmResolutionBits;///< PWM resolution in bits
    uint32_t _maxDutyCycle;   ///< Maximum PWM duty cycle value
    uint8_t _currentSpeed = 0;///< Current fan speed (0-100)
    bool _isOn = false;       ///< Current on/off state
    
    // RPM measurement variables
    uint8_t _pulsesPerRevolution;  ///< Number of pulses per revolution
    volatile uint32_t _pulseCount = 0;  ///< Counter for tachometer pulses
    uint32_t _lastPulseCount = 0;  ///< Last pulse count for RPM calculation
    uint32_t _lastRPMUpdate = 0;   ///< Last time RPM was calculated
    uint16_t _currentRPM = 0;      ///< Current RPM value
    uint32_t _rpmUpdateInterval;   ///< Interval for RPM updates in ms
        
    /**
     * @brief Interrupt service routine for tachometer pin
     * Increments the pulse counter when a rising edge is detected
     */
    static void IRAM_ATTR tachISR();
    
    /**
     * @brief Update RPM calculation based on pulse count
     */
    void updateRPM();
};

#endif // FAN_CONTROLLER_H