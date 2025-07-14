/**
 * @file Button.h
 * @brief Button widget class for the SimpleUI library.
 */

#pragma once

#include "Widget.h"

class Button : public Widget {
private:
    String _text;
    uint16_t _bgColor{0xFFFF};
    uint16_t _borderColor{0xFFFF};
    uint16_t _textColor{0x0000};
    uint8_t _textSize{1};
    bool _pressed{false};
    std::function<void()> _onClick;
public:
    /// Construct a Button widget.
    Button(int16_t x, int16_t y, int16_t w, int16_t h, const String &text, uint8_t textSize = 1);

    /// Set background, border, and text colours.
    void setColors(uint16_t bg, uint16_t border, uint16_t text);

    /// Set the text size in the button label.
    void setTextSize(uint8_t size) { _textSize = size; }

    /// Assign a callback to be invoked when the button is clicked.
    void setOnClick(std::function<void()> cb);

    /// Get button width based on text content.
    int16_t getWidth() const override;

    /// Get button height based on text size.
    int16_t getHeight() const override;

    /// Draw the button to the display.
    void draw() override;

    /// Handle touch-input events for the button.
    void handleTouch(int16_t x, int16_t y, bool pressed) override;
}; 