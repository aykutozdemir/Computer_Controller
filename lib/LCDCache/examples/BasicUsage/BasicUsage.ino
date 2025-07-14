/*
  LCDCache Basic Usage Example
  
  This example demonstrates the basic usage of the LCDCache library.
  It shows how to create a cache, set pixels, and track dirty rows.
  
  Created for the Computer Controller project
*/

#include "LCDCache.h"

// Create a cache for a 320x240 display with black background
LCDCache cache(320, 240, 0x000000);

void setup() {
  Serial.begin(115200);
  Serial.println("LCDCache Basic Usage Example");
  
  // Set some pixels to different colors
  cache.setPixel(10, 20, 0xFF0000);  // Red
  cache.setPixel(11, 20, 0xFF0000);  // Red (will be part of same run)
  cache.setPixel(12, 20, 0xFF0000);  // Red (will be part of same run)
  cache.setPixel(15, 20, 0x00FF00);  // Green
  cache.setPixel(16, 20, 0x00FF00);  // Green (will be part of same run)
  cache.setPixel(20, 20, 0x0000FF);  // Blue
  
  // Set pixels in a different row
  cache.setPixel(10, 25, 0xFFFF00);  // Yellow
  cache.setPixel(11, 25, 0xFFFF00);  // Yellow
  cache.setPixel(12, 25, 0xFFFF00);  // Yellow
  
  // Display cache information
  Serial.printf("Cache dimensions: %dx%d\n", cache.getWidth(), cache.getHeight());
  Serial.println("Background color: 0x000000 (black)");
  
  // Check which rows are dirty
  Serial.println("Dirty rows:");
  for (uint16_t y = 0; y < cache.getHeight(); y++) {
    if (cache.isRowDirty(y)) {
      Serial.printf("  Row %d is dirty\n", y);
    }
  }
  
  // Get pixel colors
  Serial.println("Pixel colors:");
  Serial.printf("  Pixel (10,20): 0x%06X\n", cache.getPixel(10, 20));
  Serial.printf("  Pixel (15,20): 0x%06X\n", cache.getPixel(15, 20));
  Serial.printf("  Pixel (20,20): 0x%06X\n", cache.getPixel(20, 20));
  Serial.printf("  Pixel (10,25): 0x%06X\n", cache.getPixel(10, 25));
  Serial.printf("  Pixel (50,50): 0x%06X (background)\n", cache.getPixel(50, 50));
  
  // Simulate updating the display
  Serial.println("Updating display...");
  for (uint16_t y = 0; y < cache.getHeight(); y++) {
    if (cache.isRowDirty(y)) {
      Serial.printf("  Updating row %d\n", y);
      // Here you would actually update your LCD display
      // For now, we just mark the row as clean
      cache.markRowClean(y);
    }
  }
  
  // Clear the cache
  Serial.println("Clearing cache...");
  cache.clear();
  
  // Verify cache is cleared
  Serial.printf("  Pixel (10,20) after clear: 0x%06X (background)\n", cache.getPixel(10, 20));
}

void loop() {
  // This example runs once in setup()
  delay(1000);
} 