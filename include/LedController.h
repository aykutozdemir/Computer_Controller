/**
 * @file LedController.h
 * @brief Header file for the LED controller class that manages LED status indication
 */

#ifndef LED_CONTROLLER_H
#define LED_CONTROLLER_H

#include <Arduino.h> // For uint8_t
#include <ezLED.h>
#include "Globals.h"

/**
 * @class LedController
 * @brief Controls an LED to indicate different system states
 * 
 * This class provides functionality to control an LED for status indication.
 * It supports different states like OFF, CONNECTING (blinking), and CONNECTED (solid on).
 * The class uses the ezLed library for LED control.
 */
class LedController {
public:
    /**
     * @enum Status
     * @brief Different states that the LED can represent
     */
    enum class Status {
        OFF,        ///< LED is turned off
        CONNECTING, ///< LED is blinking to indicate connection in progress
        CONNECTED   ///< LED is solid on to indicate successful connection
    };

    /**
     * @brief Construct a new LED Controller
     * @param pin The GPIO pin number where the LED is connected
     */
    LedController(uint8_t pin);

    /**
     * @brief Initialize the LED controller
     * Must be called before using other methods
     */
    void begin();

    /**
     * @brief Update the LED state
     * Should be called regularly in the main loop
     */
    void loop();

    /**
     * @brief Set the current status of the LED
     * @param newStatus The new status to set
     */
    void setStatus(Status newStatus);

    /**
     * @brief Get the current status of the LED
     * @return The current status
     */
    Status getStatus() const;

private:
    ezLED _led;           ///< Instance of ezLed for LED control
    Status _currentStatus; ///< Current status of the LED

    /**
     * @brief Apply the current status to the LED
     * Internal method to update the LED state based on current status
     */
    void applyStatus();
};

#endif // LED_CONTROLLER_H