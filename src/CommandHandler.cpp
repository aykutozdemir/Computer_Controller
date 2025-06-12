#include "../include/CommandHandler.h"
#include "../include/ComputerController.h"
#include "../include/Globals.h"
#include <Utilities.h>
#include "esp_task_wdt.h"  // Add watchdog timer header
#include <SimpleTimer.h>  // Add SimpleTimer header
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "esp_system.h"   // For ESP error handling
#include "esp_attr.h"  // For IRAM_ATTR and other attributes

// Define log tag
static const char* TAG = "CommandHandler";

// Message structure for Serial operations with proper alignment
struct __attribute__((aligned(4))) SerialMessage {
    enum Type { WRITE, READ, AVAILABLE, PEEK, FLUSH } type;
    union {
        char writeBuffer[256];
        int readResult;
    } data;
    size_t size;
    QueueHandle_t responseQueue;
};

// Global queues for Serial operations
static QueueHandle_t serialWriteQueue = NULL;
static QueueHandle_t serialReadQueue = NULL;
static TaskHandle_t serialTaskHandle = NULL;
static portMUX_TYPE serialMux = portMUX_INITIALIZER_UNLOCKED;

// Serial task that runs on Core 0
static void IRAM_ATTR serialTask(void* arg) {
    SerialMessage msg;
    
    while (1) {
        if (xQueueReceive(serialWriteQueue, &msg, portMAX_DELAY) == pdTRUE) {
            portENTER_CRITICAL(&serialMux);
            switch (msg.type) {
                case SerialMessage::WRITE:
                    if (Serial) {
                        Serial.write(msg.data.writeBuffer, msg.size);
                    }
                    break;
                    
                case SerialMessage::READ:
                    msg.data.readResult = Serial ? Serial.read() : -1;
                    if (msg.responseQueue) {
                        xQueueSend(msg.responseQueue, &msg, portMAX_DELAY);
                    }
                    break;
                    
                case SerialMessage::AVAILABLE:
                    msg.data.readResult = Serial ? Serial.available() : 0;
                    if (msg.responseQueue) {
                        xQueueSend(msg.responseQueue, &msg, portMAX_DELAY);
                    }
                    break;
                    
                case SerialMessage::PEEK:
                    msg.data.readResult = Serial ? Serial.peek() : -1;
                    if (msg.responseQueue) {
                        xQueueSend(msg.responseQueue, &msg, portMAX_DELAY);
                    }
                    break;
                    
                case SerialMessage::FLUSH:
                    if (Serial) {
                        Serial.flush();
                    }
                    break;
            }
            portEXIT_CRITICAL(&serialMux);
        }
        esp_task_wdt_reset();
    }
}

// Initialize Serial communication system
static bool initSerialSystem() {
    if (serialWriteQueue != NULL) return true;  // Already initialized
    
    serialWriteQueue = xQueueCreate(20, sizeof(SerialMessage));
    if (!serialWriteQueue) {
        ESP_LOGE(TAG, "Failed to create serial write queue");
        return false;
    }
    
    BaseType_t result = xTaskCreatePinnedToCore(
        serialTask,
        "SerialTask",
        4096,
        NULL,
        1,
        &serialTaskHandle,
        0  // Run on Core 0
    );
    
    if (result != pdPASS) {
        ESP_LOGE(TAG, "Failed to create serial task");
        vQueueDelete(serialWriteQueue);
        serialWriteQueue = NULL;
        return false;
    }
    
    esp_task_wdt_add(serialTaskHandle);
    ESP_LOGI(TAG, "Serial system initialized successfully");
    return true;
}

// Cleanup Serial communication system
static void cleanupSerialSystem() {
    if (serialTaskHandle) {
        esp_task_wdt_delete(serialTaskHandle);
        vTaskDelete(serialTaskHandle);
        serialTaskHandle = NULL;
        ESP_LOGI(TAG, "Serial task deleted");
    }
    if (serialWriteQueue) {
        vQueueDelete(serialWriteQueue);
        serialWriteQueue = NULL;
        ESP_LOGI(TAG, "Serial queue deleted");
    }
}

