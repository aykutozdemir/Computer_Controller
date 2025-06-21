#include "ComputerController.h"

// Define log tag for this file
static const char* TAG = "main";

// Global pointer to the ComputerController instance
static ComputerController* controller = nullptr;

// Global pointer for MQTT manager access
ComputerController* g_computerController = nullptr;

/**
 * @brief Setup function, called once at startup.
 * 
 * Initializes serial communication, persistent settings, and the main controller.
 */
void setup() {
    Serial.begin(SERIAL_BAUD_RATE);
    while (!Serial) {
        delay(10);
    }
    
    ESP_LOGI(TAG, "Starting Computer Controller");
    
    // Initialize persistent settings
    PersistentSettings::getInstance().begin();
    
    // Create and setup the main controller
    controller = new ComputerController();
    g_computerController = controller;  // Assign to global pointer for MQTT access
    controller->setup();
    
    ESP_LOGI(TAG, "Initialization complete");
}

/**
 * @brief Main loop function, called repeatedly.
 * 
 * Runs the main controller's loop and ensures a minimum loop execution time.
 */
void loop() {
    const uint32_t startTime = micros();    
    controller->loop();
    taskYIELD();
    const uint32_t executionTime = micros() - startTime;
    
    // Ensure a minimum loop time of 1ms (1000 microseconds)
    if (executionTime < 1000) {
        delayMicroseconds(1000 - executionTime);
    }
} 