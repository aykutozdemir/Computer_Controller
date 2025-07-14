#define SIMPLEUI_TAG "HorizontalLine"
#include "SimpleUI/SimpleUIDebug.h"
#include "SimpleUI/HorizontalLine.h"

HorizontalLine::HorizontalLine(int16_t x, int16_t y, int16_t w, uint16_t color, int16_t thickness)
    : Widget(x, y, w > 0 ? w : 1, thickness > 0 ? thickness : 1), _color(color) {
    _type = WidgetType::HLINE;
}

void HorizontalLine::draw() {
    if (!_visible || !_display || !_dirty) return;

    // Only draw if width and height are positive
    if (_w > 0 && _h > 0) {
        _display->fillRect(_x, _y, _w, _h, _color);
    } else {
        SUI_LOGW("Skipping draw due to non-positive size (%d x %d)", _w, _h);
    }

    markClean();
} 