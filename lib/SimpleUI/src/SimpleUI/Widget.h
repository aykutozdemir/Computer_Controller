/**
 * @file Widget.h
 * @brief Base class for all widgets in the SimpleUI library.
 */

#pragma once

#include <Arduino.h>
#include "ICacheDisplay.h"
#include <functional>
#include "SimpleUI/SimpleUIDebug.h"

// Widget type enumeration
enum class WidgetType {
    WIDGET,
    LABEL,
    BUTTON,
    PANEL,
    PROGRESS_BAR,
    SPACER,
    VERTICAL_LAYOUT,
    HORIZONTAL_LAYOUT,
    HLINE
};

// Constants for consistent sizing and spacing
namespace WidgetConstants {
    constexpr int16_t MIN_WIDGET_SIZE = 1;
    constexpr int16_t MIN_BUTTON_HEIGHT = 20;
    constexpr int16_t MIN_PROGRESS_BAR_HEIGHT = 20;
    constexpr int16_t MIN_SLIDER_HEIGHT = 20;
    constexpr int16_t MIN_LAYOUT_HEIGHT = 44;
    
    constexpr int16_t BUTTON_PADDING_HORIZONTAL = 20; // 10 left + 10 right
    constexpr int16_t BUTTON_PADDING_VERTICAL = 10;   // 5 top + 5 bottom
    
    constexpr int16_t TEXT_CHAR_WIDTH = 6;  // pixels per character (5 glyph + 1 spacing)
    constexpr int16_t TEXT_BASELINE_HEIGHT = 8; // baseline height in pixels
}

class Widget {
protected:
    int16_t _x{0}, _y{0}, _w{0}, _h{0};
    bool _visible{true};
    ICacheDisplay *_display{nullptr};
    WidgetType _type{WidgetType::WIDGET};
    Widget *_parent{nullptr};  // Pointer to parent container

    // Flag indicating the widget needs to be redrawn
    bool _dirty{true};
    
    // Flag indicating the widget's layout needs recalculation
    bool _layoutDirty{true};
    
    // Static reference to the app for dirty notification
    static class SimpleUIApp* _app;

    // Mark the widget as needing a redraw
    void markDirty();
    
    // Mark only the content as dirty (for text changes, etc.) without affecting layout
    void markContentDirty();
    
    // Mark the widget's layout as needing recalculation
    void markLayoutDirty() { 
        _layoutDirty = true; 
        // Only propagate layout dirty flag up if this widget's size/position affects parent layout
        // For containers, this means their size changed, not just their content
        if (_parent && isContainer()) {
            _parent->markLayoutDirty();
        }
    }

    // Clear the dirty flag after successful drawing
    void markClean() { _dirty = false; }
    
    // Clear the layout dirty flag after successful layout calculation
    void markLayoutClean() { _layoutDirty = false; }

    // Utility methods for consistent text measurements
    int16_t measureTextWidth(const String& text, uint8_t textSize) const;
    int16_t measureTextHeight(uint8_t textSize) const;
    
    // Utility methods for consistent fallback calculations
    int16_t getFallbackTextWidth(const String& text, uint8_t textSize) const;
    int16_t getFallbackTextHeight(uint8_t textSize) const;

public:
    /// Construct a Widget with position and size.
    Widget(int16_t x, int16_t y, int16_t w, int16_t h);
    /// Virtual destructor.
    virtual ~Widget();

    /// Set the display interface used for drawing.
    void setDisplayInterface(ICacheDisplay *display) { _display = display; }
    /// Show or hide the widget.
    void setVisible(bool v);
    /// Check whether the widget is visible.
    bool isVisible() const;

    /// Check if a point lies within widget bounds.
    bool contains(int16_t px, int16_t py) const;
    
    /// Set widget position.
    void setPosition(int16_t x, int16_t y);
    /// Set widget size.
    void setSize(int16_t w, int16_t h);
    /// Get X coordinate.
    int16_t getX() const { return _x; }
    /// Get Y coordinate.
    int16_t getY() const { return _y; }
    /// Get widget width.
    virtual int16_t getWidth() const { return _w; }
    /// Get widget height.
    virtual int16_t getHeight() const { return _h; }

    /// Return the widget type.
    WidgetType getType() const { return _type; }
    /// Convenience type check helper.
    bool isType(WidgetType type) const { return _type == type; }
    
    /// Check if this widget is a container type (has layout capabilities).
    bool isContainer() const {
        return _type == WidgetType::VERTICAL_LAYOUT || 
               _type == WidgetType::HORIZONTAL_LAYOUT || 
               _type == WidgetType::WIDGET; // Container uses WIDGET type
    }

    /// Check whether the widget requires redrawing.
    bool isDirty() const { return _dirty; }
    
    /// Check whether the widget's layout requires recalculation.
    bool isLayoutDirty() const { return _layoutDirty; }
    
    /// Force layout recalculation (for debugging or special cases).
    void forceLayoutUpdate() { markLayoutDirty(); }

    /// Get the parent container widget.
    Widget* getParent() const { return _parent; }
    /// Set the parent container widget (internal use).
    void setParent(Widget* parent) { _parent = parent; }

    /// Draw the widget.
    virtual void draw() = 0;
    /// Handle touch events.
    virtual void handleTouch(int16_t x, int16_t y, bool pressed) {}
    
    /// Set the app reference for dirty notification (static method).
    static void setApp(class SimpleUIApp* app);
}; 