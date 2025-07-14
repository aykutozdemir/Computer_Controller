#define SIMPLEUI_TAG "Gauge"
#include "SimpleUI/SimpleUIDebug.h"
#include "SimpleUI/Gauge.h"

Gauge::Gauge(int16_t x, int16_t y, int16_t size)
    : Widget(x, y, size, size) {
    SUI_LOGI("ctor: (%d,%d) size=%d", x, y, size);
}

void Gauge::setValue(float v) {
    if (v < 0.0f) v = 0.0f;
    if (v > 1.0f) v = 1.0f;
    if (v != _value) {
        _value = v;
        markDirty();
        SUI_LOGD("setValue: %.2f", _value);
    }
}

void Gauge::draw() {
    if (!_visible || !_display || !_dirty) return;
    SUI_LOGD("draw: value=%.2f", _value);

    float angle = (-135.0f + 270.0f * _value) * DEG_TO_RAD;
    int16_t cx = _x + _w / 2;
    int16_t cy = _y + _h / 2;
    int16_t r = _w / 2;
    int16_t nx = cx + static_cast<int16_t>(cosf(angle) * (r - 2));
    int16_t ny = cy + static_cast<int16_t>(sinf(angle) * (r - 2));

    // Clear area
    _display->fillRect(_x, _y, _w, _h, 0x0000);

    // Draw needle as series of pixels
    int steps = r;
    for (int i = 0; i < steps; ++i) {
        float t = (float)i / steps;
        int16_t px = cx + static_cast<int16_t>((nx - cx) * t);
        int16_t py = cy + static_cast<int16_t>((ny - cy) * t);
        _display->updateCachePixel(px, py, _needleColor);
    }
    markClean();
} 