#pragma once

#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_system.h"
#include "SimpleTimer.h"

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

// Thread-safe Serial wrapper
class ThreadSafeSerial : public Stream {
private:
    QueueHandle_t responseQueue;
    bool initialized;
    portMUX_TYPE instanceMux;
    static QueueHandle_t serialWriteQueue;
    static TaskHandle_t serialTaskHandle;
    static portMUX_TYPE serialMux;
    static const char* TAG;

    static void IRAM_ATTR serialTask(void* arg);
    static bool initSerialSystem();
    static void cleanupSerialSystem();

public:
    ThreadSafeSerial();
    ~ThreadSafeSerial();

    // Stream interface implementation
    size_t write(uint8_t c) override;
    size_t write(const uint8_t *buffer, size_t size) override;
    int available() override;
    int read() override;
    int peek() override;
    void flush() override;
}; 