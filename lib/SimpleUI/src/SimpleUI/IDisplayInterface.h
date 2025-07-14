/**
 * @file IDisplayInterface.h
 * @brief Abstract display interface used by widgets to draw themselves for the SimpleUI library.
 */

#pragma once

#include <Arduino.h>
#include <stdint.h>

class IDisplayInterface {
public:
    virtual ~IDisplayInterface() = default;

    // Pixel / rectangle primitives ----------------------------------------
    /// Update the pixel in the internal display cache.
    virtual void updateCachePixel(uint16_t x, uint16_t y, uint16_t color) = 0;
    /// Mark a rectangle region as dirty in the cache.
    virtual void updateCacheRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h) = 0;
    /// Flush the internal cache to the physical display.
    virtual void updateCache() = 0;

    // Colour helpers -------------------------------------------------------
    /// Convert 24-bit RGB to 16-bit RGB565.
    virtual uint16_t color565(uint8_t r, uint8_t g, uint8_t b) = 0;

    // Text helpers ---------------------------------------------------------
    /// Set current text colour (and optional background).
    virtual void setTextColor(uint16_t color, uint16_t bgColor = 0x0000) = 0;
    /// Set the text cursor position.
    virtual void setCursor(int16_t x, int16_t y) = 0;
    /// Set the font size multiplier.
    virtual void setTextSize(uint8_t size) = 0;
    /// Draw a UTF-8 string at the current cursor position.
    virtual void drawString(const String &text, int16_t x, int16_t y) = 0;
    /// Measure the pixel width of a string.
    virtual int16_t textWidth(const String &text) = 0;
    /// Get the current font height in pixels.
    virtual int16_t fontHeight() = 0;

    // Convenience helper: fill a rectangle using pixel plotting ------------
    /// Convenience helper: fill a rectangle with a solid colour via pixel plotting.
    void fillRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color) {
        for (uint16_t iy = 0; iy < h; ++iy) {
            for (uint16_t ix = 0; ix < w; ++ix) {
                updateCachePixel(x + ix, y + iy, color);
            }
        }
    }
}; 