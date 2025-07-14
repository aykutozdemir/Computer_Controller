/**
 * @file Cell.h
 * @brief Cell wrapper class that provides gravity-based positioning for widgets in containers.
 */

#pragma once

#include "Widget.h"

// Gravity enumeration for positioning widgets within their allocated space
enum class Gravity {
    TOP_LEFT,      // Top-left corner
    TOP_CENTER,    // Top center
    TOP_RIGHT,     // Top-right corner
    CENTER_LEFT,   // Center-left
    CENTER,        // Center
    CENTER_RIGHT,  // Center-right
    BOTTOM_LEFT,   // Bottom-left corner
    BOTTOM_CENTER, // Bottom center
    BOTTOM_RIGHT,  // Bottom-right corner
    FILL           // Fill the entire allocated space
};

class Cell {
private:
    Widget* _widget;
    Gravity _gravity;
    float _weight;
    bool _isSpacer;
    
    // Padding within the cell
    int16_t _paddingLeft;
    int16_t _paddingTop;
    int16_t _paddingRight;
    int16_t _paddingBottom;

public:
    /// Construct a Cell with a widget and gravity setting.
    Cell(Widget* widget, Gravity gravity = Gravity::CENTER, float weight = 1.0f);
    
    /// Destructor (does not delete the widget).
    ~Cell() = default;
    
    /// Get the wrapped widget.
    Widget* getWidget() const { return _widget; }
    
    /// Get the gravity setting.
    Gravity getGravity() const { return _gravity; }
    
    /// Set the gravity setting.
    void setGravity(Gravity gravity) { _gravity = gravity; }
    
    /// Get the weight for layout calculations.
    float getWeight() const { return _weight; }
    
    /// Set the weight for layout calculations.
    void setWeight(float weight) { _weight = weight; }
    
    /// Check if this cell contains a spacer widget.
    bool isSpacer() const { return _isSpacer; }
    
    /// Set padding for the cell.
    void setPadding(int16_t left, int16_t top, int16_t right, int16_t bottom);
    
    /// Set uniform padding for all sides.
    void setPadding(int16_t padding);
    
    /// Get padding values.
    int16_t getPaddingLeft() const { return _paddingLeft; }
    int16_t getPaddingTop() const { return _paddingTop; }
    int16_t getPaddingRight() const { return _paddingRight; }
    int16_t getPaddingBottom() const { return _paddingBottom; }
    
    /// Position the widget within the given bounds according to gravity.
    void positionWidget(int16_t cellX, int16_t cellY, int16_t cellWidth, int16_t cellHeight);
    
    /// Draw the cell's widget.
    void draw();
    
    /// Handle touch events for the cell's widget.
    void handleTouch(int16_t x, int16_t y, bool pressed);
}; 