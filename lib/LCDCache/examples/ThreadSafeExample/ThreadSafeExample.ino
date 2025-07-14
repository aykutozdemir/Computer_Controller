/*
  LCDCache Thread-Safe Example
  
  This example demonstrates thread-safe usage of the LCDCache library.
  It shows how to safely access the cache from multiple tasks.
  
  Created for the Computer Controller project
*/

#include "LCDCache.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// Create a cache for a 320x240 display with black background
LCDCache cache(320, 240, 0x000000);

// Task handles
TaskHandle_t task1Handle = NULL;
TaskHandle_t task2Handle = NULL;

// Task 1: Draws horizontal lines
void task1(void *parameter) {
  uint16_t y = 0;
  uint32_t colors[] = {0xFF0000, 0x00FF00, 0x0000FF, 0xFFFF00, 0xFF00FF};
  uint8_t colorIndex = 0;
  
  while (1) {
    // Lock the cache for thread-safe access
    cache.lock();
    
    // Draw a horizontal line
    for (uint16_t x = 0; x < cache.getWidth(); x++) {
      cache.setPixel(x, y, colors[colorIndex]);
    }
    
    // Unlock the cache
    cache.unlock();
    
    // Move to next row and color
    y = (y + 1) % cache.getHeight();
    colorIndex = (colorIndex + 1) % 5;
    
    // Wait a bit
    vTaskDelay(pdMS_TO_TICKS(100));
  }
}

// Task 2: Draws vertical lines
void task2(void *parameter) {
  uint16_t x = 0;
  uint32_t colors[] = {0x00FFFF, 0xFF8000, 0x8000FF, 0x80FF80, 0xFF8080};
  uint8_t colorIndex = 0;
  
  while (1) {
    // Lock the cache for thread-safe access
    cache.lock();
    
    // Draw a vertical line
    for (uint16_t y = 0; y < cache.getHeight(); y++) {
      cache.setPixel(x, y, colors[colorIndex]);
    }
    
    // Unlock the cache
    cache.unlock();
    
    // Move to next column and color
    x = (x + 1) % cache.getWidth();
    colorIndex = (colorIndex + 1) % 5;
    
    // Wait a bit
    vTaskDelay(pdMS_TO_TICKS(150));
  }
}

// Task 3: Monitors and updates display
void task3(void *parameter) {
  while (1) {
    // Check for dirty rows and update display
    uint16_t dirtyCount = 0;
    
    for (uint16_t y = 0; y < cache.getHeight(); y++) {
      if (cache.isRowDirty(y)) {
        dirtyCount++;
        
        // Simulate display update
        Serial.printf("Updating row %d\n", y);
        
        // Mark row as clean after update
        cache.markRowClean(y);
      }
    }
    
    if (dirtyCount > 0) {
      Serial.printf("Updated %d dirty rows\n", dirtyCount);
    }
    
    // Wait before next check
    vTaskDelay(pdMS_TO_TICKS(50));
  }
}

void setup() {
  Serial.begin(115200);
  Serial.println("LCDCache Thread-Safe Example");
  Serial.println("Starting tasks...");
  
  // Create tasks
  xTaskCreatePinnedToCore(
    task1,           // Task function
    "Task1",         // Task name
    4096,            // Stack size
    NULL,            // Task parameters
    1,               // Task priority
    &task1Handle,    // Task handle
    0                // Core to run on
  );
  
  xTaskCreatePinnedToCore(
    task2,           // Task function
    "Task2",         // Task name
    4096,            // Stack size
    NULL,            // Task parameters
    1,               // Task priority
    &task2Handle,    // Task handle
    1                // Core to run on
  );
  
  xTaskCreatePinnedToCore(
    task3,           // Task function
    "Task3",         // Task name
    4096,            // Stack size
    NULL,            // Task parameters
    2,               // Task priority
    &task3Handle,    // Task handle
    0                // Core to run on
  );
  
  Serial.println("Tasks started successfully!");
}

void loop() {
  // Main loop is not used in this example
  // All work is done in the FreeRTOS tasks
  vTaskDelay(pdMS_TO_TICKS(1000));
} 