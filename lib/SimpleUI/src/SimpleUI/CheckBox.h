/**
 * @file CheckBox.h
 * @brief Check box widget class for the SimpleUI library.
 */

#pragma once

#include "Widget.h"

class CheckBox : public Widget {
private:
    bool _checked{false};
    uint16_t _boxColor{0xFFFF};
    uint16_t _tickColor{0x0000};
    std::function<void(bool)> _onChanged;
public:
    /// Construct a CheckBox widget.
    CheckBox(int16_t x, int16_t y, int16_t size);

    /// Set the checked state.
    void setChecked(bool c);

    /// Return the current checked state.
    bool isChecked() const;

    /// Assign a callback invoked when the checked state changes.
    void setOnChanged(std::function<void(bool)> cb);

    /// Draw the checkbox.
    void draw() override;

    /// Handle touch events.
    void handleTouch(int16_t px, int16_t py, bool pressed) override;
}; 