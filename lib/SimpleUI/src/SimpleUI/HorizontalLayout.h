/**
 * @file HorizontalLayout.h
 * @brief Horizontal layout container widget for the SimpleUI library.
 */

#pragma once

#include "Container.h"
#include <vector>

// Forward declaration
class Spacer;
class VerticalLayout;
class HorizontalLayout;

class HorizontalLayout : public Container {
private:
    int16_t calculateAvailableSpace();
    void distributeSpace();
    bool _inLayoutCalculation = false;  // Recursion guard
    
public:
    void recalculateLayout() override;
    /// Construct a HorizontalLayout container.
    HorizontalLayout(int16_t x, int16_t y, int16_t w, int16_t h, int16_t margin = 10, int16_t spacing = 5);
    
    /// Add a child widget with a weight and gravity.
    void addChild(Widget *w, float weight = 1.0f, Gravity gravity = Gravity::CENTER) override;
    
    /// Get the height required by this layout based on its children.
    int16_t getHeight() const override;
    
    /// Draw the layout and its children.
    void draw() override;
}; 