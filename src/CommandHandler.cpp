#include "CommandHandler.h"
#include "Credentials.h"
#include <algorithm> // for std::min
#include "Globals.h"
#include <Arduino.h>
#include <freertos/queue.h>

// Define log tag
static const char* TAG = "CommandHandler";

// Replace static initialization with lazy initialization
#include "ComputerController.h" // Required for inst->controller->activatePowerRelay() etc.
CommandHandler* CommandHandler::instance = nullptr;

// Create larger buffers for command processing
#define SERIAL_BUFFER_SIZE 4096
#define TELEGRAM_BUFFER_SIZE 4096
#define TELEGRAM_PIPE_BUFFER_SIZE 4096

EXT_RAM_ATTR static char serialBuffer[SERIAL_BUFFER_SIZE];
EXT_RAM_ATTR static char telegramBuffer[TELEGRAM_BUFFER_SIZE];

// Forward declaration
static void sendSplitTelegramMessage(UniversalTelegramBot &bot, const String &chatId, const String &message);

// Define command handlers as regular functions
void cmdHelp(SerialCommands& sender, Args& args) {
    ESP_LOGI(TAG, "Help command received");
    
    CommandHandler* inst = CommandHandler::getInstance();
    if (!inst || !inst->getControllerInstance()) {
        ESP_LOGE(TAG, "CommandHandler or Controller not initialized");
        // Use Utilities::printError for initialization errors
        Utilities::printError(sender, F("CommandHandler or Controller not initialized"));
        return;
    }
    
    ESP_LOGI(TAG, "Executing help command");
    // listAllCommands prints directly to the sender's stream.
    sender.listAllCommands();
    // Always print OK for success, per simplification request.
    Utilities::printOK(sender);
    ESP_LOGI(TAG, "Help command completed");
}

void cmdStatus(SerialCommands& sender, Args& args) {
    CommandHandler* inst = CommandHandler::getInstance();
    if (!inst || !inst->getControllerInstance()) {
        // Use Utilities::printError for initialization errors
        Utilities::printError(sender, F("CommandHandler or Controller not initialized"));
        return;
    }

    ESP_LOGI(TAG, "Executing status command");
    String wifiStatus = WiFi.status() == WL_CONNECTED ? "Connected" : "Disconnected";
    String ipAddress = WiFi.status() == WL_CONNECTED ? WiFi.localIP().toString() : "N/A";
    String childLockStatus = PersistentSettings::getInstance().isChildLockEnabled() ? "Enabled" : "Disabled";
    String buzzerStatus = PersistentSettings::getInstance().isBuzzerEnabled() ? "Enabled" : "Disabled";
    String rfStatus = PersistentSettings::getInstance().isRFEnabled() ? "Enabled" : "Disabled";
    String freeHeap = String(ESP.getFreeHeap());
    String gpuFanSpeed = String(inst->getControllerInstance()->getGpuFanSpeed()) + "%";
    String gpuFanRPM = String(inst->getControllerInstance()->getGpuFanRPM()) + " RPM";
    String pcPowerStatus = inst->getControllerInstance()->isPCPoweredOn() ? "On" : "Off";

    // Environmental sensors
    auto appendValue = [](float v, const char* unit) {
        if (isnan(v)) return String("N/A");
        char buf[16];
        snprintf(buf, sizeof(buf), "%.2f %s", v, unit);
        return String(buf);
    };

    String temperatureStr = appendValue(inst->getControllerInstance()->getAmbientTemperature(), "C");
    String humidityStr    = appendValue(inst->getControllerInstance()->getRelativeHumidity(), "%");

    String responseMsg = "Status:\n";
    responseMsg += "- WiFi: " + wifiStatus + "\n";
    responseMsg += "- IP: " + ipAddress + "\n";
    responseMsg += "- PC Power: " + pcPowerStatus + "\n";
    responseMsg += "- Child Lock: " + childLockStatus + "\n";
    responseMsg += "- Buzzer: " + buzzerStatus + "\n";
    responseMsg += "- RF: " + rfStatus + "\n";
    responseMsg += "- GPU Fan: " + gpuFanSpeed + " (" + gpuFanRPM + ")\n";
    responseMsg += "- Temperature: " + temperatureStr + "\n";
    responseMsg += "- Humidity: " + humidityStr + "\n";
    responseMsg += "- Free Heap: " + freeHeap + " bytes";

    sender.getSerial().println(responseMsg);
    Utilities::printOK(sender);
}

