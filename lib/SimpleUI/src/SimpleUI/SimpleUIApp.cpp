#include "SimpleUI/SimpleUIApp.h"
#include "SimpleUI/Container.h"

// Define log tag
static const char* TAG = "SimpleUIApp";

SimpleUIApp::SimpleUIApp(ICacheDisplay *display)
    : _display(display), _isDirty(true) {
    // Set the app reference for widgets to use for dirty notification
    Widget::setApp(this);
}

SimpleUIApp::~SimpleUIApp() {
    clear();
}

void SimpleUIApp::addWidget(Widget *w) {
    if (!w) return;
    w->setDisplayInterface(_display);
    _widgets.push_back(w);
    markDirty(); // New widget added, mark as dirty
}

void SimpleUIApp::clear() {
    // Delete all widgets and clear the vector
    for (auto* widget : _widgets) {
        if (widget) {
            delete widget;
        }
    }
    _widgets.clear();
    markDirty(); // Widgets cleared, mark as dirty
}

void SimpleUIApp::draw() {
    // Only draw if the application is dirty
    if (!_isDirty) {
        return;
    }
    
    uint32_t visibleWidgets = 0;
    uint32_t layoutRecalculations = 0;
    
    // First pass: Update layouts if needed
    for (auto *w : _widgets) {
        if (w && w->isVisible()) {
            // Check if this widget or any of its children need layout recalculation
            if (w->isLayoutDirty()) {
                // Check if this is a container by using the helper method
                if (w->isContainer()) {
                    // Cast to Container (safe since we know it's a container type)
                    Container* container = static_cast<Container*>(w);
                    container->updateLayoutIfNeeded();
                    layoutRecalculations++;
                }
            }
        }
    }
    
    // Second pass: Draw all widgets
    for (auto *w : _widgets) {
        if (w && w->isVisible()) {
            w->draw();
            visibleWidgets++;
        }
    }

    if (_display) {
        _display->updateCache();
    }
    
    // Mark as clean after drawing
    markClean();
    
    // Log performance metrics occasionally
    static uint32_t drawCount = 0;
    drawCount++;
    if (drawCount % 100 == 0) {
        ESP_LOGD(TAG, "Draw cycle: %d visible widgets, %d layout recalculations", visibleWidgets, layoutRecalculations);
    }
}

void SimpleUIApp::handleTouch(int16_t x, int16_t y, bool pressed) {
    for (auto *w : _widgets) {
        if (w) w->handleTouch(x, y, pressed);
    }
}

void SimpleUIApp::markDirty() {
    _isDirty = true;
}

bool SimpleUIApp::isDirty() const {
    return _isDirty;
}

void SimpleUIApp::markClean() {
    _isDirty = false;
} 