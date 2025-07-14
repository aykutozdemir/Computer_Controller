#define SIMPLEUI_TAG "CheckBox"
#include "SimpleUI/SimpleUIDebug.h"
#include "SimpleUI/CheckBox.h"

CheckBox::CheckBox(int16_t x, int16_t y, int16_t size)
    : Widget(x, y, size, size) {
    SUI_LOGI("ctor: (%d,%d) size=%d", x, y, size);
}

void CheckBox::setChecked(bool c) { if (c != _checked) { _checked = c; markDirty(); SUI_LOGD("setChecked: %d", _checked); } }

bool CheckBox::isChecked() const { return _checked; }

void CheckBox::setOnChanged(std::function<void(bool)> cb) { _onChanged = std::move(cb); }

void CheckBox::draw() {
    if (!_visible || !_display || !_dirty) return;
    SUI_LOGD("draw: checked=%d", _checked);
    // Box background
    _display->fillRect(_x, _y, _w, _h, _boxColor);
    // Border
    _display->fillRect(_x, _y, _w, 1, _tickColor);
    _display->fillRect(_x, _y + _h - 1, _w, 1, _tickColor);
    _display->fillRect(_x, _y, 1, _h, _tickColor);
    _display->fillRect(_x + _w - 1, _y, 1, _h, _tickColor);

    // Tick if checked
    if (_checked) {
        int16_t inset = _w / 4;
        _display->fillRect(_x + inset, _y + inset, _w - 2 * inset, _h - 2 * inset, _tickColor);
    }

    // Clear dirty flag
    markClean();
}

void CheckBox::handleTouch(int16_t px, int16_t py, bool pressed) {
    if (!_visible || !pressed) return;
    if (contains(px, py)) {
        _checked = !_checked;
        markDirty();
        SUI_LOGD("handleTouch: toggled to %d", _checked);
        if (_onChanged) _onChanged(_checked);
    }
} 