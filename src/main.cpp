#include "ComputerController.h"
#include "esp_task_wdt.h"
#include "esp_log.h"
#include <WiFi.h>

// Define log tag for this file
static const char* TAG = "Main";

// Global pointer to the ComputerController instance
static ComputerController* controller = nullptr;

// Global pointer for MQTT manager access
ComputerController* g_computerController = nullptr;

/**
 * @brief Setup function, called once at startup.
 * 
 * Initializes serial communication, persistent settings, and the main controller.
 * The controller setup ensures display initialization happens before WiFi connections.
 */
void setup() {
    Serial.begin(115200);
    Serial.println("Starting Computer Controller...");
    
    // Configure watchdog
    esp_task_wdt_init(10, true); // 10 second timeout, panic on timeout
    esp_task_wdt_add(NULL);
    
    ESP_LOGI(TAG, "Initializing Computer Controller...");
    
    // Initialize persistent settings
    PersistentSettings::getInstance().begin();
    
    // Create and setup the main controller
    // Note: Display will be initialized before WiFi connections in controller->setup()
    controller = new ComputerController();
    g_computerController = controller;  // Assign to global pointer for MQTT access
    
    ESP_LOGI(TAG, "About to call controller->setup()...");
    controller->setup();
    ESP_LOGI(TAG, "controller->setup() completed");
    
    ESP_LOGI(TAG, "Computer Controller Ready");
    Serial.println("Computer Controller Ready");
    Serial.println("Type 'help' for available commands");
}

/**
 * @brief Main loop function, called repeatedly.
 * 
 * Runs the main controller's loop and ensures a minimum loop execution time.
 */
void loop() {    
    esp_task_wdt_reset();
    
    static uint32_t loopCount = 0;
    if (loopCount % 10000 == 0) { // Log every 10000th iteration
        ESP_LOGI(TAG, "Main loop iteration: %lu", loopCount);
    }
    loopCount++;
       
    controller->loop();
    
    yield();
} 