void cmdPower(SerialCommands& sender, Args& args) {
    CommandHandler* inst = CommandHandler::getInstance();
    if (!inst || !inst->getControllerInstance()) {
        // Use Utilities::printError
        Utilities::printError(sender, F("CommandHandler or Controller not initialized"));
        return;
    }

    ESP_LOGI(TAG, "Executing power command");
    inst->getControllerInstance()->activatePowerRelay();
    sender.getSerial().println(F("Power button pressed."));
    Utilities::printOK(sender);
}

void cmdReset(SerialCommands& sender, Args& args) {
    CommandHandler* inst = CommandHandler::getInstance();
    if (!inst || !inst->getControllerInstance()) {
        // Use Utilities::printError
        Utilities::printError(sender, F("CommandHandler or Controller not initialized"));
        return;
    }

    ESP_LOGI(TAG, "Executing reset command");
    inst->getControllerInstance()->activateResetRelay();
    sender.getSerial().println(F("Reset button pressed."));
    Utilities::printOK(sender);
}

void cmdChildLock(SerialCommands& sender, Args& args) {
    CommandHandler* inst = CommandHandler::getInstance();
    if (!inst || !inst->getControllerInstance()) {
        // Use Utilities::printError
        Utilities::printError(sender, F("CommandHandler or Controller not initialized"));
        return;
    }

    // For root commands defined with ARG(...) like in cmdArray,
    // the first user argument is in args[0].
    if (args[0].getType() == ArgType::Null) {
        Utilities::printError(sender, F("Usage: childlock <on|off>"));
        return;
    }

    // Assuming StaticSerialCommands ensures arg[0] is a string if not Null,
    // based on the command definition ARG(ArgType::String, "state").
    if (args[0].getType() == ArgType::String) {
        bool enable = strcmp(args[0].getString(), "on") == 0;
        PersistentSettings::getInstance().setChildLockEnabled(enable);
        sender.getSerial().print(F("Child lock "));
        sender.getSerial().println(enable ? F("enabled") : F("disabled"));
        Utilities::printOK(sender);
    } else {
        // Should not happen if command definition and StaticSerialCommands work as expected
        Utilities::printError(sender, F("Invalid argument type for childlock. Expected string."));
    }
}

void cmdBuzzer(SerialCommands& sender, Args& args) {
    CommandHandler* inst = CommandHandler::getInstance();
    if (!inst || !inst->getControllerInstance()) {
        Utilities::printError(sender, F("CommandHandler or Controller not initialized"));
        return;
    }

    if (args[0].getType() == ArgType::Null) {
        Utilities::printError(sender, F("Usage: buzzer <on|off>"));
        return;
    }

    if (args[0].getType() == ArgType::String) {
        bool enable = strcmp(args[0].getString(), "on") == 0;
        PersistentSettings::getInstance().setBuzzerEnabled(enable);
        inst->getControllerInstance()->getBuzzer().setEnabled(enable); // Update SimpleBuzzer state
        sender.getSerial().print(F("Buzzer "));
        sender.getSerial().println(enable ? F("enabled") : F("disabled"));
        Utilities::printOK(sender);
    } else {
        Utilities::printError(sender, F("Invalid argument type for buzzer. Expected string."));
    }
}

void cmdGpuFan(SerialCommands& sender, Args& args) {
    CommandHandler* inst = CommandHandler::getInstance();
    if (!inst || !inst->getControllerInstance()) {
        Utilities::printError(sender, F("CommandHandler or Controller not initialized"));
        return;
    }

    if (args[0].getType() == ArgType::Null) {
        Utilities::printError(sender, F("Usage: gpufan <0-100>"));
        return;
    }

    if (args[0].getType() == ArgType::Int) {
        int speed = args[0].getInt();
        if (speed < 0 || speed > 100) {
            Utilities::printError(sender, F("Speed must be between 0 and 100"));
            return;
        }

        if (inst->getControllerInstance()->setGpuFanSpeed(speed)) {
            sender.getSerial().print(F("GPU fan speed set to "));
            sender.getSerial().print(speed);
            sender.getSerial().println(F("%"));
            Utilities::printOK(sender);
        } else {
            Utilities::printError(sender, F("Failed to set GPU fan speed"));
        }
    } else {
        Utilities::printError(sender, F("Invalid argument type for gpufan. Expected integer."));
    }
}

