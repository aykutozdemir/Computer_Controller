#pragma once

#include <Arduino.h>
#include <WString.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>
#include "StaticSerialCommands.h"
#include "Command.h"
#include "Arg.h"
#include "PipedStream.h"
#include "SimpleTimer.h"

class ComputerController;

// Forward declarations of task functions
void commandHandlerTask(void *pvParameters);
void commandExecutionTask(void *pvParameters);

// Define enums for command identification and source
typedef enum
{
    CMD_TYPE_POWER,
    CMD_TYPE_RESET,
    CMD_TYPE_STATUS,
    CMD_TYPE_HELP
} CommandType_t;

typedef struct
{
    SerialCommands* sender;
    CommandType_t type;
} SafeCommandTask;

class CommandHandler final {
public:
    CommandHandler(ComputerController* const controller);
    ~CommandHandler();
    void setup();
    void handleSerialCommands();
    void handleTelegramCommands();

    static void executePower(const SafeCommandTask& task);
    static void executeReset(const SafeCommandTask& task);
    static void executeStatus(const SafeCommandTask& task);
    static void executeHelp(const SafeCommandTask& task);

private:
    void handleTelegramCommandsInternal(const String& chatId, const String& text, const String& fromName);
    static void queueSafeCommand(CommandType_t type, SerialCommands* sender);
    void pressRelayButton(int pin);

    ComputerController* controller;
    SerialCommands serialCommandsSerial;
    SerialCommands serialCommandsTelegram;
    PipedStreamPair telegramPipe;
    String m_currentTelegramChatId;

    // Timing management
    SimpleTimer<> serialCheckTimer{100};     // Check serial every 100ms
    SimpleTimer<> telegramUpdateTimer{5000}; // Check Telegram every 5 seconds
    SimpleTimer<> watchdogTimer{5000};       // Feed watchdog every 5 seconds

    static void cmdPower(SerialCommands &sender, Args &args);
    static void cmdReset(SerialCommands &sender, Args &args);
    static void cmdStatus(SerialCommands &sender, Args &args);
    static void cmdHelp(SerialCommands &sender, Args &args);

    static CommandHandler* instance;
    static const Command commands[];
    static QueueHandle_t commandQueue;

    // Make task functions friends
    friend void commandHandlerTask(void *pvParameters);
    friend void commandExecutionTask(void *pvParameters);
}; 