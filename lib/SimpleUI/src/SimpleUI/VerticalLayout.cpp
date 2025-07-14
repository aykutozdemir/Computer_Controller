#define SIMPLEUI_TAG "VerticalLayout"
#include "SimpleUI/SimpleUIDebug.h"
#include "VerticalLayout.h"
#include "Label.h"
#include "Button.h"
#include "ProgressBar.h"
#include "Spacer.h"
#include "HorizontalLayout.h"
#include <algorithm>

VerticalLayout::VerticalLayout(int16_t x, int16_t y, int16_t w, int16_t h, int16_t margin, int16_t spacing)
    : Container(x, y, w, h) {
    _type = WidgetType::VERTICAL_LAYOUT;
    _margin = margin;
    _spacing = spacing;
}

void VerticalLayout::addChild(Widget *w, float weight, Gravity gravity) {
    if (!w) return;
    
    // Add to children list using base class method
    Container::addChild(w, weight, gravity);
    
    // Layout will be recalculated when needed via the dirty flag system
    // No need to call recalculateLayout() here anymore
}

int16_t VerticalLayout::calculateAvailableSpace() {
    int16_t totalFixedSize = 0;
    int16_t totalSpacing = (_cells.size() > 1) ? (_cells.size() - 1) * _spacing : 0;
    
    for (size_t i = 0; i < _cells.size(); i++) {
        Cell* cell = _cells[i];
        if (!cell->isSpacer() || cell->getWeight() == 0.0f) {
            // Fixed size widget or fixed spacer
            totalFixedSize += cell->getWidget()->getHeight();
        }
    }
    
    return _h - (2 * _margin) - totalSpacing - totalFixedSize;
}

