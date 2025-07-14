#pragma once

// Core dependencies
#include "Globals.h"

// Arduino and ESP32 libraries
#include <Arduino_GFX_Library.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <WiFiManager.h>
#include <ESP32Time.h>

// Project-specific includes
#include "LCDCache.h"
#include "ButtonController.h"
#include "PowerResetController.h"
#include "CommandHandler.h"
#include "LedController.h"
#include "RFStudyManager.h"
#include "RCSwitchReceiver.h"
#include "MQTTManager.h"
#include "WebServerManager.h"
#include <FanController.h>
#include <DHT11Sensor.h>
#include <SimpleBuzzer.h>
#include <SimpleTimer.h>
#include "ICacheDisplay.h"

// Forward declarations
class ComputerControllerUI;

/**
 * @brief Main controller class for the Computer Controller project.
 * 
 * This class orchestrates all components including:
 * - Command handling (Telegram, Serial, MQTT)
 * - Display management and UI
 * - Hardware control (relays, fans, sensors)
 * - WiFi connectivity and network services
 * - Button input processing
 * - Environmental monitoring
 * 
 * The class is designed as a singleton-like controller that manages
 * the entire system lifecycle and provides a unified interface for
 * all functionality.
 */
class ComputerController final {
public:
    // =====================================================================
    // CONSTRUCTOR & LIFECYCLE METHODS
    // =====================================================================
    
    /**
     * @brief Constructor initializes all member objects and dependencies.
     */
    ComputerController();
    
    /**
     * @brief Performs complete system setup and initialization.
     * Must be called once at startup before loop().
     * 
     * Initialization order:
     * 1. Display system (before any network code)
     * 2. Hardware controllers (buttons, sensors, etc.)
     * 3. Network & communication (WiFi, MQTT, etc.)
     * 4. UI system (if display available)
     * 5. Task creation
     */
    void setup();
    
    /**
     * @brief Main event processing loop.
     * Must be called repeatedly to handle events and update system state.
     */
    void loop();
    
    /**
     * @brief Performs a complete ESP32 reset with WiFi cleanup.
     */
    void reset();
    
    /**
     * @brief Destructor to clean up resources.
     */
    ~ComputerController();

    // =====================================================================
    // HARDWARE CONTROL METHODS
    // =====================================================================
    
    /**
     * @brief Activates the power relay for a predefined duration.
     * Used for power on/off operations of the controlled computer.
     */
    void activatePowerRelay();
    
    /**
     * @brief Activates the reset relay for a predefined duration.
     * Used for hard reset operations of the controlled computer.
     */
    void activateResetRelay();
    
    /**
     * @brief Sets the GPU fan speed.
     * @param speed Speed value between 0 (off) and 100 (full speed).
     * @return true if the speed was set successfully, false otherwise.
     */
    bool setGpuFanSpeed(uint8_t speed);
    
    /**
     * @brief Gets the current GPU fan speed.
     * @return Current fan speed (0-100).
     */
    uint8_t getGpuFanSpeed() const;
    
    /**
     * @brief Checks if the GPU fan is enabled.
     * @return true if the fan is enabled, false otherwise.
     */
    bool isGpuFanEnabled() const { return gpuFan.isEnabled(); }
    
    /**
     * @brief Gets the current GPU fan RPM.
     * @return Current fan RPM.
     */
    uint16_t getGpuFanRPM() const { return gpuFan.getRPM(); }

    // =====================================================================
    // PC CONTROL CONVENIENCE METHODS
    // =====================================================================
    
    /**
     * @brief Power on the PC via relay.
     */
    void powerOnPC() { activatePowerRelay(); }
    
    /**
     * @brief Power off the PC via relay.
     */
    void powerOffPC() { activatePowerRelay(); }
    
    /**
     * @brief Reset the PC via relay.
     */
    void resetPC() { activateResetRelay(); }
    
    /**
     * @brief Gets the current PC power status.
     * @return true if PC is powered on, false if powered off.
     */
    bool isPCPoweredOn() const { return digitalRead(PC_POWERED_ON_PIN) == HIGH; }

    // =====================================================================
    // ENVIRONMENTAL SENSOR ACCESSORS
    // =====================================================================
    