void cmdRfStudy(SerialCommands& sender, Args& args) {
    CommandHandler* inst = CommandHandler::getInstance();
    if (!inst || !inst->getControllerInstance()) {
        Utilities::printError(sender, F("CommandHandler or Controller not initialized"));
        return;
    }

    RFStudyManager& rfStudyManager = inst->getControllerInstance()->getRFStudyManager();

    // Argument is present
    if (args[0].getType() == ArgType::String) {
        const char* mode = args[0].getString();
        
        if (strcmp(mode, "learn") == 0) {
            sender.getSerial().println(F("Listening for RF button press for 5 seconds..."));
            inst->flush(sender); // Ensure message is sent (also Telegram-aware)

            // Start listening with callback
            bool codeDetectedInCallback = false; // Flag to track if callback was executed

            if (rfStudyManager.startListening(5000, 
                [&sender, &codeDetectedInCallback, inst](uint32_t code) { // Capture flag and inst
                char msgBuf[70]; // "New RF button code detected and saved: 0x" + 8hex + " (" + 10dec + ")" + null
                snprintf(msgBuf, sizeof(msgBuf), "New RF button code detected and saved: 0x%lX (%lu)", code, code);
                sender.getSerial().println(msgBuf);
                Utilities::printOK(sender);
                codeDetectedInCallback = true; // Set flag when code is detected
            })) {
                // Wait for RFStudyManager to stop listening (either code found or timeout)
                while (rfStudyManager.isListening()) {
                    vTaskDelay(pdMS_TO_TICKS(50)); // Yield and wait briefly
                }

                // After listening stops (either by finding a code or by timeout in RFStudyManager),
                // check if the callback was executed.
                if (!codeDetectedInCallback) {
                    // This implies a timeout occurred within RFStudyManager, as the callback wasn't run.
                    ESP_LOGI(TAG, "RF study: No new code detected within the timeout period.");
                    sender.getSerial().println(F("No new RF button code detected (timeout)."));
                    Utilities::printError(sender, F("Timeout"));
                }
                // If codeDetectedInCallback is true, the callback already sent messages and flushed.
            } else { // rfStudyManager.startListening returned false
                ESP_LOGW(TAG, "RF study: Failed to start listening (e.g., already listening or other issue).");
                sender.getSerial().println(F("Failed to start RF code detection."));
                Utilities::printError(sender, F("Busy or internal error"));
            }
        }
        else if (strcmp(mode, "get") == 0) {
            uint32_t currentCode = rfStudyManager.getStoredCode();
            if (currentCode == 0) {
                sender.getSerial().println(F("No RF button code currently stored."));
                Utilities::printOK(sender);
            } else {
                char currentCodeBuf[30]; // Buffer for "0x" + 8 hex + " (" + 10 dec + ")" + null
                snprintf(currentCodeBuf, sizeof(currentCodeBuf), "0x%lX (%lu)", currentCode, currentCode);
                sender.getSerial().print(F("Current RF button code: "));
                sender.getSerial().println(currentCodeBuf);
                Utilities::printOK(sender);
            }
        }
        else if (strcmp(mode, "clear") == 0) {
            rfStudyManager.clearStoredCode();
            ESP_LOGI(TAG, "RF button code cleared via command.");
            sender.getSerial().println(F("RF button code cleared."));
            Utilities::printOK(sender);
        }
        else {
            // Invalid mode
            Utilities::printError(sender, F("Invalid mode. Usage: rfstudy [learn|get|clear]"));
        }
    } else {
        // Should not happen with ARG(ArgType::String, ...)
        Utilities::printError(sender, F("Unexpected argument type. Usage: rfstudy [learn|get|clear]"));
    }
}