// Thread-safe Serial wrapper
class ThreadSafeSerial : public Stream {
private:
    QueueHandle_t responseQueue;
    bool initialized;
    portMUX_TYPE instanceMux;

public:
    ThreadSafeSerial() : initialized(false) {
        instanceMux = portMUX_INITIALIZER_UNLOCKED;
        responseQueue = xQueueCreate(1, sizeof(SerialMessage));
        if (responseQueue && initSerialSystem()) {
            initialized = true;
            ESP_LOGI(TAG, "ThreadSafeSerial initialized");
        } else {
            ESP_LOGE(TAG, "ThreadSafeSerial initialization failed");
        }
    }

    ~ThreadSafeSerial() {
        if (responseQueue) {
            vQueueDelete(responseQueue);
            ESP_LOGI(TAG, "ThreadSafeSerial response queue deleted");
        }
    }

    size_t write(uint8_t c) override {
        if (!initialized) return 0;
        char buffer[2] = {(char)c, 0};
        return write((const uint8_t*)buffer, 1);
    }

    size_t write(const uint8_t *buffer, size_t size) override {
        if (!initialized || !serialWriteQueue || size >= 256) {
            ESP_LOGW(TAG, "Write failed: initialized=%d, queue=%p, size=%d", 
                    initialized, serialWriteQueue, size);
            return 0;
        }
        
        portENTER_CRITICAL(&instanceMux);
        SerialMessage msg;
        msg.type = SerialMessage::WRITE;
        memcpy(msg.data.writeBuffer, buffer, size);
        msg.size = size;
        msg.responseQueue = NULL;
        
        size_t result = xQueueSend(serialWriteQueue, &msg, pdMS_TO_TICKS(100)) == pdTRUE ? size : 0;
        portEXIT_CRITICAL(&instanceMux);
        
        if (result == 0) {
            ESP_LOGW(TAG, "Write queue send failed");
        }
        return result;
    }

    int available() override {
        if (!initialized || !serialWriteQueue || !responseQueue) return 0;
        
        portENTER_CRITICAL(&instanceMux);
        SerialMessage msg;
        msg.type = SerialMessage::AVAILABLE;
        msg.responseQueue = responseQueue;
        
        bool success = xQueueSend(serialWriteQueue, &msg, pdMS_TO_TICKS(100)) == pdTRUE;
        if (success) {
            success = xQueueReceive(responseQueue, &msg, pdMS_TO_TICKS(100)) == pdTRUE;
        }
        portEXIT_CRITICAL(&instanceMux);
        
        if (!success) {
            ESP_LOGW(TAG, "Available operation failed");
        }
        return success ? msg.data.readResult : 0;
    }

    int read() override {
        if (!initialized || !serialWriteQueue || !responseQueue) return -1;
        
        portENTER_CRITICAL(&instanceMux);
        SerialMessage msg;
        msg.type = SerialMessage::READ;
        msg.responseQueue = responseQueue;
        
        bool success = xQueueSend(serialWriteQueue, &msg, pdMS_TO_TICKS(100)) == pdTRUE;
        if (success) {
            success = xQueueReceive(responseQueue, &msg, pdMS_TO_TICKS(100)) == pdTRUE;
        }
        portEXIT_CRITICAL(&instanceMux);
        
        if (!success) {
            ESP_LOGW(TAG, "Read operation failed");
        }
        return success ? msg.data.readResult : -1;
    }

    int peek() override {
        if (!initialized || !serialWriteQueue || !responseQueue) return -1;
        
        portENTER_CRITICAL(&instanceMux);
        SerialMessage msg;
        msg.type = SerialMessage::PEEK;
        msg.responseQueue = responseQueue;
        
        bool success = xQueueSend(serialWriteQueue, &msg, pdMS_TO_TICKS(100)) == pdTRUE;
        if (success) {
            success = xQueueReceive(responseQueue, &msg, pdMS_TO_TICKS(100)) == pdTRUE;
        }
        portEXIT_CRITICAL(&instanceMux);
        
        if (!success) {
            ESP_LOGW(TAG, "Peek operation failed");
        }
        return success ? msg.data.readResult : -1;
    }

