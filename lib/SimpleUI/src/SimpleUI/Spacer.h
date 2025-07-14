/**
 * @file Spacer.h
 * @brief Flexible spacer widget used for layout gaps in the SimpleUI library.
 */

#pragma once

#include "Widget.h"

class Spacer : public Widget {
private:
    float _weight;
    bool _isFlexible;
    
public:
    /// Construct a Spacer widget.
    Spacer(int16_t size = 0, float weight = 1.0f);
    
    /// Set the layout weight of the spacer.
    void setWeight(float weight) { _weight = weight; }
    /// Get the current weight.
    float getWeight() const { return _weight; }
    
    /// Mark the spacer as flexible (expands) or fixed.
    void setFlexible(bool flexible) { _isFlexible = flexible; }
    /// Return whether the spacer is flexible.
    bool isFlexible() const { return _isFlexible; }
    
    /// Get the width of the spacer (0 for flexible, size for fixed).
    int16_t getWidth() const override;
    /// Get the height of the spacer (0 for flexible, size for fixed).
    int16_t getHeight() const override;
    
    /// Draw spacer (typically no visible output).
    void draw() override;
}; 