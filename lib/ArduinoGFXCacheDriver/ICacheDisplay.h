#pragma once
#include <stdint.h>
#include <Arduino.h>

class ICacheDisplay {
public:
    virtual ~ICacheDisplay() = default;
    virtual void drawPixel(int16_t x, int16_t y, uint32_t color) = 0;
    virtual void drawString(const String& text, int16_t x, int16_t y, uint32_t color, uint32_t bg) = 0;
    virtual int16_t getWidth() const = 0;
    virtual int16_t getHeight() const = 0;
    
    // Display control methods
    virtual bool begin() = 0;
    virtual void setRotation(uint8_t rotation) = 0;
    virtual void fillScreen(uint32_t color) = 0;
    
    // Additional drawing methods needed by widgets
    virtual void fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint32_t color) = 0;
    virtual void updateCachePixel(uint16_t x, uint16_t y, uint16_t color) = 0;
    virtual void updateCache() = 0;
    
    // Text methods needed by widgets
    virtual void setTextSize(uint8_t size) = 0;
    virtual int16_t textWidth(const String& text) = 0;
    virtual int16_t fontHeight() = 0;
}; 