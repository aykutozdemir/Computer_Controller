#define SIMPLEUI_TAG "ProgressBar"
#include "SimpleUI/SimpleUIDebug.h"
#include "SimpleUI/ProgressBar.h"

ProgressBar::ProgressBar(int16_t x, int16_t y, int16_t w, int16_t h)
    : Widget(x, y, w, h) {
    _type = WidgetType::PROGRESS_BAR;
    SUI_LOGI("ctor: (%d,%d) %dx%d", x, y, w, h);
}

void ProgressBar::setProgress(float p) {
    if (p < 0.0f) p = 0.0f;
    if (p > 1.0f) p = 1.0f;
    if (p != _progress) {
        _progress = p;
        markDirty();
        SUI_LOGD("setProgress: %.2f", _progress);
    }
}

void ProgressBar::draw() {
    if (!_visible || !_display || !_dirty) return;
    SUI_LOGD("draw: progress=%.2f", _progress);

    // Background if first draw
    if (_prevProgress < 0.0f) {
        _display->fillRect(_x, _y, _w, _h, _bgColor);
    }

    // Compute widths
    int16_t prevWidth = static_cast<int16_t>(_prevProgress * _w);
    int16_t newWidth = static_cast<int16_t>(_progress * _w);

    // If progress decreased, clear the area that is now empty
    if (newWidth < prevWidth) {
        // Clear extra area to background color
        _display->fillRect(_x + newWidth, _y, prevWidth - newWidth, _h, _bgColor);
    }

    // Draw new filled area (only the delta when increasing)
    if (newWidth > 0) {
        _display->fillRect(_x, _y, newWidth, _h, _fillColor);
    }

    // Draw the background for area beyond newWidth if prevProgress was -1 (first time)
    if (_prevProgress < 0.0f) {
        _display->fillRect(_x + newWidth, _y, _w - newWidth, _h, _bgColor);
    }

    _prevProgress = _progress;

    // Clear dirty flag
    markClean();
} 