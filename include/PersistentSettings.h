#pragma once

#include "Globals.h"
/**
 * @brief Manages persistent settings stored in Non-Volatile Storage (NVS).
 * 
 * This class provides a singleton interface to access and modify settings
 * like the child lock state. It uses the ESP32 Preferences library.
 */
class PersistentSettings {
public:
    /**
     * @brief Gets the singleton instance of PersistentSettings.
     * @return Reference to the PersistentSettings instance.
     */
    static PersistentSettings& getInstance();
    
    // Child lock settings
    /**
     * @brief Checks if the child lock feature is currently enabled.
     * @return True if child lock is enabled, false otherwise.
     */
    bool isChildLockEnabled();

    /**
     * @brief Sets the state of the child lock feature.
     * @param enabled True to enable child lock, false to disable.
     */
    void setChildLockEnabled(bool enabled);
    
    /**
     * @brief Toggles the child lock state (enabled becomes disabled, disabled becomes enabled).
     */
    void toggleChildLock();
    
    // Buzzer settings
    /**
     * @brief Checks if the buzzer is currently enabled.
     * @return True if buzzer is enabled, false otherwise.
     */
    bool isBuzzerEnabled();

    /**
     * @brief Sets the state of the buzzer.
     * @param enabled True to enable buzzer, false to disable.
     */
    void setBuzzerEnabled(bool enabled);

    /**
     * @brief Toggles the buzzer state (enabled becomes disabled, disabled becomes enabled).
     */
    void toggleBuzzer();

    /**
     * @brief Gets the stored RF button code.
     * Retrieves the previously learned RF code for remote control functionality.
     * @return The stored RF button code as a 32-bit unsigned integer.
     */
    uint32_t getRfButtonCode();

    /**
     * @brief Sets the RF button code.
     * Stores a new RF code for remote control functionality.
     * @param code The RF button code to store as a 32-bit unsigned integer.
     */
    void setRfButtonCode(uint32_t code);

    /**
     * @brief Checks if RF functionality is currently enabled.
     * @return True if RF functionality is enabled, false otherwise.
     */
    bool isRFEnabled();

    /**
     * @brief Sets the state of RF functionality.
     * @param enabled True to enable RF functionality, false to disable.
     */
    void setRFEnabled(bool enabled);
    
    /**
     * @brief Clears all settings and resets them to their default values.
     * This includes child lock, buzzer, RF settings, and RF button code.
     */
    void clearAll();
    
    // Initialize settings
    /**
     * @brief Initializes the PersistentSettings manager.
     * This typically involves beginning a session with the Preferences library
     * under a specific namespace. Must be called before using other methods.
     */
    void begin();
    
private:
    /**
     * @brief Private constructor to enforce singleton pattern.
     */
    PersistentSettings() = default;
    /**
     * @brief Private destructor.
     */
    ~PersistentSettings() = default;
    
    // Prevent copying
    PersistentSettings(const PersistentSettings&) = delete;
    PersistentSettings& operator=(const PersistentSettings&) = delete;
    
    static PersistentSettings* instance; ///< Singleton instance pointer.
    Preferences preferences;             ///< ESP32 Preferences object for NVS access.
}; 