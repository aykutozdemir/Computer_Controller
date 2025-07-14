#define SIMPLEUI_TAG "Spacer"
#include "SimpleUI/SimpleUIDebug.h"
#include "Spacer.h"

Spacer::Spacer(int16_t size, float weight)
    : Widget(0, 0, size, size), _weight(weight), _isFlexible(size == 0) {
    _type = WidgetType::SPACER;
    SUI_LOGI("ctor: size=%d weight=%.2f flexible=%d", size, weight, _isFlexible);
}

int16_t Spacer::getWidth() const {
    // Flexible spacers return 0 (they expand to fill available space)
    // Fixed spacers return their size
    return _isFlexible ? 0 : _w;
}

int16_t Spacer::getHeight() const {
    // Flexible spacers return 0 (they expand to fill available space)
    // Fixed spacers return their size
    return _isFlexible ? 0 : _h;
}

void Spacer::draw() {
    // Spacer doesn't draw anything - it's just empty space
} 