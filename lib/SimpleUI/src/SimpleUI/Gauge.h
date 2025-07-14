/**
 * @file Gauge.h
 * @brief Gauge widget for visualizing numeric values in the SimpleUI library.
 */

#pragma once

#include "Widget.h"
#include <math.h>

class Gauge : public Widget {
private:
    float _value{0.0f};
    uint16_t _needleColor{0xF800};
public:
    /// Construct a Gauge widget.
    Gauge(int16_t x, int16_t y, int16_t size);

    /// Set the gauge value to display (expected range 0.0–1.0 or domain-specific).
    void setValue(float v);

    /// Draw the gauge.
    void draw() override;
}; 