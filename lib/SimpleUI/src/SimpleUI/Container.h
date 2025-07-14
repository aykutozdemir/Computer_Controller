/**
 * @file Container.h
 * @brief Base container widget class that can hold multiple child widgets in the SimpleUI library.
 */

#pragma once

#include "Widget.h"
#include "Cell.h"
#include <vector>
#include <functional>

// Forward declarations
class Label;
class Button;
class ProgressBar;
class VerticalLayout;
class HorizontalLayout;
class Spacer;
class CheckBox;
class Slider;
class Gauge;
class HorizontalLine;

class Container : public Widget {
protected:
    std::vector<Cell *> _cells;
    int16_t _margin;
    int16_t _spacing;
    bool _equalSpacing;
    bool _inLayoutCalculation{false};  // Recursion guard
    int _layoutCalculationDepth{0};    // Track recursion depth for debugging
    
public:
    /// Construct a Container widget.
    Container(int16_t x, int16_t y, int16_t w, int16_t h);
    /// Destructor; frees child widgets.
    virtual ~Container();

    /// Add a child widget with an optional layout weight and gravity.
    virtual void addChild(Widget *w, float weight = 1.0f, Gravity gravity = Gravity::CENTER);
    /// Add a cell with a widget and gravity setting.
    virtual void addCell(Cell *cell);
    /// Retrieve read-only vector of cells.
    const std::vector<Cell *> &getCells() const { return _cells; }

    /// Draw the container and all children.
    void draw() override;
    /// Dispatch touch event to the appropriate child.
    void handleTouch(int16_t x, int16_t y, bool pressed) override;

    // -----------------------------------------------------------------
    // Layout recalculation hook – overridden by layout containers
    // -----------------------------------------------------------------
    virtual void recalculateLayout() = 0;
    
    /// Check if layout needs recalculation and perform it if necessary
    virtual void updateLayoutIfNeeded();

    /// Check if layout calculation is currently in progress (recursion guard).
    bool isInLayoutCalculation() const { return _inLayoutCalculation; }

    /// Get the current layout calculation depth (for debugging).
    int getLayoutCalculationDepth() const { return _layoutCalculationDepth; }

    // Layout management methods
    /// Set spacing between child widgets.
    virtual void setSpacing(int16_t spacing) { 
        _spacing = spacing; 
        markLayoutDirty();
    }
    /// Set margin around the layout.
    virtual void setMargin(int16_t margin) { 
        _margin = margin; 
        markLayoutDirty();
    }
    /// Toggle equal spacing mode.
    virtual void setEqualSpacing(bool equal) { 
        _equalSpacing = equal; 
        markLayoutDirty();
    }
    /// Change weight of an existing cell by index.
    virtual void setCellWeight(int index, float weight);
    /// Change gravity of an existing cell by index.
    virtual void setCellGravity(int index, Gravity gravity);

    // Convenience methods that return Cell objects with widgets
    /// Add a Label and return Cell containing it.
    virtual Cell* addLabel(const String& text, uint16_t color = 0xFFFF, uint8_t textSize = 1, 
                          float weight = 1.0f, Gravity gravity = Gravity::CENTER);
    /// Add a Button and return Cell containing it.
    virtual Cell* addButton(const String& text, std::function<void()> onClick, 
                           uint16_t bgColor = 0x07E0, uint16_t borderColor = 0x0000, uint16_t textColor = 0xFFFF, 
                           float weight = 1.0f, Gravity gravity = Gravity::CENTER, uint8_t textSize = 1);
    /// Add a ProgressBar and return Cell containing it.
    virtual Cell* addProgressBar(float progress = 0.0f, float weight = 1.0f, Gravity gravity = Gravity::FILL);
    /// Add a CheckBox and return Cell containing it.
    virtual Cell* addCheckBox(int16_t size = 20, float weight = 1.0f, Gravity gravity = Gravity::CENTER);
    /// Add a Slider and return Cell containing it.
    virtual Cell* addSlider(int16_t height = 20, float weight = 1.0f, Gravity gravity = Gravity::FILL);
    /// Add a Gauge and return Cell containing it.
    virtual Cell* addGauge(int16_t size = 50, float weight = 1.0f, Gravity gravity = Gravity::CENTER);
    /// Add a HorizontalLine and return Cell containing it.
    virtual Cell* addHorizontalLine(uint16_t color = 0xFFFF, int16_t thickness = 1, float weight = 0.0f);
    /// Add a nested vertical layout and return Cell containing it.
    virtual Cell* addVerticalLayout(float weight = 1.0f, Gravity gravity = Gravity::FILL);
    /// Add a nested horizontal layout and return Cell containing it.
    virtual Cell* addHorizontalLayout(float weight = 1.0f, Gravity gravity = Gravity::FILL);
    
    /// Add a flexible spacer with given weight and return Cell containing it.
    virtual Cell* addSpacer(float weight = 1.0f);
    /// Add a fixed-size spacer and return Cell containing it.
    virtual Cell* addFixedSpacer(int16_t size);
    
    /// Static helper to get container from a widget's parent.
    static Container* getContainerFromWidget(const Widget* widget);
}; 