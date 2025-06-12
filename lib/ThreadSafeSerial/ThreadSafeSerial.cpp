#include "ThreadSafeSerial.h"

// Initialize static members
const char* ThreadSafeSerial::TAG = "ThreadSafeSerial";
QueueHandle_t ThreadSafeSerial::serialWriteQueue = NULL;
TaskHandle_t ThreadSafeSerial::serialTaskHandle = NULL;
portMUX_TYPE ThreadSafeSerial::serialMux = portMUX_INITIALIZER_UNLOCKED;

// Serial task that runs on Core 0
void IRAM_ATTR ThreadSafeSerial::serialTask(void* arg) {
    SerialMessage msg;
    SimpleTimer<> watchdogTimer(1000);  // Feed watchdog every 1 second
    
    while (1) {
        esp_task_wdt_reset();  // Reset watchdog at the start of each loop
        
        // Use a shorter timeout for queue receive
        if (xQueueReceive(serialWriteQueue, &msg, pdMS_TO_TICKS(100)) == pdTRUE) {
            esp_task_wdt_reset();  // Reset watchdog before critical section
            
            // Check if Serial is available before entering critical section
            if (!Serial) {
                ESP_LOGW(TAG, "Serial not available");
                continue;
            }
            
            portENTER_CRITICAL(&serialMux);
            esp_task_wdt_reset();  // Reset watchdog before operations
            
            bool success = true;
            switch (msg.type) {
                case SerialMessage::WRITE:
                    if (Serial) {
                        size_t written = Serial.write(msg.data.writeBuffer, msg.size);
                        if (written != msg.size) {
                            ESP_LOGW(TAG, "Incomplete write: %d/%d bytes", written, msg.size);
                            success = false;
                        }
                        Serial.flush();  // Ensure data is sent
                    }
                    break;
                    
                case SerialMessage::READ:
                    msg.data.readResult = Serial ? Serial.read() : -1;
                    if (msg.responseQueue) {
                        if (xQueueSend(msg.responseQueue, &msg, pdMS_TO_TICKS(50)) != pdTRUE) {
                            ESP_LOGW(TAG, "Failed to send read response");
                            success = false;
                        }
                    }
                    break;
                    
                case SerialMessage::AVAILABLE:
                    msg.data.readResult = Serial ? Serial.available() : 0;
                    if (msg.responseQueue) {
                        if (xQueueSend(msg.responseQueue, &msg, pdMS_TO_TICKS(50)) != pdTRUE) {
                            ESP_LOGW(TAG, "Failed to send available response");
                            success = false;
                        }
                    }
                    break;
                    
                case SerialMessage::PEEK:
                    msg.data.readResult = Serial ? Serial.peek() : -1;
                    if (msg.responseQueue) {
                        if (xQueueSend(msg.responseQueue, &msg, pdMS_TO_TICKS(50)) != pdTRUE) {
                            ESP_LOGW(TAG, "Failed to send peek response");
                            success = false;
                        }
                    }
                    break;
                    
                case SerialMessage::FLUSH:
                    if (Serial) {
                        Serial.flush();
                    }
                    break;
            }
            
            portEXIT_CRITICAL(&serialMux);
            esp_task_wdt_reset();  // Reset watchdog after critical section
            
            if (!success) {
                ESP_LOGW(TAG, "Operation failed");
            }
        }
        
        if (watchdogTimer.isReady()) {
            esp_task_wdt_reset();
            watchdogTimer.reset();
        }
        
        // Small delay to prevent tight loop
        vTaskDelay(pdMS_TO_TICKS(1));
    }
}

