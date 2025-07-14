/**
 * @file Slider.h
 * @brief Slider widget allowing value selection in the SimpleUI library.
 */

#pragma once

#include "Widget.h"

class Slider : public Widget {
private:
    float _value{0.0f}; // 0-1
    uint16_t _trackColor{0x7BEF};
    uint16_t _knobColor{0xFFFF};
    std::function<void(float)> _onChanged;
public:
    /// Construct a Slider widget.
    Slider(int16_t x, int16_t y, int16_t w, int16_t h);

    /// Set slider value in the range [0.0, 1.0].
    void setValue(float v);

    /// Get current slider value.
    float getValue() const;

    /// Assign a callback invoked when the slider value changes.
    void setOnChanged(std::function<void(float)> cb);

    /// Draw the slider.
    void draw() override;

    /// Handle touch interactions for the slider.
    void handleTouch(int16_t x, int16_t y, bool pressed) override;
}; 