void VerticalLayout::distributeSpace() {
    if (_cells.empty()) return;

    // Total spacing between cells and inner height
    const int16_t totalSpacing = (_cells.size() > 1) ? (_cells.size() - 1) * _spacing : 0;
    const int16_t innerHeight  = _h - (2 * _margin) - totalSpacing;
    if (innerHeight <= 0) return;

    // ---------------------------------------------------------------------
    // 1) Calculate minimum heights for zero-weight widgets and total weight
    // ---------------------------------------------------------------------
    int16_t totalMinimumHeight = 0;  // Height needed for zero-weight widgets
    float   totalFlexWeight    = 0.0f;  // Total weight of flexible widgets

    for (auto *cell : _cells) {
        if (!cell) continue;
        const float weight = cell->getWeight();
        
        if (weight > 0.0f) {
            // This is a flexible widget - it will share remaining space
            totalFlexWeight += weight;
        } else {
            // This is a fixed widget - it needs its minimum height
            int16_t minHeight = 0;
            if (!cell->isSpacer()) {
                Widget *w = cell->getWidget();
                if (w) {
                    minHeight = w->getHeight();
                    // For layouts that initially report 0, give them a reasonable minimum
                    if (minHeight == 0 && cell->getGravity() == Gravity::FILL) {
                        if (w->getType() == WidgetType::HORIZONTAL_LAYOUT || w->getType() == WidgetType::VERTICAL_LAYOUT) {
                            minHeight = WidgetConstants::MIN_LAYOUT_HEIGHT; // Minimum height for button layouts
                        } else {
                            minHeight = WidgetConstants::MIN_WIDGET_SIZE;
                        }
                    }
                }
            }
            totalMinimumHeight += minHeight;
        }
        
        SUI_LOGD("height analysis: idx=%d weight=%.2f minHeight=%d", 
                 (int)std::distance(_cells.begin(), std::find(_cells.begin(), _cells.end(), cell)), 
                 weight, weight > 0.0f ? 0 : (cell->isSpacer() ? 0 : (cell->getWidget() ? cell->getWidget()->getHeight() : 0)));
    }

    // Calculate remaining space for flexible widgets
    int16_t remainingSpace = innerHeight - totalMinimumHeight;
    if (totalFlexWeight <= 0.0f) totalFlexWeight = 1.0f;

    SUI_LOGD("space calculation: innerHeight=%d totalMinimumHeight=%d remainingSpace=%d totalFlexWeight=%.2f", 
             innerHeight, totalMinimumHeight, remainingSpace, totalFlexWeight);

    // ---------------------------------------------------------------------
    // 2) Handle space constraints
    // ---------------------------------------------------------------------
    bool needsConstraining = (remainingSpace < 0);
    SUI_LOGD("constraining: needsConstraining=%s", needsConstraining ? "true" : "false");
    
    if (needsConstraining) {
        // Not enough space - we need to constrain even minimum heights
        SUI_LOGD("constraining: entering constraining mode - not enough space for minimum heights");
        remainingSpace = 0;
    } else {
        SUI_LOGD("constraining: normal mode, remainingSpace=%d available for flexible widgets", remainingSpace);
    }

    // ---------------------------------------------------------------------
    // 3) Layout pass - position widgets
    // ---------------------------------------------------------------------
    int16_t currentY = _y + _margin;
    const int16_t innerWidth = _w - (2 * _margin);

    for (auto *cell : _cells) {
        if (!cell) continue;
        const float weight = cell->getWeight();
        
        int16_t cellHeight = 0;
        
        if (weight > 0.0f) {
            // Flexible widget - gets share of remaining space
            if (needsConstraining) {
                // In constraining mode, distribute total space proportionally
                cellHeight = static_cast<int16_t>((innerHeight * weight) / totalFlexWeight);
            } else {
                // Normal mode - get minimum height plus share of remaining space
                int16_t flexSpace = static_cast<int16_t>((remainingSpace * weight) / totalFlexWeight);
                cellHeight = flexSpace; // Flexible widgets don't have a fixed minimum
            }
        } else {
            // Fixed widget - gets minimum height
            if (!cell->isSpacer()) {
                Widget *w = cell->getWidget();
                if (w) {
                    cellHeight = w->getHeight();
                    if (cellHeight == 0 && cell->getGravity() == Gravity::FILL) {
                        if (w->getType() == WidgetType::HORIZONTAL_LAYOUT || w->getType() == WidgetType::VERTICAL_LAYOUT) {
                            cellHeight = WidgetConstants::MIN_LAYOUT_HEIGHT; // Minimum height for button layouts
                        } else {
                            cellHeight = WidgetConstants::MIN_WIDGET_SIZE;
                        }
                    }
                }
            }
            
            // In constraining mode, even fixed widgets might need to be reduced
            if (needsConstraining && totalMinimumHeight > 0) {
                // Proportionally reduce minimum heights if necessary
                int16_t constrainedHeight = static_cast<int16_t>((innerHeight * cellHeight) / totalMinimumHeight);
                if (constrainedHeight < cellHeight) {
                    cellHeight = constrainedHeight;
                }
            }
        }
        
        // Ensure minimum cell height
        if (cellHeight < WidgetConstants::MIN_WIDGET_SIZE) cellHeight = WidgetConstants::MIN_WIDGET_SIZE;
        
        cell->positionWidget(_x + _margin, currentY, innerWidth, cellHeight);

        // Note: Child containers will automatically recalculate their layout
        // when their size changes through positionWidget(), so we don't need
        // to explicitly call recalculateLayout() here to avoid infinite recursion.

        SUI_LOGI("idx=%d weight=%.2f final=%d posY=%d", 
                 (int)std::distance(_cells.begin(), std::find(_cells.begin(), _cells.end(), cell)), 
                 weight, cellHeight, currentY);
        currentY += cellHeight + _spacing;
    }
}

void VerticalLayout::recalculateLayout() {
    // Prevent infinite recursion
    if (_inLayoutCalculation) {
        SUI_LOGD("recalculateLayout: already in progress, skipping");
        return;
    }
    
    _inLayoutCalculation = true;
    _layoutCalculationDepth++;
    
    SUI_LOGD("recalculateLayout: starting at depth %d", _layoutCalculationDepth);
    
    distributeSpace();

    // Update our own natural width to the widest child so that
    // parent horizontal layouts allocate enough space.
    int16_t maxChildW = 0;
    for (auto *cell : _cells) {
        if (cell && cell->getWidget()) {
            int16_t w = cell->getWidget()->getWidth();
            if (w > maxChildW) maxChildW = w;
        }
    }
    _w = maxChildW + 2 * _margin;

    // Similarly compute total height (natural) so ancestors can size appropriately.
    // Don't recalculate height - use the height allocated by parent layout
    // This allows flexible layouts to expand properly
    // _h = totalH + 2 * _margin;
    
    SUI_LOGD("recalculateLayout: completed at depth %d", _layoutCalculationDepth);
    _layoutCalculationDepth--;
    _inLayoutCalculation = false;
}

void VerticalLayout::draw() {
    // Update layout if needed before drawing
    updateLayoutIfNeeded();
    
    // Draw all children
    Container::draw();
} 