void cmdRF(SerialCommands& sender, Args& args) {
    CommandHandler* inst = CommandHandler::getInstance();
    if (!inst || !inst->getControllerInstance()) {
        Utilities::printError(sender, F("CommandHandler or Controller not initialized"));
        return;
    }

    if (args[0].getType() == ArgType::Null) {
        Utilities::printError(sender, F("Usage: rf <on|off>"));
        return;
    }

    if (args[0].getType() == ArgType::String) {
        bool enable = strcmp(args[0].getString(), "on") == 0;
        PersistentSettings::getInstance().setRFEnabled(enable);
        sender.getSerial().print(F("RF functionality "));
        sender.getSerial().println(enable ? F("enabled") : F("disabled"));
        Utilities::printOK(sender);
    } else {
        Utilities::printError(sender, F("Invalid argument type for rf. Expected string."));
    }
}

void cmdResetMCU(SerialCommands& sender, Args& args) {
    CommandHandler* inst = CommandHandler::getInstance();
    if (!inst || !inst->getControllerInstance()) {
        Utilities::printError(sender, F("CommandHandler or Controller not initialized"));
        return;
    }

    ESP_LOGI(TAG, "Executing MCU reset command");
    sender.getSerial().println(F("Resetting MCU..."));
    Utilities::printOK(sender);
    inst->flush(sender);
    
    // Give some time for the message to be sent
    delay(100);
    
    // Reset the ESP32
    ESP.restart();
}

// New command: version
void cmdVersion(SerialCommands& sender, Args& args) {
    ESP_LOGI(TAG, "Executing version command");
    sender.getSerial().print(F("Software version: "));
    sender.getSerial().println(SOFTWARE_VERSION);
    Utilities::printOK(sender);
}

// New command: identity
void cmdIdentity(SerialCommands& sender, Args& args) {
    ESP_LOGI(TAG, "Executing identity command");
    sender.getSerial().print(F("Device: "));
    sender.getSerial().print(DEVICE_NAME);
    sender.getSerial().print(F("  Version: "));
    sender.getSerial().println(SOFTWARE_VERSION);
    Utilities::printOK(sender);
}

// Define the command table *before* any SerialCommands instances are built so
// that we can safely pass a valid pointer/count to their constructors.
static const Command cmdArray[] = {
    COMMAND(cmdHelp,  "help",      NULL, "Shows this help message"),
    COMMAND(cmdStatus,"status",    NULL, "Gets current system status"),
    COMMAND(cmdPower, "power",     NULL, "Simulates power button press"),
    COMMAND(cmdReset, "reset",     NULL, "Simulates reset button press"),
    COMMAND(cmdChildLock, "childlock", ARG(ArgType::String, "state"), NULL, "Enable/disable child lock (on/off)"),
    COMMAND(cmdBuzzer, "buzzer", ARG(ArgType::String, "state"), NULL, "Enable/disable buzzer (on/off)"),
    COMMAND(cmdGpuFan, "gpufan", ARG(ArgType::Int, "speed"), NULL, "Set GPU fan speed (0-100)"),
    COMMAND(cmdRfStudy, "rfstudy", ARG(ArgType::String, "mode"), NULL, "RF button code management (learn: detect new code, get: show current code, clear: remove code)"),
    COMMAND(cmdRF, "rf", ARG(ArgType::String, "state"), NULL, "Enable/disable RF functionality (on/off)"),
    COMMAND(cmdResetMCU, "resetmcu", NULL, "Reset the ESP32 microcontroller"),
    COMMAND(cmdVersion, "version", NULL, "Show software version"),
    COMMAND(cmdIdentity, "identity", NULL, "Show device identity and version")
};

static constexpr uint16_t kCommandCount = sizeof(cmdArray) / sizeof(Command);

const Command* CommandHandler::getCommands(uint16_t* count)
{
    if (count) {
        *count = kCommandCount;
    }
    return cmdArray;
}

