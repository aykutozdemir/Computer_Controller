/**
 * @file SimpleUIApp.h
 * @brief Root application class managing the widget tree and rendering for the SimpleUI library.
 */

#pragma once

#include <vector>
#include "Widget.h"
#include "ICacheDisplay.h"

class SimpleUIApp {
private:
    std::vector<Widget *> _widgets;
    ICacheDisplay *_display;
    bool _isDirty;
public:
    /// Construct the application with a display interface.
    explicit SimpleUIApp(ICacheDisplay *display);
    /// Destructor, cleans up widgets.
    ~SimpleUIApp();

    /// Add a top-level widget to the application.
    void addWidget(Widget *w);
    /// Remove and delete all widgets.
    void clear();
    /// Draw all widgets to the display.
    void draw();
    /// Forward touch events to widgets.
    void handleTouch(int16_t x, int16_t y, bool pressed);
    
    /// Mark the application as dirty (needs redrawing).
    void markDirty();
    
    /// Check if the application is dirty.
    bool isDirty() const;
    
    /// Clear the dirty flag.
    void markClean();
}; 