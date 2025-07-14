#define SIMPLEUI_TAG "Cell"
#include "SimpleUI/SimpleUIDebug.h"
#include "SimpleUI/Cell.h"
#include "SimpleUI/Container.h"

Cell::Cell(Widget* widget, Gravity gravity, float weight)
    : _widget(widget), _gravity(gravity), _weight(weight), 
      _paddingLeft(0), _paddingTop(0), _paddingRight(0), _paddingBottom(0) {
    if (widget) {
        _isSpacer = widget->isType(WidgetType::SPACER);
    }
}

void Cell::setPadding(int16_t left, int16_t top, int16_t right, int16_t bottom) {
    _paddingLeft = left;
    _paddingTop = top;
    _paddingRight = right;
    _paddingBottom = bottom;
}

void Cell::setPadding(int16_t padding) {
    _paddingLeft = _paddingTop = _paddingRight = _paddingBottom = padding;
}

void Cell::positionWidget(int16_t cellX, int16_t cellY, int16_t cellWidth, int16_t cellHeight) {
    if (!_widget) return;
    
    // Calculate available space after padding
    int16_t availableWidth = cellWidth - _paddingLeft - _paddingRight;
    int16_t availableHeight = cellHeight - _paddingTop - _paddingBottom;
    
    if (availableWidth <= 0 || availableHeight <= 0) return;
    
    int16_t widgetX, widgetY, widgetWidth, widgetHeight;
    
    // Get widget's preferred size
    int16_t preferredWidth = _widget->getWidth();
    int16_t preferredHeight = _widget->getHeight();
    
    switch (_gravity) {
        case Gravity::FILL:
            // Widget fills the entire available space
            widgetWidth = availableWidth;
            widgetHeight = availableHeight;
            widgetX = cellX + _paddingLeft;
            widgetY = cellY + _paddingTop;
            break;
            
        case Gravity::CENTER:
            // Widget is centered in the available space
            widgetWidth = preferredWidth;
            widgetHeight = preferredHeight;
            widgetX = cellX + _paddingLeft + (availableWidth - preferredWidth) / 2;
            widgetY = cellY + _paddingTop + (availableHeight - preferredHeight) / 2;
            break;
            
        case Gravity::TOP_LEFT:
            widgetWidth = preferredWidth;
            widgetHeight = preferredHeight;
            widgetX = cellX + _paddingLeft;
            widgetY = cellY + _paddingTop;
            break;
            
        case Gravity::TOP_CENTER:
            widgetWidth = preferredWidth;
            widgetHeight = preferredHeight;
            widgetX = cellX + _paddingLeft + (availableWidth - preferredWidth) / 2;
            widgetY = cellY + _paddingTop;
            break;
            
        case Gravity::TOP_RIGHT:
            widgetWidth = preferredWidth;
            widgetHeight = preferredHeight;
            widgetX = cellX + cellWidth - _paddingRight - preferredWidth;
            widgetY = cellY + _paddingTop;
            break;
            
        case Gravity::CENTER_LEFT:
            widgetWidth = preferredWidth;
            widgetHeight = preferredHeight;
            widgetX = cellX + _paddingLeft;
            widgetY = cellY + _paddingTop + (availableHeight - preferredHeight) / 2;
            break;
            
        case Gravity::CENTER_RIGHT:
            widgetWidth = preferredWidth;
            widgetHeight = preferredHeight;
            widgetX = cellX + cellWidth - _paddingRight - preferredWidth;
            widgetY = cellY + _paddingTop + (availableHeight - preferredHeight) / 2;
            break;
            
        case Gravity::BOTTOM_LEFT:
            widgetWidth = preferredWidth;
            widgetHeight = preferredHeight;
            widgetX = cellX + _paddingLeft;
            widgetY = cellY + cellHeight - _paddingBottom - preferredHeight;
            break;
            
        case Gravity::BOTTOM_CENTER:
            widgetWidth = preferredWidth;
            widgetHeight = preferredHeight;
            widgetX = cellX + _paddingLeft + (availableWidth - preferredWidth) / 2;
            widgetY = cellY + cellHeight - _paddingBottom - preferredHeight;
            break;
            
        case Gravity::BOTTOM_RIGHT:
            widgetWidth = preferredWidth;
            widgetHeight = preferredHeight;
            widgetX = cellX + cellWidth - _paddingRight - preferredWidth;
            widgetY = cellY + cellHeight - _paddingBottom - preferredHeight;
            break;
    }
    
    // --------------------------------------------------------------
    // Apply calculated geometry to the widget
    // --------------------------------------------------------------
    SUI_LOGD("About to call setPosition(%d, %d) on widget type %d", widgetX, widgetY, static_cast<int>(_widget->getType()));
    _widget->setPosition(widgetX, widgetY);

    // Always propagate the computed size to the widget so that
    // subsequent layout passes (and the widget's own draw routine)
    // have accurate width/height information. Previously this was
    // only done for Gravity::FILL which caused widgets such as
    // Button or Slider (created with width/height = 0) to end up
    // drawing at size 0 in other gravity modes.

    SUI_LOGD("About to call setSize(%d, %d) on widget type %d", widgetWidth, widgetHeight, static_cast<int>(_widget->getType()));
    _widget->setSize(widgetWidth, widgetHeight);

    // If the contained widget is itself a layout/container, mark its layout as dirty
    // so it will be recalculated when needed. This prevents infinite recursion.
    {
        WidgetType t = _widget->getType();
        if (t == WidgetType::HORIZONTAL_LAYOUT || t == WidgetType::VERTICAL_LAYOUT || t == WidgetType::PANEL) {
            // Use the public forceLayoutUpdate() method to mark layout as dirty
            // This prevents infinite recursion while ensuring layout is updated when needed
            _widget->forceLayoutUpdate();
        }
    }

    SUI_LOGI("Pos (%d,%d) size (%d,%d) -> widget pos (%d,%d) size (%d,%d) g=%d", cellX, cellY, cellWidth, cellHeight, widgetX, widgetY, widgetWidth, widgetHeight, static_cast<int>(_gravity));
}

void Cell::draw() {
    if (_widget && _widget->isVisible()) {
        _widget->draw();
    }
}

void Cell::handleTouch(int16_t x, int16_t y, bool pressed) {
    if (_widget) {
        _widget->handleTouch(x, y, pressed);
    }
} 