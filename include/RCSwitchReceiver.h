/*
 * RCSwitchReceiver.h - Lightweight wrapper around the RCSwitch library
 *
 * This class provides a very small abstraction that mimics the interface
 * that the project previously used for MXRMReceiver so that existing code
 * in ComputerController.cpp can remain unchanged.
 *
 * Author: ComputerController project maintainers
 */
#pragma once

#include "Globals.h"

/**
 * @brief A lightweight wrapper around the RCSwitch library for RF receiver functionality.
 * 
 * This class provides a simplified interface for receiving RF codes from remote controls.
 * It handles debouncing and provides a clean interface for checking and retrieving received codes.
 */
class RCSwitchReceiver {
public:
    /**
     * @brief Construct a new RCSwitchReceiver instance.
     * 
     * @param interruptPin GPIO pin connected to the RF receiver data output.
     *                     This pin must be interrupt-capable on the ESP32.
     */
    explicit RCSwitchReceiver(uint8_t interruptPin);

    /**
     * @brief Initialize the RF receiver and attach the interrupt handler.
     * 
     * This method must be called once during setup to configure the receiver
     * and attach the interrupt service routine to the specified pin.
     */
    void begin();

    /**
     * @brief Poll for new RF codes.
     * 
     * This method must be called regularly from the main loop to check for
     * new RF codes. It implements debouncing to prevent multiple triggers
     * from the same code within a short time period.
     * 
     * @return true if a new, debounced code was received since the last call,
     *         false otherwise.
     */
    bool read();

    /**
     * @brief Get the last successfully decoded RF code.
     * 
     * This method returns the most recently received RF code and clears
     * the new-data flag. The code is only returned once per reception.
     * 
     * @return The 32-bit RF code that was last received.
     */
    uint32_t getButtonCode();

    /**
     * @brief Check if a new button code is available.
     * 
     * This method indicates whether a new RF code has been received and
     * is waiting to be read via getButtonCode().
     * 
     * @return true if a new code is available, false otherwise.
     */
    bool isNewButtonCode() const { return newCodeAvailable; }

private:
    uint8_t interruptPin;    ///< GPIO pin number for the RF receiver data output
    RCSwitch rcSwitch;       ///< Instance of the RCSwitch library

    uint32_t lastCode{0};    ///< The last received RF code
    bool newCodeAvailable{false};  ///< Flag indicating if a new code is available

    // Basic debounce so that the same code is not reported more than once
    // within RF_REPEAT_DELAY milliseconds
    unsigned long lastReportTime = 0;  ///< Timestamp of the last reported code
}; 