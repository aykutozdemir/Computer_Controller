#define SIMPLEUI_TAG "HorizontalLayout"
#include "SimpleUI/SimpleUIDebug.h"
#include "HorizontalLayout.h"
#include "Label.h"
#include "Button.h"
#include "Spacer.h"
#include "VerticalLayout.h"

HorizontalLayout::HorizontalLayout(int16_t x, int16_t y, int16_t w, int16_t h, int16_t margin, int16_t spacing)
    : Container(x, y, w, h) {
    _type = WidgetType::HORIZONTAL_LAYOUT;
    _margin = margin;
    _spacing = spacing;
}

void HorizontalLayout::addChild(Widget *w, float weight, Gravity gravity) {
    if (!w) return;
    
    // Add to children list using base class method
    Container::addChild(w, weight, gravity);
    
    // Layout will be recalculated when needed via the dirty flag system
    // No need to call recalculateLayout() here anymore
}

int16_t HorizontalLayout::calculateAvailableSpace() {
    int16_t totalFixedSize = 0;
    int16_t totalSpacing = (_cells.size() > 1) ? (_cells.size() - 1) * _spacing : 0;
    
    for (size_t i = 0; i < _cells.size(); i++) {
        Cell* cell = _cells[i];
        if (!cell->isSpacer() || cell->getWeight() == 0.0f) {
            // Fixed size widget or fixed spacer
            totalFixedSize += cell->getWidget()->getWidth();
        }
    }
    
    return _w - (2 * _margin) - totalSpacing - totalFixedSize;
}

void HorizontalLayout::distributeSpace() {
    if (_cells.empty()) return;
    
    // Space taken by margins and spacing between cells
    const int16_t totalSpacing = (_cells.size() > 1) ? (_cells.size() - 1) * _spacing : 0;
    const int16_t innerWidth   = _w - (2 * _margin) - totalSpacing;
    if (innerWidth <= 0) return; // Nothing to lay out

    // ---------------------------------------------------------------------
    // 1) Determine natural (minimum) width of every cell and accumulate
    // ---------------------------------------------------------------------
    int16_t totalNaturalWidth = 0;
    float   totalFlexWeight   = 0.0f;

    // First pass – collect metrics
    int16_t maxNaturalHeight = 0;
    for (auto *cell : _cells) {
        if (!cell) continue;

        const float weight = cell->getWeight();
        // Treat cells whose gravity is FILL but weight is 0 as having an implicit flexible weight of 1
        const float effWeight = (weight > 0.0f) ? weight : (cell->getGravity() == Gravity::FILL ? 1.0f : 0.0f);
        bool treatAsFlexible = (_equalSpacing && weight > 0.0f);
        int16_t natural = 0;
        if (!(cell->isSpacer() && weight > 0.0f) && !treatAsFlexible) {
            // For all real widgets **and** fixed-size spacers, ask widget.
            Widget *w = cell->getWidget();
            natural = w ? w->getWidth() : 0;
            if (natural == 0 && cell->getGravity() == Gravity::FILL) {
                natural = WidgetConstants::MIN_WIDGET_SIZE;
            }

            // Track height for each widget to decide our own height
            int16_t h = w ? w->getHeight() : 0;
            SUI_LOGD("Widget type %d reports height %d", w ? static_cast<int>(w->getType()) : -1, h);
            if (h > maxNaturalHeight) maxNaturalHeight = h;
        }
        
        // Accumulate natural width for all cells
        totalNaturalWidth += natural;

        if (effWeight > 0.0f) {
            totalFlexWeight += effWeight;
        }
    }

    // Ensure our height reflects tallest child before laying them out
    // Don't recalculate height - use the height allocated by parent layout
    // _h = maxNaturalHeight + 2 * _margin;

    // Recompute innerHeight now that _h is known
    const int16_t innerHeight = _h - (2 * _margin);

    SUI_LOGD("maxNaturalHeight=%d, _h=%d, innerHeight=%d", maxNaturalHeight, _h, innerHeight);

    // Calculate how much space is left after giving every cell its natural width
    int16_t remainingSpace = innerWidth - totalNaturalWidth;
    if (remainingSpace < 0) remainingSpace = 0; // Widgets don't fit; they will touch but not overlap

    // -----------------------------------------------------------------
    // Special-case: If there are *no* flexible cells (all weight == 0)
    // but some space remains, we still want to honour right/FILL gravity
    // so that (for instance) a lone CENTER_RIGHT label can be flushed
    // to the right edge. We therefore nominate ONE candidate cell that
    // will absorb all remaining pixels.
    // -----------------------------------------------------------------
    int candidateIndex = -1;
    if (totalFlexWeight <= 0.0f && remainingSpace > 0) {
        for (size_t i = 0; i < _cells.size(); ++i) {
            Gravity g = _cells[i]->getGravity();
            if (g == Gravity::FILL || g == Gravity::CENTER_RIGHT ||
                g == Gravity::TOP_RIGHT || g == Gravity::BOTTOM_RIGHT) {
                candidateIndex = static_cast<int>(i);
                break; // Use first matching cell
            }
        }
        totalFlexWeight = (candidateIndex >= 0) ? 1.0f : 1.0f; // still avoid /0
    } else if (totalFlexWeight <= 0.0f) {
        totalFlexWeight = 1.0f; // Avoid div-by-zero when nothing to distribute
    }

    // ---------------------------------------------------------------------
    // 2) Second pass – assign final widths and position widgets
    // ---------------------------------------------------------------------
    int16_t currentX = _x + _margin;
    // (innerHeight defined earlier)

    for (size_t idx = 0; idx < _cells.size(); ++idx) {
        Cell* cell = _cells[idx];
        if (!cell) continue;

        const float weight = cell->getWeight();
        const float effWeight = (weight > 0.0f) ? weight : (cell->getGravity() == Gravity::FILL ? 1.0f : 0.0f);
        bool treatAsFlexible2 = (_equalSpacing && weight > 0.0f);
        int16_t natural = 0;
        if (!(cell->isSpacer() && weight > 0.0f) && !treatAsFlexible2) {
            Widget *w = cell->getWidget();
            natural = w ? w->getWidth() : 0;
            if (natural == 0 && cell->getGravity() == Gravity::FILL) {
                natural = WidgetConstants::MIN_WIDGET_SIZE;
            }
        }

        // Extra width goes only to flexible-weight cells
        int16_t extra = 0;
        if (effWeight > 0.0f && remainingSpace > 0) {
            extra = static_cast<int16_t>((remainingSpace * effWeight) / totalFlexWeight);
        }

        int16_t cellWidth = natural + extra;

        // Special-case: The nominated right-aligned candidate gets shifted
        // rather than stretched.
        if (static_cast<int>(idx) == candidateIndex && remainingSpace > 0 && effWeight == 0.0f) {
            // Start X such that the cell's right edge is flush with the
            // inner layout width.
            currentX = _x + _margin + innerWidth - cellWidth;
        }

        cell->positionWidget(currentX, _y + _margin, cellWidth, innerHeight);

        SUI_LOGD("Called positionWidget on cell %d, widget type %d at (%d,%d) size %dx%d", 
                 (int)idx, cell->getWidget() ? static_cast<int>(cell->getWidget()->getType()) : -1, 
                 currentX, _y + _margin, cellWidth, innerHeight);

        SUI_LOGI("idx=%d natural=%d extra=%d final=%d posX=%d", (int)idx, natural, extra, cellWidth, currentX);

        currentX += cellWidth + _spacing;
    }
}

