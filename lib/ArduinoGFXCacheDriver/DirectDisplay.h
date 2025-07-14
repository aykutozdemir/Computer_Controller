#pragma once
#include "ICacheDisplay.h"
#include <Arduino_GFX_Library.h>
#include "esp_log.h"

class DirectDisplay : public ICacheDisplay {
    Arduino_GFX* _gfx;
    uint8_t _textSize = 1; // Track text size manually
public:
    DirectDisplay(Arduino_GFX* gfx) : _gfx(gfx) {}
    void drawPixel(int16_t x, int16_t y, uint32_t color) override {
        if (_gfx) _gfx->drawPixel(x, y, color);
    }
    void drawString(const String& text, int16_t x, int16_t y, uint32_t color, uint32_t bg) override {
        if (_gfx) {
            _gfx->setTextColor(color, bg);
            _gfx->setCursor(x, y);
            _gfx->print(text);
        }
    }
    int16_t getWidth() const override { return _gfx ? _gfx->width() : 0; }
    int16_t getHeight() const override { return _gfx ? _gfx->height() : 0; }
    
    // Display control methods
    bool begin() override { return _gfx ? _gfx->begin() : false; }
    void setRotation(uint8_t rotation) override { if (_gfx) _gfx->setRotation(rotation); }
    void fillScreen(uint32_t color) override { if (_gfx) _gfx->fillScreen(color); }
    
    // Additional drawing methods needed by widgets
    void fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint32_t color) override {
        if (_gfx) _gfx->fillRect(x, y, w, h, color);
    }
    
    void updateCachePixel(uint16_t x, uint16_t y, uint16_t color) override {
        if (_gfx) _gfx->drawPixel(x, y, color);
    }
    
    void updateCache() override {
        // DirectDisplay draws immediately, so no cache to update
    }
    
    // Text methods needed by widgets
    void setTextSize(uint8_t size) override { 
        if (_gfx) {
            _gfx->setTextSize(size);
            _textSize = size; // Track the size
        }
    }
    int16_t textWidth(const String& text) override { 
        if (_gfx) {
            // Use Arduino GFX's getTextBounds to get the actual text width
            int16_t x1, y1;
            uint16_t w, h;
            _gfx->getTextBounds(text, 0, 0, &x1, &y1, &w, &h);
            return w;
        }
        // Fallback to estimate if no GFX object
        return text.length() * 6 * _textSize; 
    }
    int16_t fontHeight() override { 
        if (_gfx) {
            // Use Arduino GFX's getTextBounds to get the actual font height
            int16_t x1, y1;
            uint16_t w, h;
            _gfx->getTextBounds("A", 0, 0, &x1, &y1, &w, &h);
            return h;
        }
        // Fallback to estimate if no GFX object
        return 8 * _textSize; 
    }
}; 