    void flush() override {
        if (!initialized || !serialWriteQueue) return;
        
        portENTER_CRITICAL(&instanceMux);
        SerialMessage msg;
        msg.type = SerialMessage::FLUSH;
        msg.responseQueue = NULL;
        bool success = xQueueSend(serialWriteQueue, &msg, pdMS_TO_TICKS(100)) == pdTRUE;
        portEXIT_CRITICAL(&instanceMux);
        
        if (!success) {
            ESP_LOGW(TAG, "Flush operation failed");
        }
    }
};

// Create a global instance of the thread-safe Serial wrapper
static ThreadSafeSerial threadSafeSerial;

// Initialize static members
CommandHandler *CommandHandler::instance = nullptr;
QueueHandle_t CommandHandler::commandQueue = nullptr;

// Define command array with proper initialization
const Command CommandHandler::commands[] = {
    COMMAND(CommandHandler::cmdHelp, "help", NULL, "Shows this help message"),
    COMMAND(CommandHandler::cmdStatus, "status", NULL, "Gets current system status"),
    COMMAND(CommandHandler::cmdPower, "power", NULL, "Simulates power button press"),
    COMMAND(CommandHandler::cmdReset, "reset", NULL, "Simulates reset button press")};

// Create larger buffers for command processing
static char serialBuffer[1024];  // Reduced from 4096
static char telegramBuffer[1024]; // Reduced from 4096

CommandHandler::CommandHandler(ComputerController *controller)
    : controller(controller),
      telegramPipe(1024),  // Initialize pipe first
      serialCommandsSerial(Serial, commands, sizeof(commands) / sizeof(Command), serialBuffer, sizeof(serialBuffer)),
      serialCommandsTelegram(telegramPipe.first, commands, sizeof(commands) / sizeof(Command), telegramBuffer, sizeof(telegramBuffer)),
      serialCheckTimer(500),  // Initialize serialCheckTimer
      telegramUpdateTimer(5000),  // Initialize telegramUpdateTimer
      watchdogTimer(5000)
{
    ESP_LOGI(TAG, "Initializing CommandHandler");
    m_currentTelegramChatId = "";
    
    // Set instance after object is fully constructed
    if (instance == nullptr) {
        instance = this;
        ESP_LOGI(TAG, "CommandHandler instance set");
    } else {
        ESP_LOGE(TAG, "Multiple CommandHandler instances detected!");
    }
}

CommandHandler::~CommandHandler() {
    ESP_LOGI(TAG, "Destroying CommandHandler");
    // Clear instance pointer if it points to this object
    if (instance == this) {
        instance = nullptr;
        ESP_LOGI(TAG, "CommandHandler instance cleared");
    }
}

void CommandHandler::handleSerialCommands()
{
    if (!serialCheckTimer.isReady()) {
        return;
    }
    serialCheckTimer.reset();

    // Set a timeout for serial command processing
    SimpleTimer<> processTimer(50);  // 50ms timeout for processing

    // Process serial commands with timeout
    while (Serial.available() && !processTimer.isReady())
    {
        serialCommandsSerial.readSerial();
        vTaskDelay(pdMS_TO_TICKS(1));
    }

    // If processing took too long, log it
    if (processTimer.isReady())
    {
        ESP_LOGW(TAG, "Serial command processing took too long");
    }
}

// Internal helper to process a single Telegram message
void CommandHandler::handleTelegramCommandsInternal(const String &chatId, const String &text, const String &fromName)
{
    if (!instance) {
        ESP_LOGE(TAG, "CommandHandler::instance is null in handleTelegramCommandsInternal!");
        return;
    }

    if (!instance->telegramPipe.second.available()) {
        ESP_LOGE(TAG, "Telegram pipe not available!");
        return;
    }

    ESP_LOGI(TAG, "Telegram msg from %s (%s): %s", fromName.c_str(), chatId.c_str(), text.c_str());

    // Store chat_id temporarily for the callback to access
    instance->m_currentTelegramChatId = chatId;

    unsigned long pipeStartTime = millis();
    
    // Write command to pipe for StaticSerialCommands with timeout
    if (!instance->telegramPipe.second.print(text)) {
        ESP_LOGE(TAG, "Failed to write to telegram pipe!");
        instance->m_currentTelegramChatId = "";
        return;
    }
    instance->telegramPipe.second.print('\n');

    // Process command from pipe with timeout
    unsigned long processStartTime = millis();
    instance->serialCommandsTelegram.readSerial();
    
    // Clear the temporary chat_id
    instance->m_currentTelegramChatId = "";

    if (millis() - pipeStartTime > 1000) {
        ESP_LOGW(TAG, "Telegram command processing took too long.");
    }
}

