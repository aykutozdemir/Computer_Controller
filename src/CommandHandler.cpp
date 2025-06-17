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
    String buzzerStatus = PersistentSettings::getInstance().isBuzzerEnabled() ? "Enabled" : "Disabled";
    String rfStatus = PersistentSettings::getInstance().isRFEnabled() ? "Enabled" : "Disabled";
    String freeHeap = String(ESP.getFreeHeap());
    String gpuFanSpeed = String(inst->getControllerInstance()->getGpuFanSpeed()) + "%";
    String gpuFanRPM = String(inst->getControllerInstance()->getGpuFanRPM()) + " RPM";

    // Environmental sensors
    auto appendValue = [](float v, const char* unit) {
        if (isnan(v)) return String("N/A");
        char buf[16];
        snprintf(buf, sizeof(buf), "%.2f %s", v, unit);
        return String(buf);
    };

    String temperatureStr = appendValue(inst->getControllerInstance()->getAmbientTemperature(), "C");
    String humidityStr    = appendValue(inst->getControllerInstance()->getRelativeHumidity(), "%");
    String pressureStr    = appendValue(inst->getControllerInstance()->getBarometricPressure() / 100.0f, "hPa"); // convert Pa→hPa
    String altitudeStr    = appendValue(inst->getControllerInstance()->getAltitude(), "m");

    String responseMsg = "Status:\n";
    responseMsg += "- WiFi: " + wifiStatus + "\n";
    responseMsg += "- IP: " + ipAddress + "\n";
    responseMsg += "- Child Lock: " + childLockStatus + "\n";
    responseMsg += "- Buzzer: " + buzzerStatus + "\n";
    responseMsg += "- RF: " + rfStatus + "\n";
    responseMsg += "- GPU Fan: " + gpuFanSpeed + " (" + gpuFanRPM + ")\n";
    responseMsg += "- Temperature: " + temperatureStr + "\n";
    responseMsg += "- Humidity: " + humidityStr + "\n";
    responseMsg += "- Pressure: " + pressureStr + "\n";
    responseMsg += "- Altitude: " + altitudeStr + "\n";
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
      telegramPipe(),
      serialCommandsSerial(Serial, cmdArray, kCommandCount, serialBuffer, sizeof(serialBuffer)),
      serialCommandsTelegram(telegramPipe.first, cmdArray, kCommandCount, telegramBuffer, sizeof(telegramBuffer)),
      m_currentTelegramChatId(),
      serialCheckTimer(SERIAL_CHECK_INTERVAL),
      telegramUpdateTimer(MESSAGE_CHECK_INTERVAL)
{
    // Set singleton instance pointer
    CommandHandler::instance = this;

    ESP_LOGI(TAG, "Initializing CommandHandler");
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
        
        // Process up to 256 bytes at a time to improve throughput while still yielding control
        const int MAX_BYTES_PER_LOOP = 256;
        int bytesProcessed = 0;
        
        while (Serial.available() && bytesProcessed < MAX_BYTES_PER_LOOP && !processTimer.isReady()) {
            serialCommandsSerial.readSerial();
            bytesProcessed++;
            // No explicit delay; allow FreeRTOS scheduler to switch tasks naturally
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
    if (!inst || !inst->controller) {
        ESP_LOGE(TAG, "CommandHandler::instance or controller is null in handleTelegramCommandsInternal!");
        // Attempt to send an error message if possible
        if (inst && inst->controller) { // Check again, just in case for sending error message
             inst->controller->getTelegramBot().sendMessage(chatId, "Internal error: Command handler not ready.", "");
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
        if (inst->controller) {
            inst->controller->getTelegramBot().sendMessage(chatId, "Error: Could not process your command (internal pipe error).", "");
        }
        return;
    }
    if (!inst->telegramPipe.second.print('\n')) { // Ensure command is terminated by newline
        ESP_LOGE(TAG, "Failed to write newline to telegram pipe!");
        inst->m_currentTelegramChatId = "";
        if (inst->controller) {
            inst->controller->getTelegramBot().sendMessage(chatId, "Error: Could not process your command (internal pipe error).", "");
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
        inst->controller->getTelegramBot().sendMessage(chatId, responseMessage, "");
    } else {
        ESP_LOGI(TAG, "No explicit response (or only whitespace) for Telegram command: %s", commandText.c_str());
        inst->controller->getTelegramBot().sendMessage(chatId, "Command processed, no specific output or only markers received.", "");
    }

    if (writeTimer.isReady() || processTimer.isReady()) { // Check original timers for writing/processing
        ESP_LOGW(TAG, "Telegram command writing to pipe or initial processing took too long.");
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

    // Truncate if needed
    if (message.length() > TELEGRAM_MAX_MESSAGE) {
        message = message.substring(0, TELEGRAM_MAX_MESSAGE);
    }

    // Send via Telegram bot
    controller->getTelegramBot().sendMessage(chatId, message, "");
}