#pragma once

#include "Globals.h"
#include "DisplayManager.h"
#include "ButtonController.h"
#include "PowerResetController.h"
#include "CommandHandler.h"
#include "LedController.h"
#include "RFStudyManager.h"
#include "RCSwitchReceiver.h"

#include <FanController.h>

/**
 * @brief Main controller class for the Computer Controller project.
 * 
 * Orchestrates all components including command handling, display management,
 * button inputs, WiFi connectivity, and interactions with the connected computer.
 * This class is marked as final and cannot be inherited from.
 */
class ComputerController final {
public:    
    /**
     * @brief Constructor for ComputerController.
     * Initializes all member objects and dependencies.
     */
    ComputerController();
    /**
     * @brief Performs setup operations for the controller and its components.
     * Must be called once at startup.
     */
    void setup();
    /**
     * @brief Main loop function for the controller.
     * Must be called repeatedly to process events and update states.
     */
    void loop();
    /**
     * @brief Initiates a reset sequence.
     * (Clarify if this resets the ESP32 or the controlled computer).
     */
    void reset();
    
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
     * @brief Updates the content displayed on the screen.
     */
    void updateDisplay();

    /**
     * @brief Activates the power relay for a predefined duration.
     */
    void activatePowerRelay();
    
    /**
     * @brief Activates the reset relay for a predefined duration.
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

    /* ---------------------------------------------------------------------
     * Environmental sensor accessors
     * ------------------------------------------------------------------*/

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

private:
    // Command channels
    WiFiClientSecure telegramClient;        ///< Secure client for Telegram communication.
    UniversalTelegramBot telegramBot;       ///< Telegram bot instance.

    // Command handler
    CommandHandler* commandHandler;         ///< Pointer to the command handler instance.

    // Timers
    SimpleTimer<> wifiCheckTimer;           ///< Timer for periodic WiFi status checks.
    SimpleTimer<> debugTimer;               ///< Timer for periodic debug outputs.
    SimpleTimer<> displayUpdateTimer;       ///< Timer for display refresh intervals.
    SimpleTimer<> rfCheckTimer;             ///< Timer for RF receiver checks.
    SimpleTimer<> relayTimer;               ///< Timer for managing relay press durations.

    // Hardware controllers
    WiFiManager wifiManager;                ///< WiFiManager instance for network configuration.
    ESP32Time rtc;                          ///< Real-time clock instance.
    DisplayManager display;                 ///< Display manager instance.
    SimpleBuzzer buzzer;                    ///< Buzzer for auditory feedback.
    ButtonController buttons;               ///< Controller for the main user button.
    PowerResetController powerReset;        ///< Controller for physical power/reset buttons.
    PersistentSettings& settings;           ///< Reference to persistent settings manager.
    LedController led;                      ///< LED controller instance.
    RCSwitchReceiver rfReceiver;            ///< RF receiver instance.
    RFStudyManager rfStudyManager;          ///< RF study manager instance.
    FanController gpuFan;                   ///< GPU fan controller

    // Environmental sensors
    DHT11Sensor dht11;                      ///< Temperature/Humidity sensor (DHT11)

    bool isConnected = false;               ///< Flag indicating WiFi connection status.
    
    // Relay control state
    /**
     * @brief Defines the possible states for relay control operations.
     */
    enum class RelayState {
        IDLE,           ///< Relays are idle.
        POWER_PRESSING, ///< Power relay is currently being activated.
        RESET_PRESSING  ///< Reset relay is currently being activated.
    };
    RelayState currentRelayState = RelayState::IDLE; ///< Current state of the relay operations.

    void connectWiFi();
    void handleRFInput();  // New method to handle RF input
    void handlePowerResetButtons();  // New method to handle power and reset buttons
    void setPowerRelay(bool state);  // New method to control power relay
    void setResetRelay(bool state);  // New method to control reset relay
    void updateRelayState();  // New method to handle relay state machine

    // Static task runner for peripheral handling
    static void peripheralTaskRunner(void* pvParameters);

    void handleFactoryReset();
}; 