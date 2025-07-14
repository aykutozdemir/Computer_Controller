/**
 * @file Panel.h
 * @brief Panel container widget with optional border for the SimpleUI library.
 */

#pragma once

#include "Container.h"

class Panel : public Container {
private:
    uint16_t _bgColor{0x0000};
    uint16_t _borderColor{0xFFFF};
    int16_t _borderThickness{1};
    bool _bgDrawn{false};
    bool _inLayoutCalculation{false};  // Recursion guard
    
public:
    /// Construct a Panel container.
    Panel(int16_t x, int16_t y, int16_t w, int16_t h);

    /// Override setPosition to add debug logging
    void setPosition(int16_t x, int16_t y);
    
    /// Override setSize to add debug logging
    void setSize(int16_t w, int16_t h);

    /// Set background and border colours.
    void setColors(uint16_t bg, uint16_t border);

    /// Set border thickness.
    void setBorderThickness(int16_t thickness);

    /// Draw the panel and its children.
    void draw() override;

    /// Panels don't manage layout actively, but it forwards layout updates to any child container.
    void recalculateLayout() override final;
}; 