#pragma once

#include "Globals.h"

class ComputerController;

/**
 * @brief Enumerates the types of commands that can be processed.
 */
enum CommandType_t {
    CMD_TYPE_POWER,       ///< Command to simulate a power button press.
    CMD_TYPE_RESET,       ///< Command to simulate a reset button press.
    CMD_TYPE_STATUS,      ///< Command to get the current system status.
    CMD_TYPE_HELP,        ///< Command to display available commands.
    CMD_TYPE_CHILD_LOCK   ///< Command to enable or disable child lock.
};

/**
 * @brief Handles command processing from serial and Telegram interfaces.
 * 
 * This class uses the StaticSerialCommands library to parse and execute commands.
 * It acts as a central point for command dispatching and interacts with the
 * main ComputerController.
 */
class CommandHandler {
public:
    /**
     * @brief Constructor for CommandHandler.
     * @param controller Pointer to the main ComputerController instance.
     */
    CommandHandler(ComputerController* controller);

    /**
     * @brief Destructor for CommandHandler.
     */
    ~CommandHandler();

    /**
     * @brief Handles incoming commands from the serial interface.
     * Must be called repeatedly in the main loop.
     */
    void handleSerialCommands();

    /**
     * @brief Handles incoming commands from the Telegram bot.
     * Must be called repeatedly in the main loop.
     */
    void handleTelegramCommands();

    /**
     * @brief Performs setup operations for the command handler.
     */
    void setup();

    /**
     * @brief Main loop function for the command handler.
     * Calls internal handlers for serial and Telegram commands.
     */
    void loop();

    // Static command callback functions for StaticSerialCommands
    static void cmdPower(SerialCommands &sender, Args &args);
    static void cmdReset(SerialCommands &sender, Args &args);
    static void cmdStatus(SerialCommands &sender, Args &args);
    static void cmdHelp(SerialCommands &sender, Args &args);
    static void cmdChildLock(SerialCommands &sender, Args &args);

    /**
     * @brief Gets the singleton instance of CommandHandler.
     * Implements lazy initialization.
     * @param controller Pointer to ComputerController, required for first-time initialization.
     * @return Pointer to the CommandHandler instance.
     */
    static CommandHandler* getInstance(ComputerController* controller = nullptr) {
        if (instance == nullptr && controller != nullptr) {
            instance = new CommandHandler(controller);
        }
        return instance;
    }
    /**
     * @brief Gets the array of defined commands and their count.
     * @param count Pointer to a uint16_t to store the number of commands.
     * @return Pointer to the constant array of Command objects.
     */
    static const Command* getCommands(uint16_t* count);

    /**
     * @brief Returns pointer to the associated ComputerController instance.
     */
    ComputerController* getControllerInstance() const { return controller; }

private:
    ComputerController* controller;             ///< Pointer to the main ComputerController.
    PipedStreamPair telegramPipe;               ///< Pipe for channeling Telegram messages to StaticSerialCommands.
    SerialCommands serialCommandsSerial;        ///< StaticSerialCommands instance for the hardware serial port.
    SerialCommands serialCommandsTelegram;      ///< StaticSerialCommands instance for the Telegram input pipe.
    String m_currentTelegramChatId;             ///< Stores the chat ID of the current Telegram message being processed.
    SimpleTimer<> serialCheckTimer{10};         ///< Timer for polling serial input.
    SimpleTimer<> telegramUpdateTimer{MESSAGE_CHECK_INTERVAL}; ///< Timer for polling Telegram updates.

    /**
     * @brief Internal helper to process a single Telegram message.
     * @param chatId The chat ID from which the message originated.
     * @param text The text content of the message.
     * @param fromName The name of the user who sent the message.
     */
    void handleTelegramCommandsInternal(const String &chatId, const String &text, const String &fromName);

    static CommandHandler* instance; ///< Singleton instance of CommandHandler.
}; 