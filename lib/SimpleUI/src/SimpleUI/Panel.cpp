#define SIMPLEUI_TAG "Panel"
#include "SimpleUI/SimpleUIDebug.h"
#include "SimpleUI/Panel.h"

Panel::Panel(int16_t x, int16_t y, int16_t w, int16_t h)
    : Container(x, y, w, h) {
    _type = WidgetType::PANEL;
    SUI_LOGI("ctor: (%d,%d) %dx%d", x, y, w, h);
}

void Panel::setPosition(int16_t x, int16_t y) {
    SUI_LOGD("setPosition: (%d,%d) -> (%d,%d)", _x, _y, x, y);
    Widget::setPosition(x, y);
}

void Panel::setSize(int16_t w, int16_t h) {
    SUI_LOGD("setSize: %dx%d -> %dx%d", _w, _h, w, h);
    Widget::setSize(w, h);
}

void Panel::setColors(uint16_t bg, uint16_t border) {
    _bgColor = bg;
    _borderColor = border;
    _bgDrawn = false; // force redraw
    markDirty();
    SUI_LOGD("setColors: bg=0x%04X border=0x%04X", bg, border);
}

void Panel::setBorderThickness(int16_t thickness) {
    _borderThickness = thickness;
    markDirty();
    SUI_LOGD("setBorderThickness: %d", thickness);
}

void Panel::draw() {
    if (!_visible || !_display) return;

    SUI_LOGD("draw: dirty=%d pos=(%d,%d) size=%dx%d", _dirty, _x, _y, _w, _h);
    // Draw background only when the panel is dirty or background hasn't been drawn yet
    if (_dirty || !_bgDrawn) {
        // Repaint full background once when dirty.
        _display->fillRect(_x, _y, _w, _h, _bgColor);
        _bgDrawn = true;
        markClean();
    }

    // Always draw the border (cheap and prevents it being overwritten by other widgets)
    const int bt = _borderThickness; // use configurable border thickness
    // Top and bottom borders
    _display->fillRect(_x,          _y,         _w, bt, _borderColor);            // Top
    _display->fillRect(_x,          _y + _h - bt, _w, bt, _borderColor);          // Bottom
    // Left and right borders
    _display->fillRect(_x,          _y,         bt,  _h, _borderColor);            // Left
    _display->fillRect(_x + _w - bt, _y,       bt,  _h, _borderColor);            // Right

    // Always draw children so dirty widgets inside get updated
    Container::draw();
}

// ---------------------------------------------------------------------------
// Layout propagation
// ---------------------------------------------------------------------------

void Panel::recalculateLayout() {
    // Prevent infinite recursion
    if (_inLayoutCalculation) {
        SUI_LOGD("recalculateLayout: already in progress, skipping");
        return;
    }
    
    _inLayoutCalculation = true;
    _layoutCalculationDepth++;
    
    SUI_LOGD("recalculateLayout: starting at depth %d", _layoutCalculationDepth);
    SUI_LOGD("Panel size: %dx%d, border thickness: %d", _w, _h, _borderThickness);
    
    // Panels need to position their children relative to their own position
    // Calculate the inner area (excluding border thickness)
    const int16_t innerWidth = _w - (2 * _borderThickness);
    const int16_t innerHeight = _h - (2 * _borderThickness);
    
    SUI_LOGD("Panel inner area: %dx%d", innerWidth, innerHeight);
    
    // For panels with a single child (like our status layout), give it the full inner area
    if (_cells.size() == 1) {
        Cell* cell = _cells[0];
        if (cell && cell->getWidget()) {
            // Calculate the absolute position for the child (panel position + border thickness)
            const int16_t childX = _x + _borderThickness;
            const int16_t childY = _y + _borderThickness;
            
            SUI_LOGD("Positioning single child at (%d,%d) with size %dx%d", childX, childY, innerWidth, innerHeight);
            // Position the child at the panel's inner area position
            cell->positionWidget(childX, childY, innerWidth, innerHeight);
            
            // If the child is a Container, trigger its layout recalculation
            Widget* w = cell->getWidget();
            const WidgetType t = w->getType();
            if (t == WidgetType::VERTICAL_LAYOUT || t == WidgetType::HORIZONTAL_LAYOUT || t == WidgetType::PANEL) {
                Container* childContainer = static_cast<Container*>(w);
                if (!childContainer->isInLayoutCalculation()) {
                    childContainer->recalculateLayout();
                }
            }
        }
    } else {
        // For multiple children, use the existing logic
        for (auto *cell : _cells) {
            if (!cell) continue;
            Widget* w = cell->getWidget();
            if (!w) continue;

            // Calculate the absolute position for the child (panel position + border thickness)
            const int16_t childX = _x + _borderThickness;
            const int16_t childY = _y + _borderThickness;

            // Position the child at the panel's inner area position
            cell->positionWidget(childX, childY, innerWidth, innerHeight);

            // If the child is also a Container-derived widget, forward the recalculateLayout call
            const WidgetType t = w->getType();
            if (t == WidgetType::VERTICAL_LAYOUT || t == WidgetType::HORIZONTAL_LAYOUT || t == WidgetType::PANEL) {
                Container* childContainer = static_cast<Container*>(w);
                // Check if the child container is already in layout calculation to prevent infinite recursion
                if (!childContainer->isInLayoutCalculation()) {
                    childContainer->recalculateLayout();
                }
            }
        }
    }
    
    SUI_LOGD("recalculateLayout: completed at depth %d", _layoutCalculationDepth);
    _layoutCalculationDepth--;
    _inLayoutCalculation = false;
} 