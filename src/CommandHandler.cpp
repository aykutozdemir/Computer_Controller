#include "CommandHandler.h"

// Define log tag
static const char* TAG = "CommandHandler";

// Replace static initialization with lazy initialization
#include "ComputerController.h" // Required for inst->controller->activatePowerRelay() etc.
CommandHandler* CommandHandler::instance = nullptr;

// Create larger buffers for command processing
EXT_RAM_ATTR static char serialBuffer[4096];
EXT_RAM_ATTR static char telegramBuffer[4096];

// Define command handlers as regular functions
void cmdHelp(SerialCommands& sender, Args& args) {
    CommandHandler* inst = CommandHandler::getInstance();
    if (!inst || !inst->getControllerInstance()) {
        // Use Utilities::printError for initialization errors
        Utilities::printError(sender, F("CommandHandler or Controller not initialized"));
        return;
    }
    
    ESP_LOGI(TAG, "Executing help command");
    // listAllCommands prints directly to the sender's stream.
    sender.listAllCommands();
    // Always print OK for success, per simplification request.
    Utilities::printOK(sender);
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
    String freeHeap = String(ESP.getFreeHeap());

    String responseMsg = "Status:\n";
    responseMsg += "- WiFi: " + wifiStatus + "\n";
    responseMsg += "- IP: " + ipAddress + "\n";
    responseMsg += "- Child Lock: " + childLockStatus + "\n";
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

    if (PersistentSettings::getInstance().isChildLockEnabled()) {
        Utilities::printError(sender, F("Child lock is enabled"));
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

    if (PersistentSettings::getInstance().isChildLockEnabled()) {
        Utilities::printError(sender, F("Child lock is enabled"));
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

// Define the command table *before* any SerialCommands instances are built so
// that we can safely pass a valid pointer/count to their constructors.
static const Command cmdArray[] = {
    COMMAND(cmdHelp,  "help",      NULL, "Shows this help message"),
    COMMAND(cmdStatus,"status",    NULL, "Gets current system status"),
    COMMAND(cmdPower, "power",     NULL, "Simulates power button press"),
    COMMAND(cmdReset, "reset",     NULL, "Simulates reset button press"),
    COMMAND(cmdChildLock, "childlock", ARG(ArgType::String, "state"), NULL, "Enable/disable child lock (on/off)")
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
      telegramPipe(4096),
      serialCommandsSerial(Serial, cmdArray, kCommandCount, serialBuffer, sizeof(serialBuffer)),
      serialCommandsTelegram(telegramPipe.first, cmdArray, kCommandCount, telegramBuffer, sizeof(telegramBuffer))
{
    // Set singleton instance pointer
    CommandHandler::instance = this;

    ESP_LOGI(TAG, "Initializing CommandHandler");
    m_currentTelegramChatId = "";
}

CommandHandler::~CommandHandler() {
    ESP_LOGI(TAG, "Destroying CommandHandler");
    if (CommandHandler::instance == this) {
        CommandHandler::instance = nullptr;
    }
}

void CommandHandler::setup()
{
    ESP_LOGI(TAG, "Setting up CommandHandler");
}

void CommandHandler::loop()
{
    handleSerialCommands();
    handleTelegramCommands();
}

void CommandHandler::handleSerialCommands()
{
    if (!serialCheckTimer.isReady()) {
        return;
    }
    serialCheckTimer.reset();

    // Set a timeout for serial command processing
    SimpleTimer<> processTimer(20);  // 20ms timeout for processing

    // Process serial commands with timeout
    int available = Serial.available();
    if (available > 0) {
        ESP_LOGD(TAG, "Processing %d bytes from serial", available);
        
        // Process up to 64 bytes at a time to prevent blocking
        const int MAX_BYTES_PER_LOOP = 64;
        int bytesProcessed = 0;
        
        while (Serial.available() && bytesProcessed < MAX_BYTES_PER_LOOP && !processTimer.isReady()) {
            serialCommandsSerial.readSerial();
            bytesProcessed++;
            delay(1);  // Small delay between bytes
        }
        
        if (processTimer.isReady()) {
            ESP_LOGW(TAG, "Serial command processing took too long");
        }
        
        if (bytesProcessed > 0) {
            ESP_LOGD(TAG, "Processed %d bytes", bytesProcessed);
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

    SimpleTimer<> updateTimer(2000);          // 2-second software timeout
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

    if (updateTimer.isReady())
    {
        ESP_LOGW(TAG, "Telegram getUpdates took too long");
        return;
    }

    if (numNewMessages > 0)
    {
        ESP_LOGI(TAG, "Processing %d new messages", numNewMessages);
        for (int i = 0; i < numNewMessages; i++)
        {
            SimpleTimer<> messageTimer(1000);
            handleTelegramCommandsInternal(
                String(bot.messages[i].chat_id),
                bot.messages[i].text,
                bot.messages[i].from_name);
                
            if (messageTimer.isReady()) {
                ESP_LOGW(TAG, "Message processing took too long");
            }
        }
    }
}

void CommandHandler::handleTelegramCommandsInternal(const String &chatId, const String &text, const String &fromName)
{
    CommandHandler* inst = getInstance();
    // Enhanced null check for inst and controller instance
    if (!inst || !inst->getControllerInstance()) {
        ESP_LOGE(TAG, "CommandHandler::instance or controller is null in handleTelegramCommandsInternal!");
        // Attempt to send an error message if possible
        if (inst && inst->getControllerInstance()) { // Check again, just in case for sending error message
             inst->getControllerInstance()->getTelegramBot().sendMessage(chatId, "Internal error: Command handler not ready.", "");
        }
        return;
    }

    String commandText = text; // Make a mutable copy to potentially modify
    if (commandText.startsWith("/")) {
        commandText.remove(0, 1); // Remove the leading '/'
    }

    ESP_LOGI(TAG, "Telegram msg from %s (%s): %s (processed: %s)", fromName.c_str(), chatId.c_str(), text.c_str(), commandText.c_str());

    // Store chat_id temporarily for the callback to access
    inst->m_currentTelegramChatId = chatId;
    // Write command to pipe for StaticSerialCommands with timeout
    SimpleTimer<> writeTimer(500);  // 500ms timeout for writing
    if (!inst->telegramPipe.second.print(commandText)) { // Use the potentially modified commandText
        ESP_LOGE(TAG, "Failed to write command text to telegram pipe!");
        inst->m_currentTelegramChatId = "";
        if (inst->getControllerInstance()) {
            inst->getControllerInstance()->getTelegramBot().sendMessage(chatId, "Error: Could not process your command (internal pipe error).", "");
        }
        return;
    }
    if (!inst->telegramPipe.second.print('\n')) { // Ensure command is terminated by newline
        ESP_LOGE(TAG, "Failed to write newline to telegram pipe!");
        inst->m_currentTelegramChatId = "";
        if (inst->getControllerInstance()) {
            inst->getControllerInstance()->getTelegramBot().sendMessage(chatId, "Error: Could not process your command (internal pipe error).", "");
        }
        return;
    }
    // Process command from pipe with timeout
    SimpleTimer<> processTimer(1000);  // Timeout for command processing itself
    inst->serialCommandsTelegram.readSerial();
    
    // Clear the temporary chat_id
    inst->m_currentTelegramChatId = "";

    // Read the response from the pipe with marker detection
    String responseMessage = "";
    String currentLineBuffer = "";
    bool finishedReading = false;
    SimpleTimer<> responseReadTimer(1500); // Timeout for reading the full response from the pipe

    ESP_LOGD(TAG, "Telegram response: Starting to read from pipe...");

    while (!responseReadTimer.isReady() && !finishedReading) {
        if (inst->telegramPipe.second.available() > 0) {
            char c = (char)inst->telegramPipe.second.read();
            responseMessage += c;
            currentLineBuffer += c;

            if (c == '\n') {
                ESP_LOGD(TAG, "Telegram response: Read line: [%s]", currentLineBuffer.c_str());
                // Check for "ERROR: ...\r\n" (Utilities::printError sends "ERROR: <msg>\r\n")
                if (currentLineBuffer.startsWith("ERROR: ")) { 
                    ESP_LOGI(TAG, "Telegram response: Found ERROR marker.");
                    finishedReading = true;
                }
                // Check for "\r\nOK\r\n" (Utilities::printOK sends "\r\nOK\r\n")
                else if (currentLineBuffer.equals("\r\nOK\r\n")) {
                     ESP_LOGI(TAG, "Telegram response: Found OK marker.");
                     finishedReading = true;
                }
                currentLineBuffer = ""; // Reset for the next line
            }
        } else {
            if (finishedReading) { // If markers found and pipe is now empty, exit
                break;
            }
            delay(5); // Small delay if pipe is temporarily empty but not finished reading
        }
    }

    if (responseReadTimer.isReady() && !finishedReading) {
        ESP_LOGW(TAG, "Telegram response: Timed out waiting for OK/ERROR markers. Sending what was received.");
    } else if (finishedReading) {
        ESP_LOGI(TAG, "Telegram response: Reading finished, markers detected.");
    } else { 
        ESP_LOGI(TAG, "Telegram response: Reading finished (pipe empty or other condition), no specific markers found before loop exit.");
    }
    
    responseMessage.trim(); // Remove leading/trailing whitespace

    if (responseMessage.length() > 0) {
        ESP_LOGI(TAG, "Sending Telegram reply to %s: %s", chatId.c_str(), responseMessage.c_str());
        // Note: Telegram messages have a max length of 4096 chars. Truncate if necessary.
        inst->getControllerInstance()->getTelegramBot().sendMessage(chatId, responseMessage, "");
    } else {
        ESP_LOGI(TAG, "No explicit response (or only whitespace) for Telegram command: %s", commandText.c_str());
        inst->getControllerInstance()->getTelegramBot().sendMessage(chatId, "Command processed, no specific output or only markers received.", "");
    }

    if (writeTimer.isReady() || processTimer.isReady()) { // Check original timers for writing/processing
        ESP_LOGW(TAG, "Telegram command writing to pipe or initial processing took too long.");
    }
}