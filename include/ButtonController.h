#pragma once

#include <ezButton.h>
#include "SimpleBuzzer.h"
#include "SimpleTimer.h"
#include "Globals.h"
#include "PersistentSettings.h"

/**
 * @brief Manages a single button, detecting short, long, and very long presses.
 * 
 * This class uses the ezButton library for debouncing and basic press detection,
 * and extends it to differentiate between various press durations.
 * It can optionally interact with a SimpleBuzzer for auditory feedback.
 */
class ButtonController {
public:
    /**
     * @brief Defines the possible states of a button press.
     */
    enum class State {
        NO_PRESS,        ///< Button is not currently pressed.
        SHORT_PRESS,     ///< Button was pressed for a short duration.
        LONG_PRESS,      ///< Button was pressed for a long duration.
        VERY_LONG_PRESS  ///< Button was pressed for a very long duration.
    };

    /**
     * @brief Constructor for ButtonController.
     * @param pin The GPIO pin number to which the button is connected.
     * @param b Reference to a SimpleBuzzer object for auditory feedback.
     */
    ButtonController(uint8_t pin, SimpleBuzzer& b);

    /**
     * @brief Initializes the button controller.
     * Currently, this method does not perform any specific initialization beyond what the constructor does.
     */
    void begin();

    /**
     * @brief Main loop function for the button controller.
     * This function must be called repeatedly in the main loop to update the button's state
     * and detect press types.
     */
    void loop();
    
    /**
     * @brief Gets the current state of the button press.
     * @return The current press state (NO_PRESS, SHORT_PRESS, LONG_PRESS, VERY_LONG_PRESS).
     */
    State state() const;

    /**
     * @brief Checks if the button is currently considered to be in any pressed state (short, long, or very long).
     * @return True if the button is in any pressed state, false otherwise.
     */
    bool isPressing() const;

private:
    ezButton button;                  ///< ezButton instance for handling the raw button input.
    SimpleBuzzer& buzzer;             ///< Reference to a SimpleBuzzer for feedback.
    SimpleTimer<> beepTimer;          ///< Timer for generating auditory feedback.
    unsigned long pressStartTime = 0; ///< Timestamp (from millis()) when the button was initially pressed.
    State currentState;               ///< The current detected press state of the button.
}; 