void CommandHandler::handleTelegramCommands()
{
    if (!instance || !controller) {
        ESP_LOGE(TAG, "CommandHandler::instance or controller is null in handleTelegramCommands!");
        return;
    }

    if (!telegramUpdateTimer.isReady()) {
        return;
    }
    telegramUpdateTimer.reset();

    auto &bot = controller->getTelegramBot();

    SimpleTimer<> updateTimer(2000);  // 2 second timeout for updates
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

    if (updateTimer.isReady())
    {
        ESP_LOGW(TAG, "getUpdates timed out");
        return;
    }

    if (numNewMessages > 0)
    {
        ESP_LOGI(TAG, "Processing %d new messages", numNewMessages);
        for (int i = 0; i < numNewMessages; i++)
        {
            handleTelegramCommandsInternal(
                String(bot.messages[i].chat_id),
                bot.messages[i].text,
                bot.messages[i].from_name);
        }
    }
}

// Command execution task (runs on Core 1)
void commandExecutionTask(void *pvParameters)
{
    SimpleTimer<> watchdogTimer(5000);  // Feed watchdog every 5 seconds
    CommandHandler* localInstance = nullptr;

    while (1)
    {
        SafeCommandTask task;
        if (xQueueReceive(CommandHandler::commandQueue, &task, pdMS_TO_TICKS(5000)) == pdTRUE)
        {
            esp_task_wdt_reset();
            
            if (!task.sender) {
                ESP_LOGE(TAG, "Received task with null sender");
                continue;
            }

            // Get a local copy of the instance pointer
            localInstance = CommandHandler::instance;
            if (!localInstance) {
                ESP_LOGE(TAG, "CommandHandler instance is null in commandExecutionTask");
                continue;
            }

            switch (task.type)
            {
            case CMD_TYPE_POWER:
                CommandHandler::executePower(task);
                break;
            case CMD_TYPE_RESET:
                CommandHandler::executeReset(task);
                break;
            case CMD_TYPE_STATUS:
                CommandHandler::executeStatus(task);
                break;
            case CMD_TYPE_HELP:
            default:
                CommandHandler::executeHelp(task);
                break;
            }
            
            esp_task_wdt_reset();
        }
        else {
            if (watchdogTimer.isReady()) {
                esp_task_wdt_reset();
                watchdogTimer.reset();
            }
        }
    }
}