CommandHandler::CommandHandler(ComputerController* controller)
    : controller(controller),
      telegramPipe(TELEGRAM_PIPE_BUFFER_SIZE),
      serialCommandsSerial(Serial, cmdArray, kCommandCount, serialBuffer, sizeof(serialBuffer)),
      serialCommandsTelegram(telegramPipe.first, cmdArray, kCommandCount, telegramBuffer, sizeof(telegramBuffer)),
      m_currentTelegramChatId(),
      serialCheckTimer(SERIAL_CHECK_INTERVAL),
      telegramUpdateTimer(MESSAGE_CHECK_INTERVAL),
      telegramTaskHandle(nullptr),
      telegramQueue(nullptr),
      responseQueue(nullptr)
{
    // Set singleton instance pointer
    CommandHandler::instance = this;

    ESP_LOGI(TAG, "Initializing CommandHandler");
    ESP_LOGI(TAG, "Configured chat ID: %s", CHAT_ID);
    
    // Create queue for Telegram messages
    telegramQueue = xQueueCreate(10, sizeof(TelegramMessage));
    if (!telegramQueue) {
        ESP_LOGE(TAG, "Failed to create Telegram message queue!");
        return;
    }
    
    // Create task for processing Telegram messages
    BaseType_t taskCreated = xTaskCreatePinnedToCore(
        telegramTaskFunction,   // Task function
        "TelegramTask",         // Task name
        4096,                   // Stack size
        this,                   // Task parameter
        1,                      // Task priority
        &telegramTaskHandle,    // Task handle
        APP_CPU_NUM             // Run on APP CPU
    );
    
    if (taskCreated != pdPASS) {
        ESP_LOGE(TAG, "Failed to create Telegram processing task!");
        vQueueDelete(telegramQueue);
        telegramQueue = nullptr;
    }

    // Create queue for Telegram responses
    responseQueue = xQueueCreate(10, sizeof(TelegramResponse));
    if (!responseQueue) {
        ESP_LOGE(TAG, "Failed to create Telegram response queue!");
        vQueueDelete(telegramQueue);
        telegramQueue = nullptr;
        return;
    }
}

CommandHandler::~CommandHandler() {
    ESP_LOGI(TAG, "Destroying CommandHandler");
    
    if (telegramTaskHandle) {
        vTaskDelete(telegramTaskHandle);
        telegramTaskHandle = nullptr;
    }
    
    if (telegramQueue) {
        vQueueDelete(telegramQueue);
        telegramQueue = nullptr;
    }
    
    if (responseQueue) {
        vQueueDelete(responseQueue);
        responseQueue = nullptr;
    }
    
    if (CommandHandler::instance == this) {
        CommandHandler::instance = nullptr;
    }
}

void CommandHandler::setup()
{
    ESP_LOGI(TAG, "Setting up CommandHandler");
    
    if (!Serial) {
        ESP_LOGE(TAG, "Serial port not initialized!");
        return;
    }

    while (Serial.available()) {
        Serial.read();
    }
    
    // Print welcome message and test command
    Serial.println(F("\nComputer Controller Ready"));
    Serial.println(F("Type 'help' for available commands"));
    Serial.println();
}

void CommandHandler::loop()
{
    handleSerialCommands();
    handleTelegramCommands();
    handleTelegramResponses();
}

void CommandHandler::handleSerialCommands()
{
    if (!serialCheckTimer.isReady()) {
        return;
    }
    serialCheckTimer.reset();

    // Process serial commands with timeout
    int available = Serial.available();
    if (available > 0) {
        ESP_LOGI(TAG, "Processing %d bytes from serial", available);
        
        // Process up to 256 bytes at a time to improve throughput while still yielding control
        const int MAX_BYTES_PER_LOOP = 256;
        int bytesProcessed = 0;
        
        while (Serial.available() && bytesProcessed < MAX_BYTES_PER_LOOP) {
            char c = Serial.peek();
            ESP_LOGD(TAG, "Processing byte: 0x%02X ('%c')", c, isprint(c) ? c : '.');
            
            // Ensure command is properly terminated
            if (c == '\n' || c == '\r') {
                ESP_LOGD(TAG, "Command terminator found");
            }
            
            serialCommandsSerial.readSerial();
            bytesProcessed++;
        }

        if (bytesProcessed > 0) {
            ESP_LOGI(TAG, "Processed %d bytes", bytesProcessed);
        }
    }
}

