#include <Arduino.h>
#include "../include/DisplayManager.h"
#include "../include/Globals.h"
#include <TFT_eSPI.h>
#include <SPI.h>

// Pin definitions are now in platformio.ini

DisplayManager::DisplayManager() : 
    Traceable(F("DisplayManager"), static_cast<Level>(DEBUG_DISPLAY_MANAGER)),
    tft()
{
    TRACE_INFO() << F("DisplayManager constructed");
}

void DisplayManager::begin()
{
    TRACE_INFO() << F("Starting display initialization");
    
    // Initialize display
    tft.init();
    delay(100);  // Give display time to initialize
    
    // Set rotation to portrait mode
    tft.setRotation(0);
    delay(100);
    
    // Clear screen and set text parameters
    tft.fillScreen(DISPLAY_BLACK);
    tft.setTextColor(DISPLAY_WHITE);
    tft.setTextSize(2);
    tft.setTextWrap(false);
    
    TRACE_INFO() << F("Display initialization complete");
    
    // Test pattern
    try {
        TRACE_INFO() << F("Starting display test pattern");
        
        // Red screen
        tft.fillScreen(DISPLAY_RED);
        tft.setCursor(10, 10);
        tft.println("Red Screen");
        delay(1000);
        
        // Green screen
        tft.fillScreen(DISPLAY_GREEN);
        tft.setCursor(10, 10);
        tft.println("Green Screen");
        delay(1000);
        
        // Blue screen
        tft.fillScreen(DISPLAY_BLUE);
        tft.setCursor(10, 10);
        tft.println("Blue Screen");
        delay(1000);
        
        // Final black screen with white text
        tft.fillScreen(DISPLAY_BLACK);
        tft.setCursor(10, 10);
        tft.println("Display Test Complete");
        
        TRACE_INFO() << F("Display test pattern complete");
    } catch (const std::exception& e) {
        TRACE_ERROR() << F("Error during display test pattern: ") << e.what();
    }
}

void DisplayManager::update()
{
    TRACE_DEBUG() << F("Display update called");
}

void DisplayManager::showMessage(const char* message)
{
    TRACE_INFO() << F("Showing message: ") << message;
    tft.fillScreen(DISPLAY_BLACK);
    tft.setCursor(10, 10);
    tft.println(message);
}

void DisplayManager::showError(const char* error)
{
    TRACE_ERROR() << F("Showing error: ") << error;
    tft.fillScreen(DISPLAY_RED);
    tft.setCursor(10, 10);
    tft.println(error);
}

void DisplayManager::showSuccess(const char* message)
{
    TRACE_INFO() << F("Showing success: ") << message;
    tft.fillScreen(DISPLAY_GREEN);
    tft.setCursor(10, 10);
    tft.println(message);
}

void DisplayManager::clear()
{
    TRACE_DEBUG() << F("Clearing display");
    tft.fillScreen(DISPLAY_BLACK);
}

void DisplayManager::setTextSize(uint8_t size)
{
    TRACE_DEBUG() << F("Setting text size: ") << size;
    tft.setTextSize(size);
}

void DisplayManager::setTextColor(uint16_t color)
{
    TRACE_DEBUG() << F("Setting text color: 0x") << String(color, HEX);
    tft.setTextColor(color);
}

void DisplayManager::setCursor(int16_t x, int16_t y)
{
    TRACE_DEBUG() << F("Setting cursor position: (") << x << F(",") << y << F(")");
    tft.setCursor(x, y);
}

void DisplayManager::print(const char* text)
{
    TRACE_DEBUG() << F("Printing text: ") << text;
    tft.print(text);
}

void DisplayManager::println(const char* text)
{
    TRACE_DEBUG() << F("Printing line: ") << text;
    tft.println(text);
}

void DisplayManager::drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color)
{
    TRACE_DEBUG() << F("Drawing rectangle: (") << x << F(",") << y << F(") ") << w << F("x") << h;
    tft.drawRect(x, y, w, h, color);
}

void DisplayManager::fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color)
{
    TRACE_DEBUG() << F("Filling rectangle: (") << x << F(",") << y << F(") ") << w << F("x") << h;
    tft.fillRect(x, y, w, h, color);
}

void DisplayManager::drawCircle(int16_t x, int16_t y, int16_t r, uint16_t color)
{
    TRACE_DEBUG() << F("Drawing circle: (") << x << F(",") << y << F(") r=") << r;
    tft.drawCircle(x, y, r, color);
}

void DisplayManager::fillCircle(int16_t x, int16_t y, int16_t r, uint16_t color)
{
    TRACE_DEBUG() << F("Filling circle: (") << x << F(",") << y << F(") r=") << r;
    tft.fillCircle(x, y, r, color);
}

void DisplayManager::drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color)
{
    TRACE_DEBUG() << F("Drawing line: (") << x0 << F(",") << y0 << F(") to (") << x1 << F(",") << y1 << F(")");
    tft.drawLine(x0, y0, x1, y1, color);
}

void DisplayManager::drawPixel(int16_t x, int16_t y, uint16_t color)
{
    TRACE_DEBUG() << F("Drawing pixel: (") << x << F(",") << y << F(")");
    tft.drawPixel(x, y, color);
}

void DisplayManager::display()
{
    TRACE_DEBUG() << F("Display refresh called");
    // For TFT_eSPI, we don't need to call display() as it's not buffered
    // This method is kept for compatibility with other display types
} 