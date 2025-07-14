/**
 * @file HorizontalLine.h
 * @brief Horizontal line widget class for the SimpleUI library.
 */

#pragma once

#include "Widget.h"

class HorizontalLine : public Widget {
private:
    uint16_t _color;
public:
    /// Construct a horizontal line.
    HorizontalLine(int16_t x, int16_t y, int16_t w, uint16_t color = 0xFFFF, int16_t thickness = 1);

    /// Set line colour and mark for redraw.
    void setColor(uint16_t color) { _color = color; markDirty(); }
    /// Set line thickness in pixels.
    void setThickness(int16_t thickness) { _h = thickness; markDirty(); }

    /// Get the width - horizontal lines should expand to fill available space
    int16_t getWidth() const override { return _w > 0 ? _w : 100; }

    /// Draw the horizontal line.
    void draw() override;
}; 