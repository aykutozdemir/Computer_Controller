#pragma once
#include "ICacheDisplay.h"
#include "LCDCache.h"
#include <Arduino_GFX_Library.h>
#include "esp_log.h"

static const char* CACHE_DISPLAY_TAG = "CacheDisplay";

class CacheDisplay : public ICacheDisplay {
    LCDCache* _cache;
    Arduino_GFX* _realGfx; // Add reference to real GFX driver
    uint8_t _textSize = 1; // Track text size manually
public:
    CacheDisplay(LCDCache* cache, Arduino_GFX* realGfx) : _cache(cache), _realGfx(realGfx) {}
    void drawPixel(int16_t x, int16_t y, uint32_t color) override {
        _cache->setPixel(x, y, color);
    }
    void drawString(const String& text, int16_t x, int16_t y, uint32_t color, uint32_t bg) override {
        if (_realGfx && _cache) {
            // Use the real GFX driver to render text, then cache the pixels
            // We need to intercept the pixel drawing to cache them
            // For now, let's use a simple approach: render to a temporary area and copy pixels
            
            // Set up the GFX driver for text rendering
            _realGfx->setTextColor(color, bg);
            _realGfx->setCursor(x, y);
            
            // Calculate text bounds using real font dimensions
            int16_t x1, y1;
            uint16_t w, h;
            _realGfx->getTextBounds(text, 0, 0, &x1, &y1, &w, &h);
            int16_t textWidth = w;
            int16_t textHeight = h;
            
            // Render text pixel by pixel to the cache
            for (int16_t py = y; py < y + textHeight; py++) {
                for (int16_t px = x; px < x + textWidth; px++) {
                    // For now, just set background color for text area
                    // In a full implementation, we'd need to get the actual pixel color from GFX
                    _cache->setPixel(px, py, bg);
                }
            }
            
            // Mark the text area as dirty in the cache
            // The cache will handle this automatically when we call setPixel
        }
    }
    int16_t getWidth() const override { return _cache ? _cache->getWidth() : 0; }
    int16_t getHeight() const override { return _cache ? _cache->getHeight() : 0; }
    
    // Display control methods
    bool begin() override { 
        // The real hardware is already initialized before creating the cache
        return _cache != nullptr; 
    }
    void setRotation(uint8_t rotation) override { /* Cache doesn't need rotation */ }
    void fillScreen(uint32_t color) override { 
        if (_cache) {
            for (int16_t y = 0; y < _cache->getHeight(); y++) {
                for (int16_t x = 0; x < _cache->getWidth(); x++) {
                    _cache->setPixel(x, y, color);
                }
            }
        }
    }
    
    // Additional drawing methods needed by widgets
    void fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint32_t color) override {
        if (_cache) {
            for (int16_t py = y; py < y + h; py++) {
                for (int16_t px = x; px < x + w; px++) {
                    _cache->setPixel(px, py, color);
                }
            }
        }
    }
    
    void updateCachePixel(uint16_t x, uint16_t y, uint16_t color) override {
        if (_cache) {
            _cache->setPixel(x, y, color);
        }
    }
    
    void updateCache() override {
        if (_cache) {
            ESP_LOGD(CACHE_DISPLAY_TAG, "CacheDisplay::updateCache() - calling cache update");
            _cache->update(); // Update the display with cached data
        } else {
            ESP_LOGW(CACHE_DISPLAY_TAG, "CacheDisplay::updateCache() - no cache available");
        }
    }
    
    // Text methods needed by widgets
    void setTextSize(uint8_t size) override { 
        if (_realGfx) {
            _realGfx->setTextSize(size);
            _textSize = size; // Track the size
        }
    }
    int16_t textWidth(const String& text) override { 
        if (_realGfx) {
            // Use Arduino GFX's getTextBounds to get the actual text width
            int16_t x1, y1;
            uint16_t w, h;
            _realGfx->getTextBounds(text, 0, 0, &x1, &y1, &w, &h);
            return w;
        }
        // Fallback to estimate if no GFX object
        return text.length() * 6 * _textSize; 
    }
    int16_t fontHeight() override { 
        if (_realGfx) {
            // Use Arduino GFX's getTextBounds to get the actual font height
            int16_t x1, y1;
            uint16_t w, h;
            _realGfx->getTextBounds("A", 0, 0, &x1, &y1, &w, &h);
            return h;
        }
        // Fallback to estimate if no GFX object
        return 8 * _textSize; 
    }
}; 