#ifndef POWER_RESET_CONTROLLER_H
#define POWER_RESET_CONTROLLER_H

#include <ezButton.h>
#include "Globals.h"

/**
 * @brief Manages physical power and reset buttons.
 *
 * This class uses the ezButton library for debouncing and detecting button presses
 * for dedicated power and reset input buttons.
 */
class PowerResetController {
public:
    /**
     * @brief Constructs the PowerResetController.
     * Initializes button objects with their respective GPIO pins (POWER_BUTTON_PIN, RESET_BUTTON_PIN)
     * and sets them to INPUT_PULLUP mode.
     */
    PowerResetController();

    /**
     * @brief Completes initialization of the buttons.
     * Sets debounce time (using DEBOUNCE_TIME from Globals.h) and count mode.
     * This method should be called once during the setup phase.
     */
    void begin();

    /**
     * @brief Updates the internal state of the power and reset buttons.
     * This method must be called repeatedly in the main application loop.
     */
    void loop();

    /**
     * @brief Checks if the physical power button is currently pressed.
     * @return True if the power button is pressed, false otherwise.
     */
    bool isPowerPressed() const;

    /**
     * @brief Checks if the physical reset button is currently pressed.
     * @return True if the reset button is pressed, false otherwise.
     */
    bool isResetPressed() const;

private:
    ezButton powerButton; ///< ezButton instance for the power button.
    ezButton resetButton; ///< ezButton instance for the reset button.
};

#endif // POWER_RESET_CONTROLLER_H 