void CommandHandler::handleTelegramCommands()
{
    CommandHandler* inst = getInstance();
    if (!inst || !inst->getControllerInstance()) {
        ESP_LOGE(TAG, "CommandHandler::instance or controller is null in handleTelegramCommands!");
        return;
    }

    if (!telegramUpdateTimer.isReady()) {
        return;
    }
    telegramUpdateTimer.reset();
    UniversalTelegramBot &bot = inst->getControllerInstance()->getTelegramBot();

    ESP_LOGI(TAG, "Polling for Telegram updates...");
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    ESP_LOGI(TAG, "Received %d new messages from Telegram API", numNewMessages);
    
    if (numNewMessages > 0)
    {
        int authorizedMessages = 0;
        
        for (int i = 0; i < numNewMessages; i++)
        {
            String chatId = bot.messages[i].chat_id;
            String fromName = bot.messages[i].from_name;
            String messageText = bot.messages[i].text;
            bool isFromBot = fromName.indexOf("bot") != -1 || fromName.indexOf("Bot") != -1; // Check if name contains "bot"
            
            ESP_LOGI(TAG, "Message %d: Chat ID: %s, From: %s, Text: %s, IsBot: %s", 
                     i, chatId.c_str(), fromName.c_str(), messageText.c_str(), isFromBot ? "Yes" : "No");
            
            // Filter: Only process messages from authorized chat IDs
            if (chatId != CHAT_ID) {
                ESP_LOGW(TAG, "Ignoring message from unauthorized chat ID: %s (expected: %s)", 
                         chatId.c_str(), CHAT_ID);
                continue;
            }
            
            // Allow bot-to-bot communication in authorized chats
            if (isFromBot) {
                ESP_LOGI(TAG, "Processing bot-to-bot message from: %s in authorized chat: %s", 
                         fromName.c_str(), chatId.c_str());
            }
            
            authorizedMessages++;
            ESP_LOGI(TAG, "Processing authorized message from chat ID: %s", chatId.c_str());
            
            TelegramMessage msg;
            msg.chatId = new String(chatId);
            msg.text = new String(messageText);
            msg.fromName = new String(fromName);
            
            // Queue the message for processing
            if (xQueueSend(telegramQueue, &msg, 0) != pdPASS) {
                ESP_LOGW(TAG, "Failed to queue Telegram message from %s", msg.fromName->c_str());
                // Free allocated memory to avoid leaks if the message could not be queued
                delete msg.chatId;
                delete msg.text;
                delete msg.fromName;
            } else {
                ESP_LOGI(TAG, "Successfully queued message for processing");
            }
        }
        
        if (authorizedMessages > 0) {
            ESP_LOGI(TAG, "Queued %d authorized messages for processing", authorizedMessages);
        } else {
            ESP_LOGI(TAG, "No authorized messages found");
        }
    }
}

void CommandHandler::handleTelegramResponses()
{
    if (!responseQueue) {
        ESP_LOGE(TAG, "Response queue is null!");
        return;
    }

    // Use static allocation to avoid heap issues
    static TelegramResponse response;
    
    // Check if there are any messages in the queue
    UBaseType_t queueSize = uxQueueMessagesWaiting(responseQueue);
    if (queueSize > 0) {
        ESP_LOGI(TAG, "Found %d messages in response queue", queueSize);
    }
    
    while (xQueueReceive(responseQueue, &response, 0) == pdPASS) {
        ESP_LOGI(TAG, "Processing response for chat ID: %s", response.chatId && !response.chatId->isEmpty() ? response.chatId->c_str() : "<empty>");
        
        if (controller) {
            // Create copies of the strings (not cleared until after send)
            String *chatIdCopy = response.chatId;
            String *messageCopy = response.message;

            // Send the message first (while original still intact)
            if (chatIdCopy && messageCopy && !chatIdCopy->isEmpty() && !messageCopy->isEmpty()) {
                ESP_LOGI(TAG, "Sending Telegram message to %s: %s", chatIdCopy->c_str(), messageCopy->c_str());
                sendSplitTelegramMessage(controller->getTelegramBot(), *chatIdCopy, *messageCopy);
            } else {
                ESP_LOGW(TAG, "Skipping empty message or chat ID");
            }

            // Now that sending is done, clear the response struct to free memory
            delete response.chatId;
            delete response.message;
            response.chatId = nullptr;
            response.message = nullptr;
            ESP_LOGI(TAG, "Response processed and cleared");
        } else {
            ESP_LOGE(TAG, "Controller is null, cannot send response");
        }
    }
}

