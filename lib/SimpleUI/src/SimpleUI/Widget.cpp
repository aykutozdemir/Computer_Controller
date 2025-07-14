#define SIMPLEUI_TAG "Widget"
#include "SimpleUI/SimpleUIDebug.h"
#include "SimpleUI/Widget.h"
#include "SimpleUI/SimpleUIApp.h"

Widget::Widget(int16_t x, int16_t y, int16_t w, int16_t h)
    : _x(x), _y(y), _w(w), _h(h), _visible(true), _display(nullptr) {
    SUI_LOGI("ctor: x=%d y=%d w=%d h=%d", x, y, w, h);
}

Widget::~Widget() = default;

void Widget::setVisible(bool v) {
    _visible = v;
    markDirty();
    SUI_LOGD("setVisible: %d", v);
}

bool Widget::isVisible() const { return _visible; }

bool Widget::contains(int16_t px, int16_t py) const {
    return (px >= _x) && (px < _x + _w) && (py >= _y) && (py < _y + _h);
}

void Widget::setPosition(int16_t x, int16_t y) {
    _x = x;
    _y = y;
    markDirty();
    SUI_LOGD("setPosition: x=%d y=%d", x, y);
}

void Widget::setSize(int16_t w, int16_t h) {
    _w = w;
    _h = h;
    markDirty();
    SUI_LOGD("setSize: w=%d h=%d", w, h);
}

int16_t Widget::measureTextWidth(const String& text, uint8_t textSize) const {
    if (!_display) return getFallbackTextWidth(text, textSize);
    
    _display->setTextSize(textSize);
    int16_t w = _display->textWidth(text);
    if (w <= 0) {
        w = getFallbackTextWidth(text, textSize);
    }
    return w;
}

int16_t Widget::measureTextHeight(uint8_t textSize) const {
    if (!_display) return getFallbackTextHeight(textSize);
    
    _display->setTextSize(textSize);
    int16_t h = _display->fontHeight();
    if (h <= 0) {
        h = getFallbackTextHeight(textSize);
    }
    return h;
}

int16_t Widget::getFallbackTextWidth(const String& text, uint8_t textSize) const {
    // Fallback: use constant for pixels per character times text size
    return text.length() * WidgetConstants::TEXT_CHAR_WIDTH * textSize;
}

int16_t Widget::getFallbackTextHeight(uint8_t textSize) const {
    // Fallback: use constant for baseline height times text size
    return WidgetConstants::TEXT_BASELINE_HEIGHT * textSize;
}

// Static member initialization
SimpleUIApp* Widget::_app = nullptr;

void Widget::setApp(SimpleUIApp* app) {
    _app = app;
}

void Widget::markDirty() {
    _dirty = true;
    // Notify the app that something changed
    if (_app) {
        _app->markDirty();
    }
}

void Widget::markContentDirty() {
    _dirty = true;
    // Notify the app that something needs redrawing (content change only)
    if (_app) {
        _app->markDirty();
    }
} 