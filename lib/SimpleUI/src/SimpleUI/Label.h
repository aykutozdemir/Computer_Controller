/**
 * @file Label.h
 * @brief Label widget class for the SimpleUI library.
 */

#pragma once

#include "Widget.h"

class Label : public Widget {
private:
    String _text;
    uint16_t _color;
    uint8_t _textSize{1};
    int16_t _prevWidth{0};
    int16_t _prevHeight{0};
    String _prevText{String("")};
    uint16_t _prevColor{0};
    uint8_t _prevSize{1};
    int16_t _prevX{0};
    int16_t _prevY{0};
    // Background colour used when clearing previously drawn text. Defaults to the global UI background.
    uint16_t _bgColor{0x0000}; // Default to black to avoid external dependencies
public:
    /// Construct a Label widget.
    Label(int16_t x, int16_t y, const String &text, uint16_t color = 0xFFFF, uint8_t textSize = 1);

    /// Update the label text and mark for redraw.
    void setText(const String &text);

    /// Set the font size of the label text.
    void setTextSize(uint8_t size) { _textSize = size; markDirty(); }

    /// Set the colour of the label text.
    void setTextColor(uint16_t color) { _color = color; markDirty(); }

    /// Set the background colour that will be painted behind the text when it is redrawn.
    void setBackgroundColor(uint16_t color) { _bgColor = color; markDirty(); }

    /// Get the current text of the label.
    const String &getText() const;
    
    /// Return the calculated text width.
    int16_t getWidth() const override;

    /// Return the calculated text height.
    int16_t getHeight() const override;

    /// Draw the label.
    void draw() override;
}; 