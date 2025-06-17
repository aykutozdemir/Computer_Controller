#pragma once

#include "RCSwitchReceiver.h"
#include "PersistentSettings.h"
#include <functional>

/**
 * @brief Manages RF study functionality for the Computer Controller.
 * 
 * This class handles the RF study process, including listening for new RF codes
 * and managing callbacks for RF detection events. It provides a high-level interface
 * for learning and storing RF codes from remote controls.
 */
class RFStudyManager {
public:
    /**
     * @brief Callback function type for RF code detection.
     * 
     * This function type is used to notify the caller when a new RF code is detected
     * during the study process.
     * 
     * @param code The detected RF code as a 32-bit unsigned integer.
     */
    using RFCodeCallback = std::function<void(uint32_t code)>;

    /**
     * @brief Construct a new RF Study Manager instance.
     * 
     * @param rfReceiver Reference to the RF receiver instance that will be used
     *                   to detect RF codes.
     * @param settings Reference to persistent settings where learned codes will be stored.
     */
    RFStudyManager(RCSwitchReceiver& rfReceiver, PersistentSettings& settings);

    /**
     * @brief Start listening for RF codes.
     * 
     * Initiates the RF study process. The manager will listen for incoming RF codes
     * until either a code is detected, the timeout is reached, or stopListening() is called.
     * 
     * @param timeoutMs Timeout in milliseconds. If 0, no timeout is applied.
     * @param callback Optional callback function to be called when a code is detected.
     * @return true if listening started successfully, false if already listening.
     */
    bool startListening(uint32_t timeoutMs = 0, RFCodeCallback callback = nullptr);

    /**
     * @brief Stop listening for RF codes.
     * 
     * Terminates the current RF study process. This method can be called at any time
     * to stop listening for codes, even if a timeout was specified.
     */
    void stopListening();

    /**
     * @brief Check if currently listening for RF codes.
     * 
     * @return true if the manager is currently in listening mode,
     *         false otherwise.
     */
    bool isListening() const { return listening; }

    /**
     * @brief Get the currently stored RF code.
     * 
     * Retrieves the last successfully learned RF code from persistent storage.
     * 
     * @return The stored RF code as a 32-bit unsigned integer.
     *         Returns 0 if no code has been stored.
     */
    uint32_t getStoredCode() const;

    /**
     * @brief Clear the stored RF code.
     * 
     * Removes the currently stored RF code from persistent storage.
     * This effectively disables the RF control functionality until
     * a new code is learned.
     */
    void clearStoredCode();

    /**
     * @brief Process RF input.
     * 
     * This method must be called regularly in the main loop to process
     * incoming RF signals and handle timeouts. It checks for new codes
     * and manages the study process state.
     */
    void process();

private:
    RCSwitchReceiver& rfReceiver;    ///< Reference to the RF receiver instance
    PersistentSettings& settings;     ///< Reference to persistent settings storage
    bool listening = false;           ///< Current listening state
    uint32_t startTime = 0;          ///< Timestamp when listening started
    uint32_t timeout = 0;            ///< Timeout duration in milliseconds
    RFCodeCallback callback = nullptr; ///< Callback function for code detection
}; 