void CommandHandler::telegramTaskFunction(void* parameter)
{
    CommandHandler* inst = static_cast<CommandHandler*>(parameter);
    TelegramMessage msg;
    
    while (true) {
        // Wait for a message to arrive in the queue
        if (xQueueReceive(inst->telegramQueue, &msg, portMAX_DELAY) == pdPASS) {
            inst->processTelegramMessage(msg);
            // Free dynamically allocated strings inside the message after processing
            delete msg.chatId;
            delete msg.text;
            delete msg.fromName;
        }
    }
}

void CommandHandler::processTelegramMessage(const TelegramMessage& msg)
{
    // Dereference pointers inside the struct
    String commandText = *msg.text;
    if (commandText.startsWith("/")) {
        commandText.remove(0, 1);
    }
    int atIndex = commandText.indexOf('@');
    if (atIndex != -1) {
        commandText.remove(atIndex, commandText.length() - atIndex);
    }
    commandText.toLowerCase();

    ESP_LOGI(TAG, "Processing Telegram msg from %s (%s): %s (processed: %s)", 
             msg.fromName->c_str(), msg.chatId->c_str(), msg.text->c_str(), commandText.c_str());

    m_currentTelegramChatId = *msg.chatId;
    if (!telegramPipe.second.print(commandText)) {
        ESP_LOGE(TAG, "Failed to write command text to telegram pipe!");
        m_currentTelegramChatId = "";
        
        // Create response with new strings
        TelegramResponse response;
        response.chatId = new String(*msg.chatId);
        response.message = new String("Error: Could not process your command (internal pipe error).");
        
        // Send to queue and immediately clear local copy
        if (xQueueSend(responseQueue, &response, 0) == pdPASS) {
            ESP_LOGI(TAG, "Queued error response for chat ID: %s", msg.chatId->c_str());
            response.chatId = nullptr;
            response.message = nullptr;
        } else {
            ESP_LOGE(TAG, "Failed to queue error response!");
            delete response.chatId;
            delete response.message;
        }
        return;
    }
    
    if (!telegramPipe.second.print('\n')) {
        ESP_LOGE(TAG, "Failed to write newline to telegram pipe!");
        m_currentTelegramChatId = "";
        
        // Create response with new strings
        TelegramResponse response;
        response.chatId = new String(*msg.chatId);
        response.message = new String("Error: Could not process your command (internal pipe error).");
        
        // Send to queue and immediately clear local copy
        if (xQueueSend(responseQueue, &response, 0) == pdPASS) {
            ESP_LOGI(TAG, "Queued error response for chat ID: %s", msg.chatId->c_str());
            response.chatId = nullptr;
            response.message = nullptr;
        } else {
            ESP_LOGE(TAG, "Failed to queue error response!");
            delete response.chatId;
            delete response.message;
        }
        return;
    }
    
    serialCommandsTelegram.readSerial();
    m_currentTelegramChatId = "";

    String responseMessage = "";
    String currentLineBuffer = "";
    bool finishedReading = false;

    ESP_LOGD(TAG, "Telegram response: Starting to read from pipe...");

    SimpleTimer<> readTimer(READ_TIMEOUT_MS);

    while (!finishedReading && !readTimer.isReady()) {
        if (telegramPipe.second.available() > 0) {
            char c = (char)telegramPipe.second.read();
            responseMessage += c;
            currentLineBuffer += c;

            // Extend timeout as long as data keeps coming
            readTimer.reset();

            if (c == '\n') {
                ESP_LOGD(TAG, "Telegram response: Read line: [%s]", currentLineBuffer.c_str());
                String trimmedLine = currentLineBuffer;
                trimmedLine.trim();
                if (trimmedLine.startsWith("ERROR:")) {
                    ESP_LOGI(TAG, "Telegram response: Found ERROR marker.");
                    finishedReading = true;
                }
                else if (trimmedLine.equals("OK")) {
                     ESP_LOGI(TAG, "Telegram response: Found OK marker.");
                     finishedReading = true;
                }
                currentLineBuffer = "";
            }
        } else {
            vTaskDelay(pdMS_TO_TICKS(1));
        }
    }
    
    if (!finishedReading) {
        ESP_LOGW(TAG, "Telegram response: Timed out waiting for OK/ERROR marker (>%lu ms)", READ_TIMEOUT_MS);
    }
    
    responseMessage.trim();

    // Create response with new strings
    TelegramResponse response;
    response.chatId = new String(*msg.chatId);
    
    if (responseMessage.length() > 0) {
        ESP_LOGI(TAG, "Queueing Telegram reply to %s: %s", msg.chatId->c_str(), responseMessage.c_str());
        response.message = new String(responseMessage);
    } else {
        ESP_LOGI(TAG, "No explicit response (or only whitespace) for Telegram command: %s", commandText.c_str());
        response.message = new String("Command processed, no specific output or only markers received.");
    }

    // Send to queue and immediately clear local copy
    if (xQueueSend(responseQueue, &response, 0) == pdPASS) {
        ESP_LOGI(TAG, "Successfully queued response for chat ID: %s", msg.chatId->c_str());
        response.chatId = nullptr;
        response.message = nullptr;
    } else {
        ESP_LOGE(TAG, "Failed to queue response for chat ID: %s", msg.chatId->c_str());
        delete response.chatId;
        delete response.message;
    }
}