    /**
     * @brief Returns the most recent ambient temperature value.
     * @return Temperature in °C, or NAN if not available.
     */
    float getAmbientTemperature() const { return dht11.getTemperature(); }
    
    /**
     * @brief Returns the most recent relative-humidity value.
     * @return Humidity in percent, or NAN if not available.
     */
    float getRelativeHumidity() const { return dht11.getHumidity(); }

    // =====================================================================
    // SETTINGS MANAGEMENT METHODS
    // =====================================================================
    
    /**
     * @brief Toggle child lock setting.
     */
    void toggleChildLock() { settings.toggleChildLock(); }
    
    /**
     * @brief Check if child lock is enabled.
     * @return true if child lock is enabled, false otherwise.
     */
    bool isChildLockEnabled() const { return settings.isChildLockEnabled(); }
    
    /**
     * @brief Toggle buzzer setting.
     */
    void toggleBuzzer() { 
        settings.toggleBuzzer(); 
        buzzer.setEnabled(settings.isBuzzerEnabled());
    }
    
    /**
     * @brief Check if buzzer is enabled.
     * @return true if buzzer is enabled, false otherwise.
     */
    bool isBuzzerEnabled() const { return settings.isBuzzerEnabled(); }

    // =====================================================================
    // TIME MANAGEMENT METHODS
    // =====================================================================
    
    /**
     * @brief Synchronizes time with NTP servers.
     * @return true if synchronization was successful, false otherwise.
     */
    bool syncTimeWithNTP();
    
    /**
     * @brief Gets the current time as a formatted string.
     * @return Current time string in format "HH:MM:SS".
     */
    String getCurrentTimeString();
    
    /**
     * @brief Gets the MQTT connection status.
     * @return true if connected to MQTT broker, false otherwise.
     */
    bool isMQTTConnected() const { return mqttManager.isConnectedToBroker(); }
    


    // =====================================================================
    // COMPONENT ACCESSORS
    // =====================================================================
    
    /**
     * @brief Gets a reference to the UniversalTelegramBot instance.
     * @return Reference to the UniversalTelegramBot object.
     */
    UniversalTelegramBot& getTelegramBot() { return telegramBot; }
    
    /**
     * @brief Gets a reference to the WiFiClientSecure instance used for Telegram.
     * @return Reference to the WiFiClientSecure object.
     */
    WiFiClientSecure& getTelegramClient() { return telegramClient; }
    
    /**
     * @brief Gets a reference to the SimpleBuzzer instance.
     * @return Reference to the SimpleBuzzer object.
     */
    SimpleBuzzer& getBuzzer() { return buzzer; }
    
    /**
     * @brief Gets a reference to the unified display interface.
     * @return Reference to the ICacheDisplay object.
     */
    ICacheDisplay& getDisplay() { return *display; }
    
    /**
     * @brief Gets a reference to the RCSwitchReceiver instance.
     * @return Reference to the RCSwitchReceiver object.
     */
    RCSwitchReceiver& getRCSwitchReceiver() { return rfReceiver; }
    
    /**
     * @brief Gets a reference to the RFStudyManager instance.
     * @return Reference to the RFStudyManager object.
     */
    RFStudyManager& getRFStudyManager() { return rfStudyManager; }
    
    /**
     * @brief Gets a reference to the LedController instance.
     * @return Reference to the LedController object.
     */
    LedController& getLed() { return led; }
    
    /**
     * @brief Gets a reference to the ButtonController instance.
     * @return Reference to the ButtonController object.
     */
    ButtonController& getButtons() { return button; }

private:
    // =====================================================================
    // PRIVATE ENUMS AND TYPES
    // =====================================================================
    
    /**
     * @brief Defines the possible states for relay control operations.
     */
    enum class RelayState {
        IDLE,           ///< Relays are idle and ready for new operations.
        POWER_ACTIVE,   ///< Power relay is currently being activated.
        RESET_ACTIVE    ///< Reset relay is currently being activated.
    };

    // =====================================================================
    // PRIVATE MEMBER VARIABLES - COMMUNICATION
    // =====================================================================
    
