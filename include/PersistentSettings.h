#pragma once

#include "Globals.h"
#include <Preferences.h>
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
    
    static constexpr const char* NAMESPACE = "compCtrl";      ///< Namespace for NVS storage.
    static constexpr const char* KEY_CHILD_LOCK = "childLock"; ///< Key for storing child lock state.
}; 