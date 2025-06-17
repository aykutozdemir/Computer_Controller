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
     * Cleans up any resources used by the command handler.
     */
    ~CommandHandler();

    /**
     * @brief Performs setup operations for the command handler.
     * Initializes serial communication, command processing, and Telegram bot.
     * Must be called before using any other methods.
     */
    void setup();

    /**
     * @brief Main loop function for the command handler.
     * Processes incoming commands from both serial and Telegram interfaces.
     * Should be called regularly in the main program loop.
     */
    void loop();

    /**
     * @brief Static callback for power command.
     * Simulates a power button press on the computer.
     * @param sender The SerialCommands instance that received the command
     * @param args Command arguments (if any)
     */
    static void cmdPower(SerialCommands &sender, Args &args);

    /**
     * @brief Static callback for reset command.
     * Simulates a reset button press on the computer.
     * @param sender The SerialCommands instance that received the command
     * @param args Command arguments (if any)
     */
    static void cmdReset(SerialCommands &sender, Args &args);

    /**
     * @brief Static callback for status command.
     * Returns the current system status including power state and sensor readings.
     * @param sender The SerialCommands instance that received the command
     * @param args Command arguments (if any)
     */
    static void cmdStatus(SerialCommands &sender, Args &args);

    /**
     * @brief Static callback for help command.
     * Displays available commands and their usage.
     * @param sender The SerialCommands instance that received the command
     * @param args Command arguments (if any)
     */
    static void cmdHelp(SerialCommands &sender, Args &args);

    /**
     * @brief Static callback for child lock command.
     * Enables or disables the child lock feature.
     * @param sender The SerialCommands instance that received the command
     * @param args Command arguments (if any)
     */
    static void cmdChildLock(SerialCommands &sender, Args &args);

    /**
     * @brief Static callback for buzzer command.
     * Controls the buzzer functionality.
     * @param sender The SerialCommands instance that received the command
     * @param args Command arguments (if any)
     */
    static void cmdBuzzer(SerialCommands &sender, Args &args);

    /**
     * @brief Static callback for RF study command.
     * Initiates RF code learning mode for remote control.
     * @param sender The SerialCommands instance that received the command
     * @param args Command arguments (if any)
     */
    static void cmdRfStudy(SerialCommands& sender, Args& args);

    /**
     * @brief Static callback for RF command.
     * Sends RF signals for remote control.
     * @param sender The SerialCommands instance that received the command
     * @param args Command arguments (if any)
     */
    static void cmdRF(SerialCommands& sender, Args& args);

    /**
     * @brief Static callback for MCU reset command.
     * Resets the ESP32 microcontroller.
     * @param sender The SerialCommands instance that received the command
     * @param args Command arguments (if any)
     */
    static void cmdResetMCU(SerialCommands& sender, Args& args);

    /**
     * @brief Gets the singleton instance of CommandHandler.
     * Implements lazy initialization pattern.
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
     * @return Pointer to the ComputerController instance.
     */
    ComputerController* getControllerInstance() const { return controller; }

    /**
     * @brief Gets the SerialCommands instance for serial interface.
     * @return Reference to the SerialCommands instance for serial communication.
     */
    SerialCommands& getSerialCommandsSerial() { return serialCommandsSerial; }

    /**
     * @brief Gets the SerialCommands instance for Telegram interface.
     * @return Reference to the SerialCommands instance for Telegram communication.
     */
    SerialCommands& getSerialCommandsTelegram() { return serialCommandsTelegram; }

    /**
     * @brief Gets the Telegram pipe pair for message handling.
     * @return Reference to the PipedStreamPair used for Telegram communication.
     */
    PipedStreamPair& getTelegramPipe() { return telegramPipe; }

    /**
     * @brief Gets the current Telegram chat ID.
     * @return Constant reference to the current Telegram chat ID string.
     */
    const String& getCurrentTelegramChatId() const { return m_currentTelegramChatId; }

    /**
     * @brief Flushes output for the given SerialCommands sender.
     * If the sender corresponds to the Serial (USB) interface, this simply calls flush() on the stream.
     * If it corresponds to the Telegram interface, it sends the accumulated data immediately via the bot.
     * @param sender The SerialCommands instance whose output should be flushed.
     */
    void flush(SerialCommands &sender);

private:
    // Private members
    ComputerController* controller;             ///< Pointer to the main ComputerController.
    PipedStreamPair telegramPipe;               ///< Pipe for channeling Telegram messages to StaticSerialCommands.
    SerialCommands serialCommandsSerial;        ///< StaticSerialCommands instance for the hardware serial port.
    SerialCommands serialCommandsTelegram;      ///< StaticSerialCommands instance for the Telegram input pipe.
    String m_currentTelegramChatId;             ///< Stores the chat ID of the current Telegram message being processed.
    SimpleTimer<unsigned long> serialCheckTimer;         ///< Timer for polling serial input.
    SimpleTimer<unsigned long> telegramUpdateTimer; ///< Timer for polling Telegram updates.

    /**
     * @brief Handles incoming serial commands.
     * Processes commands received through the serial interface.
     */
    void handleSerialCommands();

    /**
     * @brief Handles incoming Telegram commands.
     * Processes commands received through the Telegram interface.
     */
    void handleTelegramCommands();

    /**
     * @brief Internal method to handle Telegram commands.
     * Processes a specific Telegram message.
     * @param chatId The Telegram chat ID
     * @param text The message text
     * @param fromName The name of the sender
     */
    void handleTelegramCommandsInternal(const String &chatId, const String &text, const String &fromName);

    // Singleton instance
    static CommandHandler* instance;
}; 