int16_t HorizontalLayout::getHeight() const {
    if (_cells.empty()) return _h;
    
    int16_t maxChildHeight = 0;
    for (auto *cell : _cells) {
        if (!cell) continue;
        Widget *w = cell->getWidget();
        if (w) {
            int16_t childHeight = w->getHeight();
            SUI_LOGD("getHeight: child type %d reports height %d", static_cast<int>(w->getType()), childHeight);
            if (childHeight > maxChildHeight) {
                maxChildHeight = childHeight;
            }
        }
    }
    
    // Return the height needed: tallest child + margins
    int16_t result = maxChildHeight + (2 * _margin);
    SUI_LOGD("getHeight: maxChildHeight=%d, margin=%d, result=%d", maxChildHeight, _margin, result);
    return result;
}

void HorizontalLayout::recalculateLayout() {
    // Prevent infinite recursion
    if (_inLayoutCalculation) {
        SUI_LOGD("recalculateLayout: already in progress, skipping");
        return;
    }
    
    _inLayoutCalculation = true;
    _layoutCalculationDepth++;
    
    SUI_LOGD("recalculateLayout: starting at depth %d", _layoutCalculationDepth);
    
    distributeSpace();

    // -----------------------------------------------------------------
    // Update our own natural height so parent containers know how tall
    // we actually are. We take the maximum widget height among all
    // children plus top/bottom margins.
    // -----------------------------------------------------------------
    int16_t maxChildH = 0;
    for (auto *cell : _cells) {
        if (cell && cell->getWidget()) {
            int16_t h = cell->getWidget()->getHeight();
            if (h > maxChildH) maxChildH = h;
        }
    }
    // Include vertical margins in the reported height
    _h = maxChildH + 2 * _margin;
    
    SUI_LOGD("recalculateLayout: completed at depth %d", _layoutCalculationDepth);
    _layoutCalculationDepth--;
    _inLayoutCalculation = false;
}

void HorizontalLayout::draw() {
    // Update layout if needed before drawing
    updateLayoutIfNeeded();
    
    // Draw all children
    Container::draw();
} 