// Initialize Serial communication system
bool ThreadSafeSerial::initSerialSystem() {
    if (serialWriteQueue != NULL) return true;  // Already initialized
    
    serialWriteQueue = xQueueCreate(20, sizeof(SerialMessage));
    if (!serialWriteQueue) {
        ESP_LOGE(TAG, "Failed to create serial write queue");
        return false;
    }
    
    BaseType_t result = xTaskCreatePinnedToCore(
        serialTask,
        "SerialTask",
        8192,  // Increased stack size
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
    
    ESP_LOGI(TAG, "Serial system initialized successfully");
    return true;
}

// Cleanup Serial communication system
void ThreadSafeSerial::cleanupSerialSystem() {
    if (serialTaskHandle) {
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

ThreadSafeSerial::ThreadSafeSerial() : initialized(false) {
    instanceMux = portMUX_INITIALIZER_UNLOCKED;
    responseQueue = xQueueCreate(1, sizeof(SerialMessage));
    if (responseQueue && initSerialSystem()) {
        initialized = true;
        ESP_LOGI(TAG, "ThreadSafeSerial initialized");
    } else {
        ESP_LOGE(TAG, "ThreadSafeSerial initialization failed");
    }
}

ThreadSafeSerial::~ThreadSafeSerial() {
    if (responseQueue) {
        vQueueDelete(responseQueue);
        ESP_LOGI(TAG, "ThreadSafeSerial response queue deleted");
    }
}

size_t ThreadSafeSerial::write(const uint8_t *buffer, size_t size) {
    if (!initialized || !buffer || size == 0) return 0;
    
    // Split large writes into smaller chunks
    const size_t MAX_CHUNK = 32;  // Reduced chunk size
    size_t totalWritten = 0;
    
    while (totalWritten < size) {
        size_t chunkSize = min(MAX_CHUNK, size - totalWritten);
        SerialMessage msg;
        msg.type = SerialMessage::WRITE;
        msg.size = chunkSize;
        memcpy(msg.data.writeBuffer, buffer + totalWritten, chunkSize);
        
        // Use a shorter timeout for queue send
        if (xQueueSend(serialWriteQueue, &msg, pdMS_TO_TICKS(50)) != pdTRUE) {
            ESP_LOGW(TAG, "Failed to queue serial write");
            break;
        }
        
        totalWritten += chunkSize;
        vTaskDelay(pdMS_TO_TICKS(2));  // Increased delay between chunks
    }
    
    return totalWritten;
}

int ThreadSafeSerial::available() {
    if (!initialized) return 0;
    
    SerialMessage msg;
    msg.type = SerialMessage::AVAILABLE;
    msg.responseQueue = responseQueue;
    
    // Use a shorter timeout for queue operations
    if (xQueueSend(serialWriteQueue, &msg, pdMS_TO_TICKS(50)) != pdTRUE) {
        ESP_LOGW(TAG, "Failed to queue available request");
        return 0;
    }
    
    if (xQueueReceive(responseQueue, &msg, pdMS_TO_TICKS(50)) != pdTRUE) {
        ESP_LOGW(TAG, "Failed to receive available response");
        return 0;
    }
    
    return msg.data.readResult;
}

int ThreadSafeSerial::read() {
    if (!initialized) return -1;
    
    SerialMessage msg;
    msg.type = SerialMessage::READ;
    msg.responseQueue = responseQueue;
    
    // Use a shorter timeout for queue operations
    if (xQueueSend(serialWriteQueue, &msg, pdMS_TO_TICKS(50)) != pdTRUE) {
        ESP_LOGW(TAG, "Failed to queue read request");
        return -1;
    }
    
    if (xQueueReceive(responseQueue, &msg, pdMS_TO_TICKS(50)) != pdTRUE) {
        ESP_LOGW(TAG, "Failed to receive read response");
        return -1;
    }
    
    return msg.data.readResult;
}

int ThreadSafeSerial::peek() {
    if (!initialized) return -1;
    
    SerialMessage msg;
    msg.type = SerialMessage::PEEK;
    msg.responseQueue = responseQueue;
    
    // Use a shorter timeout for queue operations
    if (xQueueSend(serialWriteQueue, &msg, pdMS_TO_TICKS(50)) != pdTRUE) {
        ESP_LOGW(TAG, "Failed to queue peek request");
        return -1;
    }
    
    if (xQueueReceive(responseQueue, &msg, pdMS_TO_TICKS(50)) != pdTRUE) {
        ESP_LOGW(TAG, "Failed to receive peek response");
        return -1;
    }
    
    return msg.data.readResult;
}

void ThreadSafeSerial::flush() {
    if (!initialized) return;
    
    SerialMessage msg;
    msg.type = SerialMessage::FLUSH;
    msg.responseQueue = NULL;
    
    // Use a shorter timeout for queue operations
    if (xQueueSend(serialWriteQueue, &msg, pdMS_TO_TICKS(50)) != pdTRUE) {
        ESP_LOGW(TAG, "Failed to queue flush request");
    }
}