// Command Handler loop task (runs on Core 0)
void commandHandlerTask(void *pvParameters)
{
    ESP_LOGI(TAG, "Command handler task started");
    SimpleTimer<> watchdogTimer(5000);  // Feed watchdog every 5 seconds
    CommandHandler* localInstance = nullptr;

    while (1)
    {
        ESP_LOGD(TAG, "Starting task loop iteration");
        
        // Get a local copy of the instance pointer
        localInstance = CommandHandler::instance;
        if (!localInstance) {
            ESP_LOGE(TAG, "CommandHandler::instance is null in commandHandlerTask!");
            vTaskDelay(pdMS_TO_TICKS(1000));
            continue;
        }

        if (watchdogTimer.isReady()) {
            esp_task_wdt_reset();
            watchdogTimer.reset();
            ESP_LOGD(TAG, "Watchdog reset");
        }

        ESP_LOGD(TAG, "About to handle serial commands...");
        localInstance->handleSerialCommands();
        ESP_LOGD(TAG, "Serial commands handled");
        
        ESP_LOGD(TAG, "About to handle telegram commands...");
        localInstance->handleTelegramCommands();
        ESP_LOGD(TAG, "Telegram commands handled");
        
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

void CommandHandler::executePower(const SafeCommandTask &task)
{
    if (!instance || !task.sender) {
        ESP_LOGE(TAG, "Invalid instance or sender in executePower");
        return;
    }
    ESP_LOGI(TAG, "Executing power command");
    instance->pressRelayButton(POWER_RELAY_PIN);
    task.sender->getSerial().println("Power button pressed.");
    Utilities::printOK(*(task.sender));
}

void CommandHandler::executeReset(const SafeCommandTask &task)
{
    if (!instance || !task.sender) {
        ESP_LOGE(TAG, "Invalid instance or sender in executeReset");
        return;
    }
    ESP_LOGI(TAG, "Executing reset command");
    instance->pressRelayButton(RESET_RELAY_PIN);
    task.sender->getSerial().println(F("Reset button pressed."));
    Utilities::printOK(*(task.sender));
}

void CommandHandler::executeStatus(const SafeCommandTask &task)
{
    if (!instance || !task.sender) {
        ESP_LOGE(TAG, "Invalid instance or sender in executeStatus");
        return;
    }
    ESP_LOGI(TAG, "Executing status command");
    String response = "Status: System Nominal. Free Heap: " + String(ESP.getFreeHeap());
    task.sender->getSerial().println(response);
    Utilities::printOK(*(task.sender));
}

void CommandHandler::executeHelp(const SafeCommandTask &task)
{
    if (!instance || !task.sender) {
        ESP_LOGE(TAG, "Invalid instance or sender in executeHelp");
        return;
    }
    ESP_LOGI(TAG, "Executing help command");
    task.sender->listCommands();
    Utilities::printOK(*(task.sender));
}

void CommandHandler::queueSafeCommand(CommandType_t type, SerialCommands *sender)
{
    if (!CommandHandler::instance || !sender) {
        ESP_LOGE(TAG, "Invalid instance or sender in queueSafeCommand");
        return;
    }

    if (commandQueue == NULL) {
        ESP_LOGE(TAG, "Command queue not initialized");
        return;
    }

    SafeCommandTask cmd_task;
    cmd_task.sender = sender;
    cmd_task.type = type;

    if (xQueueSendToBack(commandQueue, &cmd_task, pdMS_TO_TICKS(100)) != pdTRUE) {
        ESP_LOGE(TAG, "Failed to queue command");
    } else {
        ESP_LOGD(TAG, "Command queued successfully");
    }
}

// Static command callbacks for StaticSerialCommands
void CommandHandler::cmdPower(SerialCommands &sender, Args &args)
{
    queueSafeCommand(CMD_TYPE_POWER, &sender);
}

void CommandHandler::cmdReset(SerialCommands &sender, Args &args)
{
    queueSafeCommand(CMD_TYPE_RESET, &sender);
}

void CommandHandler::cmdStatus(SerialCommands &sender, Args &args)
{
    queueSafeCommand(CMD_TYPE_STATUS, &sender);
}

void CommandHandler::cmdHelp(SerialCommands &sender, Args &args)
{
    queueSafeCommand(CMD_TYPE_HELP, &sender);
}

void CommandHandler::pressRelayButton(int pin)
{
    ESP_LOGI(TAG, "Pressing relay button on pin %d", pin);
    digitalWrite(pin, HIGH);
    delay(RELAY_BUTTON_PRESS_DURATION);
    digitalWrite(pin, LOW);
    ESP_LOGI(TAG, "Relay button released");
}

void CommandHandler::setup()
{
    ESP_LOGI(TAG, "Setting up CommandHandler");
    
    // Create command queue with proper size
    commandQueue = xQueueCreate(10, sizeof(SafeCommandTask));
    if (commandQueue == NULL) {
        ESP_LOGE(TAG, "Failed to create command queue");
        return;
    }
    ESP_LOGI(TAG, "Command queue created successfully");
    
    // Create command handler task (runs on Core 0)
    BaseType_t result = xTaskCreatePinnedToCore(
        commandHandlerTask,
        "CmdHandler",
        8192,  // Stack size
        NULL,
        1,     // Priority
        NULL,
        0      // Core 0
    );
    
    if (result != pdPASS) {
        ESP_LOGE(TAG, "Failed to create command handler task");
        return;
    }
    
    // Create command execution task (runs on Core 1)
    result = xTaskCreatePinnedToCore(
        commandExecutionTask,
        "CmdExec",
        8192,  // Stack size
        NULL,
        1,     // Priority
        NULL,
        1      // Core 1
    );
    
    if (result != pdPASS) {
        ESP_LOGE(TAG, "Failed to create command execution task");
        return;
    }
    
    ESP_LOGI(TAG, "Command handler tasks created successfully");
}