// Static flush helper
void CommandHandler::flush(SerialCommands &sender) {
    // Always flush the underlying stream first
    sender.getSerial().flush();

    if (!controller) {
        return;
    }

    // Check if this is the Telegram sender
    if (&sender != &serialCommandsTelegram) {
        return;
    }

    // Get current chat ID
    const String& chatId = m_currentTelegramChatId;
    if (chatId.isEmpty()) {
        return;
    }

    // Read accumulated data from pipe
    String message;
    while (telegramPipe.second.available() > 0) {
        char c = (char)telegramPipe.second.read();
        message += c;
    }

    message.trim();
    if (message.isEmpty()) {
        return;
    }

    // Send via Telegram bot, split into chunks if necessary
    sendSplitTelegramMessage(controller->getTelegramBot(), chatId, message);
}

// Helper that sends a long message to Telegram by splitting it into chunks that
// respect Telegram's maximum message length (defined in Globals.h).
static void sendSplitTelegramMessage(UniversalTelegramBot &bot, const String &chatId, const String &message)
{
    const size_t maxLen = TELEGRAM_MAX_MESSAGE; // 4096
    size_t total = message.length();
    ESP_LOGD(TAG, "Sending message of %u characters to Telegram chat %s", (unsigned)total, chatId.c_str());
    for (size_t offset = 0; offset < total; offset += maxLen)
    {
        String chunk = message.substring(offset, std::min(offset + maxLen, total));
        ESP_LOGD(TAG, "Chunk length after trim: %u", chunk.length());

        bool ok = bot.sendMessage(chatId, chunk, "");
        if (!ok) {
            ESP_LOGE(TAG, "bot.sendMessage failed; attempting retry. (No last_error available)");
            // Retry once after encoding newlines, as Telegram API may reject raw newlines in urlencoded body
            String encodedChunk = chunk;
            encodedChunk.replace("%", "%25"); // pre-escape % first
            encodedChunk.replace("+", "%2B"); // escape plus sign to avoid space conversion
            encodedChunk.replace("\n", "%0A");
            encodedChunk.replace("\r", "");
            ESP_LOGW(TAG, "Retrying Telegram send after encoding newlines...");
            ok = bot.sendMessage(chatId, encodedChunk, "");
        }

        if (ok)
        {
            ESP_LOGI(TAG, "Telegram chunk sent successfully (%u / %u)", (unsigned)std::min(offset + maxLen, total), (unsigned)total);
        }
        else
        {
            ESP_LOGE(TAG, "Failed to send Telegram chunk starting at character %u", (unsigned)offset);
        }
        // Small delay to avoid hitting Telegram flood limits
        delay(20);
    }
}