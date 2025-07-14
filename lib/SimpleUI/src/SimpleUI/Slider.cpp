#define SIMPLEUI_TAG "Slider"
#include "SimpleUI/SimpleUIDebug.h"
#include "SimpleUI/Slider.h"

Slider::Slider(int16_t x, int16_t y, int16_t w, int16_t h)
    : Widget(x, y, w, h) {
    SUI_LOGI("ctor: (%d,%d) %dx%d", x, y, w, h);
}

void Slider::setValue(float v) {
    if (v < 0.0f) v = 0.0f;
    if (v > 1.0f) v = 1.0f;
    if (v != _value) {
        _value = v;
        markDirty();
        SUI_LOGD("setValue: %.2f", _value);
    }
}

float Slider::getValue() const { return _value; }

void Slider::setOnChanged(std::function<void(float)> cb) { _onChanged = std::move(cb); }

void Slider::draw() {
    if (!_visible || !_display || !_dirty) return;
    SUI_LOGD("draw: value=%.2f", _value);
    // Track
    _display->fillRect(_x, _y + _h / 2 - 2, _w, 4, _trackColor);
    // Knob - square with side equal to _h
    int16_t kx = _x + static_cast<int16_t>(_value * (_w - _h));
    _display->fillRect(kx, _y, _h, _h, _knobColor);

    markClean();
}

void Slider::handleTouch(int16_t px, int16_t py, bool pressed) {
    if (!_visible || !pressed) return;
    if (contains(px, py)) {
        float rel = (float)(px - _x) / (float)(_w);
        setValue(rel);
        SUI_LOGD("handleTouch: rel=%.2f", rel);
        if (_onChanged) _onChanged(_value);
    }
} 