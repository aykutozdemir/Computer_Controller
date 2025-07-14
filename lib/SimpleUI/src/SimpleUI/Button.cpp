#define SIMPLEUI_TAG "Button"
#include "SimpleUI/SimpleUIDebug.h"
#include "SimpleUI/Button.h"

Button::Button(int16_t x, int16_t y, int16_t w, int16_t h, const String &text, uint8_t textSize)
    : Widget(x, y, w, h), _text(text), _textSize(textSize) {
    _type = WidgetType::BUTTON;
    SUI_LOGI("ctor: (%d,%d) %dx%d text='%s'", x, y, w, h, _text.c_str());
}

void Button::setColors(uint16_t bg, uint16_t border, uint16_t text) {
    _bgColor = bg;
    _borderColor = border;
    _textColor = text;
    markDirty();
    SUI_LOGD("setColors: bg=0x%04X border=0x%04X text=0x%04X", bg, border, text);
}

void Button::setOnClick(std::function<void()> cb) {
    _onClick = std::move(cb);
}

int16_t Button::getWidth() const {
    // Get text width and add padding for button appearance
    int16_t textWidth = measureTextWidth(_text, _textSize);
    return textWidth + WidgetConstants::BUTTON_PADDING_HORIZONTAL;
}

int16_t Button::getHeight() const {
    // Get text height and add padding for button appearance
    int16_t textHeight = measureTextHeight(_textSize);
    int16_t result = textHeight + WidgetConstants::BUTTON_PADDING_VERTICAL;
    SUI_LOGD("getHeight: text='%s', textHeight=%d, textSize=%d, result=%d", _text.c_str(), textHeight, _textSize, result);
    return result;
}

void Button::draw() {
    if (!_visible || !_display || !_dirty) return;
    SUI_LOGD("draw: (%d,%d) %dx%d", _x, _y, _w, _h);
    // ---------------------------------------------------------------------
    // Background & border
    // ---------------------------------------------------------------------
    // Fill full button rectangle with background colour
    _display->fillRect(_x, _y, _w, _h, _bgColor);

    // 1-pixel border around the button
    _display->fillRect(_x, _y, _w, 1, _borderColor);                       // Top
    _display->fillRect(_x, _y + _h - 1, _w, 1, _borderColor);              // Bottom
    _display->fillRect(_x, _y, 1, _h, _borderColor);                       // Left
    _display->fillRect(_x + _w - 1, _y, 1, _h, _borderColor);              // Right

    // ---------------------------------------------------------------------
    // Center text inside the button
    // ---------------------------------------------------------------------
    _display->setTextSize(_textSize);
    int16_t textWidth  = _display->textWidth(_text);
    int16_t textHeight = _display->fontHeight();

    int16_t tx = _x + (_w - textWidth) / 2;
    int16_t ty = _y + (_h - textHeight) / 2;

    _display->drawString(_text, tx, ty, _textColor, _bgColor);
    // Clear dirty flag
    markClean();
}

void Button::handleTouch(int16_t px, int16_t py, bool pressed) {
    if (!_visible) return;
    bool inside = contains(px, py);
    if (pressed && inside && !_pressed) {
        _pressed = true;
    } else if (!pressed && _pressed) {
        _pressed = false;
        if (inside && _onClick) _onClick();
    }
} 