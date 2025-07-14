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

    /**
     * @brief Get current signal strength counter.
     * 
     * @return Current signal strength value (0 to RF_MIN_SIGNAL_STRENGTH).
     */
    uint16_t getSignalStrength() const { return signalStrength; }

    /**
     * @brief Check if the last signal was validated.
     * 
     * @return true if the last signal passed validation, false otherwise.
     */
    bool isSignalValidated() const { return signalValidated; }

    /**
     * @brief Get signal quality statistics.
     * 
     * @param totalSignals Reference to store total signals received.
     * @param validSignals Reference to store valid signals count.
     * @param noiseSignals Reference to store noise signals count.
     */
    void getSignalStats(uint32_t& totalSignals, uint32_t& validSignals, uint32_t& noiseSignals) const;

    /**
     * @brief Reset signal statistics and validation state.
     */
    void resetSignalStats();

    /**
     * @brief Enable or disable fallback mode for better signal reception.
     * @param enabled True to enable fallback mode, false to disable.
     */
    void setFallbackMode(bool enabled);

    /**
     * @brief Check if fallback mode is currently enabled.
     * @return true if fallback mode is enabled, false otherwise.
     */
    bool isFallbackModeEnabled() const { return fallbackMode; }

private:
    uint8_t interruptPin;    ///< GPIO pin number for the RF receiver data output
    RCSwitch rcSwitch;       ///< Instance of the RCSwitch library

    uint32_t lastCode{0};    ///< The last received RF code
    bool newCodeAvailable{false};  ///< Flag indicating if a new code is available

    // Basic debounce so that the same code is not reported more than once
    // within RF_REPEAT_DELAY milliseconds
    unsigned long lastReportTime = 0;  ///< Timestamp of the last reported code
    
    // Signal validation and noise filtering
    uint32_t signalValidationBuffer[RF_SIGNAL_VALIDATION_COUNT];  ///< Buffer for signal validation
    uint8_t signalValidationIndex = 0;  ///< Current index in validation buffer
    unsigned long lastSignalTime = 0;    ///< Timestamp of last signal
    uint16_t signalStrength = 0;         ///< Current signal strength counter
    bool signalValidated = false;        ///< Flag indicating if signal passed validation
    
    // Signal statistics for monitoring
    uint32_t totalSignalsReceived = 0;   ///< Total signals received
    uint32_t validSignalsCount = 0;      ///< Count of valid signals
    uint32_t noiseSignalsCount = 0;      ///< Count of noise signals
    
    // Fallback mode for poor signal conditions
    bool fallbackMode = false;           ///< Flag to enable more lenient signal processing
    unsigned long lastValidSignalTime = 0; ///< Timestamp of last valid signal
}; 