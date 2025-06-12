#pragma once

#include "Globals.h"
#include "CommandHandler.h"
#include "DisplayManager.h"
#include "ButtonController.h"
#include "PowerResetController.h"
#include "PersistentSettings.h"
#include "MXRMReceiver.h"

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
    MXRMReceiver rfReceiver;                ///< RF receiver instance.

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
}; 