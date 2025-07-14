#define SIMPLEUI_TAG "Label"
#include "SimpleUI/SimpleUIDebug.h"
#include "SimpleUI/Container.h"
#include "SimpleUI/Label.h"

Label::Label(int16_t x, int16_t y, const String &text, uint16_t color, uint8_t textSize)
    : Widget(x, y, 0, 0), _text(text), _color(color), _textSize(textSize) {
    _type = WidgetType::LABEL;
    SUI_LOGI("ctor: (%d,%d) text='%s'", x, y, _text.c_str());
}

void Label::setText(const String &text) {
    // Measure current size before changing text
    int16_t oldW = getWidth();
    int16_t oldH = getHeight();

    _text = text;
    
    // Measure new size after update
    int16_t newW = getWidth();
    int16_t newH = getHeight();

    // If dimensions changed, mark layout as dirty to trigger recalculation
    if ((newW != oldW || newH != oldH)) {
        markLayoutDirty();
        markDirty(); // Full redraw needed for size change
    } else {
        // Only content changed, no layout recalculation needed
        markContentDirty();
    }

    SUI_LOGD("setText: '%s' size_changed=%s", text.c_str(), (newW != oldW || newH != oldH) ? "true" : "false");
}

const String &Label::getText() const { return _text; }

int16_t Label::getWidth() const {
    return measureTextWidth(_text, _textSize);
}

int16_t Label::getHeight() const {
    return measureTextHeight(_textSize);
}

void Label::draw() {
    if (!_visible || !_display || !_dirty) return;
    SUI_LOGD("draw: '%s' pos=(%d,%d)", _text.c_str(), _x, _y);
    // Clear previously drawn area if any using background colour
    if (_prevWidth > 0 && _prevHeight > 0) {
        _display->fillRect(_prevX, _prevY, _prevWidth, _prevHeight, _bgColor);
    }
    // Draw the label text    _
    _display->setTextSize(_textSize);
    _display->drawString(_text, _x, _y, _color, _bgColor);
    // Store current state for future diffing
    _prevText = _text;
    _prevColor = _color;
    _prevSize = _textSize;
    _prevWidth = getWidth();
    _prevHeight = getHeight();
    _prevX = _x;
    _prevY = _y;
    // Clear dirty flag
    markClean();
} 