    WiFiClientSecure telegramClient;        ///< Secure client for Telegram communication.
    UniversalTelegramBot telegramBot;       ///< Telegram bot instance.
    CommandHandler* commandHandler;         ///< Pointer to the command handler instance.
    MQTTManager mqttManager;                ///< MQTT manager instance.
    WebServerManager* webServerManager;     ///< Web server manager instance (only when connected).

    // =====================================================================
    // PRIVATE MEMBER VARIABLES - DISPLAY SYSTEM
    // =====================================================================
    
    ICacheDisplay* display;                 ///< Unified display interface (cache or direct).
    ComputerControllerUI* ui;               ///< UI manager instance.

    // =====================================================================
    // PRIVATE MEMBER VARIABLES - TIMERS
    // =====================================================================
    
    SimpleTimer<unsigned long> wifiCheckTimer;        ///< Timer for WiFi status checks.
    SimpleTimer<unsigned long> debugTimer;            ///< Timer for debug output.
    SimpleTimer<unsigned long> displayUpdateTimer;    ///< Timer for display updates.
    SimpleTimer<unsigned long> rfCheckTimer;          ///< Timer for RF receiver checks.
    SimpleTimer<unsigned long> relayTimer;            ///< Timer for relay operations.

    // =====================================================================
    // PRIVATE MEMBER VARIABLES - HARDWARE CONTROLLERS
    // =====================================================================
    
    WiFiManager wifiManager;                ///< WiFiManager instance for network configuration.
    ESP32Time rtc;                          ///< Real-time clock instance.
    SimpleBuzzer buzzer;                    ///< Buzzer for auditory feedback.
    ButtonController button;               ///< Controller for the main user button.
    PowerResetController powerReset;        ///< Controller for physical power/reset buttons.
    LedController led;                      ///< LED controller instance.
    RCSwitchReceiver rfReceiver;            ///< RF receiver instance.
    RFStudyManager rfStudyManager;          ///< RF study manager instance.
    FanController gpuFan;                   ///< GPU fan controller.
    DHT11Sensor dht11;                      ///< Temperature/Humidity sensor (DHT11).

    // =====================================================================
    // PRIVATE MEMBER VARIABLES - STATE MANAGEMENT
    // =====================================================================
    
    PersistentSettings& settings;           ///< Reference to persistent settings manager.
    bool isConnected = false;               ///< Flag indicating WiFi connection status.
    RelayState currentRelayState = RelayState::IDLE; ///< Current state of the relay operations.
    bool wasInSetupMode = false;            ///< Flag to track if device was in WiFi setup mode.
    bool portalActive = false;              ///< Flag indicating configuration portal is active.

    // =====================================================================
    // PRIVATE METHODS - HARDWARE CONTROL
    // =====================================================================
    
    /**
     * @brief Handles RF input processing and code matching.
     */
    void handleRFInput();
    
    /**
     * @brief Processes power and reset button inputs from PowerResetController.
     */
    void handlePowerResetButtons();
    
    /**
     * @brief Sets the power relay state.
     * @param state true to activate, false to deactivate.
     */
    void setPowerRelay(bool state);
    
    /**
     * @brief Sets the reset relay state.
     * @param state true to activate, false to deactivate.
     */
    void setResetRelay(bool state);
    
    /**
     * @brief Updates the relay state machine and handles timing.
     */
    void updateRelayState();

    // =====================================================================
    // PRIVATE METHODS - SYSTEM MANAGEMENT
    // =====================================================================
    
    /**
     * @brief Static task runner for peripheral handling on separate core.
     * @param pvParameters Pointer to ComputerController instance.
     */
    static void peripheralTaskRunner(void* pvParameters);
    
    /**
     * @brief Performs factory reset by clearing all settings and restarting.
     */
    void handleFactoryReset();
    
    /**
     * @brief Fills the LCD with colored dots for testing display functionality.
     * Bypasses the UI system and directly uses the Arduino GFX driver.
     */
    void fillLCDWithColoredDots();

    // =====================================================================
    // PRIVATE METHODS - NETWORK MANAGEMENT
    // =====================================================================

    bool tryStaConnect(uint16_t timeoutSeconds = 15);
    void startConfigPortal();
    void handleWiFiModeTransition();
    bool setupAPMode();
    bool tryDirectWiFiConnection();
    void connectWiFi();
    bool hasStoredCredentials();
    

}; 