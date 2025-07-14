# SimpleUI Library

A comprehensive, lightweight UI framework for Arduino and ESP32 projects with support for flexible layouts, themes, and a wide variety of widgets.

## Table of Contents

1. [Overview](#overview)
2. [Features](#features)
3. [Installation](#installation)
4. [Quick Start](#quick-start)
5. [Core Concepts](#core-concepts)
6. [Widgets](#widgets)
7. [Layouts](#layouts)
8. [Cell System with Gravity](#cell-system-with-gravity)
9. [Color and Theme System](#color-and-theme-system)
10. [Examples](#examples)
11. [API Reference](#api-reference)
12. [Best Practices](#best-practices)
13. [Contributing](#contributing)

## Overview

SimpleUI is a modern, flexible UI framework designed for embedded systems with displays. It provides a complete solution for creating user interfaces with:

- **Flexible Layout System**: Weight-based layouts with support for nested containers
- **Comprehensive Widget Library**: Buttons, labels, progress bars, sliders, and more
- **Theme System**: Professional color schemes with Material Design support
- **Touch Support**: Built-in touch handling for interactive widgets
- **Memory Efficient**: Optimized for resource-constrained environments

## Features

### 🎨 **Flexible Layout System**
- Weight-based space distribution
- Nested layouts (horizontal and vertical)
- Spacer objects for flexible spacing
- Automatic layout recalculation
- Equal spacing and weight-based modes
- **Cell system with gravity-based positioning**

### 🎯 **Rich Widget Library**
- **Basic Widgets**: Labels, Buttons, Panels
- **Interactive Widgets**: Sliders, CheckBoxes, Progress Bars
- **Data Display**: Gauges, Progress Indicators
- **Layout Widgets**: Containers, Spacers

### 🌈 **Color and Theme System**
- RGB565 color support
- Material Design color palette
- Predefined themes (Light, Dark, Color-specific)
- Custom theme creation
- Color manipulation utilities

### 📱 **Touch Support**
- Built-in touch handling
- Widget-specific touch events
- Automatic hit detection

### ⚡ **Performance Optimized**
- Memory-efficient design
- Minimal RAM usage
- Fast rendering
- Optimized for embedded systems
- **Layout Dirty Flag System**: Only recalculates layouts when necessary
- **Efficient Rendering**: Avoids unnecessary redraws

## Installation

### PlatformIO
Add to your `platformio.ini`:
```ini
lib_deps = 
    SimpleUI
```

### Arduino IDE
1. Download the library
2. Extract to your Arduino libraries folder
3. Restart Arduino IDE

## Quick Start

```cpp
#include <Arduino.h>
#include "ArduinoGFXAdapter.h"               // Example display implementing IDisplayInterface
#include "SimpleUI/SimpleUIApp.h"
#include "SimpleUI/VerticalLayout.h"
#include "SimpleUI/Button.h"
#include "SimpleUI/Label.h"
#include "SimpleUI/Theme.h"

// Global display object (fill in your concrete display driver)
ArduinoGFXAdapter display;
// Create the main application and pass the display interface
SimpleUIApp app(&display);

void setup() {
    // (Initialise your display here if required)
    // display.begin();

    // Create a vertical layout that fills the screen
    auto *layout = new VerticalLayout(0, 0, 320, 240);

    layout->addLabel("SimpleUI Demo", Theme::getText(), 2, 1.0f);
    layout->addSpacer(0.5f);

    layout->addButton("Primary Action", []() {
        Serial.println("Primary action");
    }, Theme::getPrimary(), Theme::getBorder(), Theme::getText(), 1.0f);

    layout->addButton("Secondary Action", []() {
        Serial.println("Secondary action");
    }, Theme::getSecondary(), Theme::getBorder(), Theme::getText(), 1.0f);

    app.addWidget(layout);
}

void loop() {
    app.draw();        // Render UI
    // Optionally handle touch here: app.handleTouch(x, y, pressed);
}
```

## Core Concepts

### Widget Hierarchy
```
SimpleUIApp
└── Root Widget (Layout)
    ├── Child Widget 1
    ├── Child Widget 2
    └── Child Widget 3
```

### Layout System
- **HorizontalLayout**: Arranges widgets horizontally
- **VerticalLayout**: Arranges widgets vertically
- **Weights**: Control relative space allocation
- **Spacers**: Create flexible gaps between widgets
- **Cell System**: Gravity-based positioning within allocated space
- **Layout Optimization**: Smart dirty flag system for efficient recalculation

### Theme System
- **Color Class**: RGB565 color representation and manipulation
- **Theme Class**: Predefined color schemes and theme management
- **Material Design**: Professional color palette

### Layout Optimization System

SimpleUI includes an intelligent layout optimization system that significantly improves performance by avoiding unnecessary layout recalculations:

#### How It Works
- **Dirty Flag Tracking**: Each widget tracks whether its layout needs recalculation
- **Propagation**: Layout dirty flags automatically propagate up the widget tree
- **Lazy Evaluation**: Layouts are only recalculated when actually needed
- **Smart Drawing**: The app checks for dirty layouts before each draw cycle

#### Benefits
- **Performance**: Dramatically reduces CPU usage by avoiding redundant calculations
- **Responsiveness**: Faster UI updates, especially with complex layouts
- **Efficiency**: Only recalculates what has actually changed

#### Usage
The system works automatically - no code changes required. However, you can manually trigger layout updates if needed:

```cpp
// Force layout recalculation for a specific widget
widget->forceLayoutUpdate();

// Check if layout needs recalculation
if (widget->isLayoutDirty()) {
    // Layout will be recalculated on next draw cycle
}
```

#### When Layouts Are Recalculated
- Adding/removing child widgets
- Changing widget properties that affect size (text, colors, etc.)
- Modifying layout parameters (spacing, margins, weights)
- Resizing containers

## Widgets

### Basic Widgets

#### Label
Display text with customizable colors and sizes.
```cpp
Label* label = new Label(0, 0, "Hello World", Color::White, 2);
```

#### Button
Interactive button with click callbacks.
```cpp
Button* button = new Button(0, 0, 100, 40, "Click Me");
button->setOnClick([]() { Serial.println("Clicked!"); });
button->setColors(Color::Blue, Color::Black, Color::White);
```

#### Panel
Container widget for grouping other widgets.
```cpp
Panel* panel = new Panel(0, 0, 200, 150);
panel->addChild(new Label(10, 10, "Panel Content"));
```

### Interactive Widgets

#### Slider
Horizontal or vertical slider with value tracking.
```cpp
Slider* slider = new Slider(0, 0, 200, 20);
slider->setOnChanged([](float value) {
    Serial.printf("Slider value: %.2f\n", value);
});
```

#### CheckBox
Toggle widget with checked/unchecked states.
```cpp
CheckBox* checkbox = new CheckBox(0, 0, 20);
checkbox->setOnChanged([](bool checked) {
    Serial.printf("Checkbox: %s\n", checked ? "ON" : "OFF");
});
```

#### ProgressBar
Visual progress indicator.
```cpp
ProgressBar* progress = new ProgressBar(0, 0, 200, 20);
progress->setProgress(0.75f); // 75% complete
```

### Data Display Widgets

#### Gauge
Circular or linear gauge for displaying values.
```cpp
Gauge* gauge = new Gauge(0, 0, 100);
gauge->setValue(0.75f);
```

## Layouts

### HorizontalLayout
Arranges widgets horizontally with flexible spacing.

```cpp
HorizontalLayout* layout = new HorizontalLayout(0, 0, 300, 50);

// Add widgets with different weights
layout->addButton("Small", callback, 1.0f);   // 1/6 of space
layout->addButton("Medium", callback, 2.0f);  // 2/6 of space
layout->addButton("Large", callback, 3.0f);   // 3/6 of space
```

### VerticalLayout
Arranges widgets vertically with flexible spacing.

```cpp
VerticalLayout* layout = new VerticalLayout(0, 0, 300, 200);

// Add widgets
layout->addLabel("Title", Color::White, 2, 1.0f);
layout->addSpacer(0.5f);  // Flexible spacer
layout->addButton("Action", callback, 1.0f);
```

### Nested Layouts
Create complex arrangements by nesting layouts.

```cpp
// Main vertical layout
VerticalLayout* mainLayout = new VerticalLayout(0, 0, 320, 240);

// Create horizontal toolbar
HorizontalLayout* toolbar = new HorizontalLayout(0, 0, 300, 40);
toolbar->addButton("File", callback);
toolbar->addButton("Edit", callback);
toolbar->addSpacer(1.0f);  // Push buttons to left
toolbar->addButton("Help", callback);

// Add toolbar to main layout
mainLayout->addChild(toolbar, 0.5f);
```

### Spacers
Create flexible gaps between widgets.

```cpp
// Flexible spacer (expands to fill available space)
layout->addSpacer(2.0f);  // Takes 2x the space of other widgets

// Fixed spacer (specific size)
layout->addFixedSpacer(20);  // 20 pixels
```

## Cell System with Gravity

The SimpleUI library includes a powerful Cell system that provides fine-grained control over how widgets are positioned within their allocated space in containers. This system is inspired by Android's gravity system and allows for precise layout control.

### Overview

The Cell system wraps widgets with positioning information, allowing you to control:
- **Gravity**: How the widget is positioned within its allocated space
- **Weight**: How much space the widget should take relative to other widgets
- **Padding**: Internal spacing within the cell

### Gravity Options

The `Gravity` enum provides the following positioning options:

- `TOP_LEFT`: Position widget at the top-left corner
- `TOP_CENTER`: Position widget at the top center
- `TOP_RIGHT`: Position widget at the top-right corner
- `CENTER_LEFT`: Position widget at the center-left
- `CENTER`: Position widget at the center (default)
- `CENTER_RIGHT`: Position widget at the center-right
- `BOTTOM_LEFT`: Position widget at the bottom-left corner
- `BOTTOM_CENTER`: Position widget at the bottom center
- `BOTTOM_RIGHT`: Position widget at the bottom-right corner
- `FILL`: Widget fills the entire allocated space

### Basic Usage

#### Creating Cells with Widgets

```cpp
// Create a container
VerticalLayout* layout = new VerticalLayout(0, 0, 320, 240);

// Add a label with TOP_CENTER gravity
Cell* titleCell = layout->addLabel("My Title", 0xFFFF, 2, 1.0f, Gravity::TOP_CENTER);
titleCell->setPadding(10); // Add padding around the title

// Add a button with CENTER gravity
Cell* buttonCell = layout->addButton("Click Me", []() { 
    Serial.println("Button pressed!"); 
}, 0x07E0, 0x0000, 0xFFFF, 1.0f, Gravity::CENTER);

// Add a progress bar that fills its space
Cell* progressCell = layout->addProgressBar(0.5f, 1.0f, Gravity::FILL);
```

#### Setting Cell Properties

```cpp
// Get a cell from a container method
Cell* cell = layout->addLabel("Example", 0xFFFF);

// Set gravity
cell->setGravity(Gravity::BOTTOM_RIGHT);

// Set weight for layout calculations
cell->setWeight(2.0f);

// Set padding (individual sides)
cell->setPadding(5, 10, 5, 10); // left, top, right, bottom

// Set uniform padding
cell->setPadding(10); // All sides get 10 pixels

// Get the wrapped widget
Widget* widget = cell->getWidget();
```

#### Working with Different Gravity Settings

```cpp
// Create a horizontal layout for buttons
HorizontalLayout* buttonRow = new HorizontalLayout(0, 0, 320, 50);

// Left-aligned button
Cell* leftBtn = buttonRow->addButton("Left", callback, 0x07E0, 0x0000, 0xFFFF, 1.0f, Gravity::CENTER_LEFT);

// Center button
Cell* centerBtn = buttonRow->addButton("Center", callback, 0xF800, 0x0000, 0xFFFF, 1.0f, Gravity::CENTER);

// Right-aligned button
Cell* rightBtn = buttonRow->addButton("Right", callback, 0x001F, 0x0000, 0xFFFF, 1.0f, Gravity::CENTER_RIGHT);
```

### Advanced Usage

#### Nested Layouts with Gravity

```cpp
// Main vertical layout
VerticalLayout* mainLayout = new VerticalLayout(0, 0, 320, 240);

// Add a horizontal layout that fills its space
Cell* hLayoutCell = mainLayout->addHorizontalLayout(2.0f, Gravity::FILL);
HorizontalLayout* hLayout = static_cast<HorizontalLayout*>(hLayoutCell->getWidget());

// Add widgets to the horizontal layout with different gravities
Cell* topWidget = hLayout->addGauge(30, 1.0f, Gravity::TOP_CENTER);
Cell* centerWidget = hLayout->addGauge(30, 1.0f, Gravity::CENTER);
Cell* bottomWidget = hLayout->addGauge(30, 1.0f, Gravity::BOTTOM_CENTER);
```

#### Dynamic Gravity Changes

```cpp
// Create a cell
Cell* cell = layout->addButton("Dynamic", callback);

// Change gravity at runtime
cell->setGravity(Gravity::TOP_LEFT);

// The layout will automatically recalculate positions
```

### Migration from Old System

If you're migrating from the old widget system, here are the key changes:

#### Old Way
```cpp
// Old method - direct widget creation
Label* label = layout->addLabel("Text", 0xFFFF, 1, 1.0f);
Button* button = layout->addButton("Click", callback, 0x07E0, 0x0000, 0xFFFF, 1.0f);
```

#### New Way
```cpp
// New method - returns Cell objects
Cell* labelCell = layout->addLabel("Text", 0xFFFF, 1, 1.0f, Gravity::CENTER);
Cell* buttonCell = layout->addButton("Click", callback, 0x07E0, 0x0000, 0xFFFF, 1.0f, Gravity::CENTER);

// Access the widget if needed
Label* label = static_cast<Label*>(labelCell->getWidget());
Button* button = static_cast<Button*>(buttonCell->getWidget());
```

### Benefits

1. **Precise Control**: Fine-grained positioning within allocated space
2. **Flexible Layouts**: Easy to create complex layouts with different alignments
3. **Consistent API**: All container methods now return Cell objects
4. **Padding Support**: Built-in padding control for better spacing
5. **Runtime Changes**: Can modify gravity and other properties at runtime

## Color and Theme System

### Color Class
Comprehensive color management with RGB565 support.

```cpp
// Create colors
Color red(0xF800);                    // From RGB565
Color green(0, 255, 0);              // From RGB
Color blue = Color::Blue;             // Predefined

// Color manipulation
Color lighter = blue.lighten(50);
Color darker = blue.darken(30);
Color blended = blue.blend(Color::Red, 0.5f);
Color inverted = blue.invert();

// Brightness and contrast
bool isDark = color.isDark();
Color contrast = color.getContrastColor();
```

### Predefined Colors

#### Basic Colors
```cpp
Color::Black, Color::White, Color::Red, Color::Green, Color::Blue
Color::Yellow, Color::Magenta, Color::Cyan, Color::Orange
Color::Purple, Color::Pink, Color::Brown, Color::Gray
Color::LightGray, Color::DarkGray
```

#### Material Design Colors
```cpp
Color::MaterialRed, Color::MaterialBlue, Color::MaterialGreen
Color::MaterialPurple, Color::MaterialOrange, Color::MaterialAmber
// ... and many more
```

### Theme Class
Professional theme management with predefined schemes.

```cpp
// Set predefined themes
Theme::setTheme(Theme::Light);
Theme::setTheme(Theme::Dark);
Theme::setTheme(Theme::Blue);

// Get theme colors
Color primary = Theme::getPrimary();
Color background = Theme::getBackground();
Color text = Theme::getText();
Color success = Theme::getSuccess();
Color warning = Theme::getWarning();
Color error = Theme::getError();
```

### Predefined Themes
- **Light**: White background, dark text, blue primary
- **Dark**: Black background, light text, light blue primary
- **Blue/Green/Red/Purple/Orange**: Color-specific themes
- **Material**: Material Design inspired theme
- **HighContrast**: High contrast accessibility theme

### Custom Themes
```cpp
// Create custom theme
Theme::ColorScheme custom = Theme::createCustomTheme(
    Color::Purple,        // Primary
    Color::Pink,          // Secondary
    Color::White,         // Background
    Color::LightGray,     // Surface
    Color::Black,         // Text
    Color::DarkGray       // Text Secondary
);
Theme::setTheme(custom);
```

## Examples

### Basic Example
```cpp
#include "SimpleUI/SimpleUIApp.h"
#include "SimpleUI/VerticalLayout.h"
#include "SimpleUI/Button.h"
#include "SimpleUI/Label.h"
#include "SimpleUI/Theme.h"

void setup() {
    SimpleUIApp app;
    
    auto *layout = new VerticalLayout(0, 0, 320, 240);
    
    layout->addLabel("SimpleUI Demo", Theme::getText(), 2, 1.0f);
    layout->addSpacer(0.5f);
    
    layout->addButton("Primary Action", []() {
        Serial.println("Primary action");
    }, Theme::getPrimary(), Theme::getBorder(), Theme::getText(), 1.0f);
    
    layout->addButton("Secondary Action", []() {
        Serial.println("Secondary action");
    }, Theme::getSecondary(), Theme::getBorder(), Theme::getText(), 1.0f);
    
    app.setRootWidget(layout);
    app.begin();
}
```

### Complex Layout Example
```cpp
#include "SimpleUI/SimpleUIApp.h"
#include "SimpleUI/VerticalLayout.h"
#include "SimpleUI/HorizontalLayout.h"
#include "SimpleUI/Button.h"
#include "SimpleUI/Label.h"
#include "SimpleUI/ProgressBar.h"
#include "SimpleUI/Theme.h"

void setup() {
    SimpleUIApp app;
    
    // Main layout
    auto *mainLayout = new VerticalLayout(0, 0, 320, 240);
    
    // Title
    mainLayout->addLabel("Complex Layout Demo", Theme::getText(), 2, 1.0f);
    mainLayout->addSpacer(0.3f);
    
    // Toolbar
    auto *toolbar = new HorizontalLayout(0, 0, 300, 40);
    toolbar->addButton("File", []() { Serial.println("File"); });
    toolbar->addButton("Edit", []() { Serial.println("Edit"); });
    toolbar->addSpacer(1.0f);
    toolbar->addButton("Help", []() { Serial.println("Help"); });
    mainLayout->addChild(toolbar, 0.5f);
    
    // Content area
    auto *content = new HorizontalLayout(0, 0, 300, 120);
    
    // Left panel
    auto *leftPanel = new VerticalLayout(0, 0, 100, 120);
    leftPanel->addLabel("Left Panel", Theme::getText(), 1, 1.0f);
    leftPanel->addButton("Option 1", []() { Serial.println("Option 1"); });
    leftPanel->addButton("Option 2", []() { Serial.println("Option 2"); });
    leftPanel->addSpacer(1.0f);
    
    // Right panel
    auto *rightPanel = new VerticalLayout(0, 0, 100, 120);
    rightPanel->addLabel("Right Panel", Theme::getText(), 1, 1.0f);
    rightPanel->addProgressBar(80, 15, 0.75f, 1.0f);
    rightPanel->addLabel("Status: Active", Theme::getSuccess(), 1, 1.0f);
    rightPanel->addSpacer(1.0f);
    
    content->addChild(leftPanel, 1.0f);
    content->addChild(rightPanel, 1.0f);
    
    mainLayout->addChild(content, 2.0f);
    
    app.setRootWidget(mainLayout);
    app.begin();
}

void loop() {
    app.draw();
}
```

### Cell System with Gravity Example
```cpp
#include "SimpleUI/SimpleUIApp.h"
#include "SimpleUI/VerticalLayout.h"
#include "SimpleUI/HorizontalLayout.h"
#include "SimpleUI/Button.h"
#include "SimpleUI/Label.h"
#include "SimpleUI/ProgressBar.h"
#include "SimpleUI/Gauge.h"
#include "SimpleUI/Theme.h"

void setup() {
    SimpleUIApp app;
    
    // Main vertical layout
    VerticalLayout* mainLayout = new VerticalLayout(0, 0, 320, 240);
    
    // Add a title label with TOP_CENTER gravity
    Cell* titleCell = mainLayout->addLabel("Cell Gravity Demo", 0xFFFF, 2, 1.0f, Gravity::TOP_CENTER);
    titleCell->setPadding(10); // Add some padding around the title
    
    // Add a horizontal layout for buttons with different gravities
    Cell* buttonLayoutCell = mainLayout->addHorizontalLayout(2.0f, Gravity::FILL);
    HorizontalLayout* buttonLayout = static_cast<HorizontalLayout*>(buttonLayoutCell->getWidget());
    
    // Add buttons with different gravity settings
    Cell* leftButtonCell = buttonLayout->addButton("Left", []() { Serial.println("Left button pressed"); }, 
                                                   0x07E0, 0x0000, 0xFFFF, 1.0f, Gravity::CENTER_LEFT);
    leftButtonCell->setPadding(5);
    
    Cell* centerButtonCell = buttonLayout->addButton("Center", []() { Serial.println("Center button pressed"); }, 
                                                    0xF800, 0x0000, 0xFFFF, 1.0f, Gravity::CENTER);
    centerButtonCell->setPadding(5);
    
    Cell* rightButtonCell = buttonLayout->addButton("Right", []() { Serial.println("Right button pressed"); }, 
                                                    0x001F, 0x0000, 0xFFFF, 1.0f, Gravity::CENTER_RIGHT);
    rightButtonCell->setPadding(5);
    
    // Add a progress bar that fills its space
    Cell* progressCell = mainLayout->addProgressBar(0.7f, 1.0f, Gravity::FILL);
    progressCell->setPadding(10);
    
    // Add a horizontal layout for gauges
    Cell* gaugeLayoutCell = mainLayout->addHorizontalLayout(1.5f, Gravity::FILL);
    HorizontalLayout* gaugeLayout = static_cast<HorizontalLayout*>(gaugeLayoutCell->getWidget());
    
    // Add gauges with different gravity settings
    Cell* topGaugeCell = gaugeLayout->addGauge(40, 1.0f, Gravity::TOP_CENTER);
    topGaugeCell->setPadding(5);
    
    Cell* centerGaugeCell = gaugeLayout->addGauge(40, 1.0f, Gravity::CENTER);
    centerGaugeCell->setPadding(5);
    
    Cell* bottomGaugeCell = gaugeLayout->addGauge(40, 1.0f, Gravity::BOTTOM_CENTER);
    bottomGaugeCell->setPadding(5);
    
    // Add a spacer to push everything to the top
    mainLayout->addSpacer(1.0f);
    
    app.setRootWidget(mainLayout);
    app.begin();
}

void loop() {
    app.draw();
}
```

## API Reference

### Core Classes

#### SimpleUIApp
Main application class that manages the UI lifecycle.
```cpp
class SimpleUIApp {
public:
    explicit SimpleUIApp(IDisplayInterface* display);
    ~SimpleUIApp();

    void addWidget(Widget* w);
    void clear();
    void draw();
    void handleTouch(int16_t x, int16_t y, bool pressed);
};
```

#### Widget
Base class for all UI elements.
```cpp
class Widget {
public:
    Widget(int16_t x, int16_t y, int16_t w, int16_t h);
    virtual void draw() = 0;
    virtual void handleTouch(int16_t x, int16_t y, bool pressed);
    void setPosition(int16_t x, int16_t y);
    void setSize(int16_t w, int16_t h);
    bool contains(int16_t x, int16_t y) const;
};
```

#### Container
Base class for layout containers.
```cpp
class Container : public Widget {
public:
    virtual void addChild(Widget* w, float weight = 1.0f, Gravity gravity = Gravity::CENTER);
    virtual void addCell(Cell* cell);
    const std::vector<Cell*>& getCells() const;
    virtual void setCellWeight(int index, float weight);
    virtual void setCellGravity(int index, Gravity gravity);
};
```

#### Cell
Wrapper class for widgets with gravity-based positioning.
```cpp
class Cell {
public:
    Cell(Widget* widget, Gravity gravity = Gravity::CENTER, float weight = 1.0f);
    
    Widget* getWidget() const;
    Gravity getGravity() const;
    void setGravity(Gravity gravity);
    float getWeight() const;
    void setWeight(float weight);
    bool isSpacer() const;
    
    void setPadding(int16_t left, int16_t top, int16_t right, int16_t bottom);
    void setPadding(int16_t padding);
    int16_t getPaddingLeft() const;
    int16_t getPaddingTop() const;
    int16_t getPaddingRight() const;
    int16_t getPaddingBottom() const;
    
    void positionWidget(int16_t cellX, int16_t cellY, int16_t cellWidth, int16_t cellHeight);
    void draw();
    void handleTouch(int16_t x, int16_t y, bool pressed);
};
```

### Layout Classes

#### HorizontalLayout
```cpp
class HorizontalLayout : public Container {
public:
    HorizontalLayout(int16_t x, int16_t y, int16_t w, int16_t h, 
                     int16_t margin = 10, int16_t spacing = 5);
    
    void addChild(Widget* w, float weight = 1.0f, Gravity gravity = Gravity::CENTER) override;
    void addSpacer(float weight = 1.0f);
    void addFixedSpacer(int16_t size);
    void setSpacing(int16_t spacing);
    void setMargin(int16_t margin);
    void setEqualSpacing(bool equal);
    
    // Helper methods (return Cell* objects)
    Cell* addLabel(const String& text, uint16_t color = 0xFFFF, 
                   uint8_t textSize = 1, float weight = 1.0f, Gravity gravity = Gravity::CENTER);
    Cell* addButton(const String& text, std::function<void()> onClick,
                    uint16_t bgColor = 0x07E0, uint16_t borderColor = 0x0000,
                    uint16_t textColor = 0xFFFF, float weight = 1.0f, Gravity gravity = Gravity::CENTER);
    Cell* addProgressBar(float progress = 0.0f, float weight = 1.0f, Gravity gravity = Gravity::FILL);
    Cell* addCheckBox(int16_t size = 20, float weight = 1.0f, Gravity gravity = Gravity::CENTER);
    Cell* addSlider(int16_t height = 20, float weight = 1.0f, Gravity gravity = Gravity::FILL);
    Cell* addGauge(int16_t size = 50, float weight = 1.0f, Gravity gravity = Gravity::CENTER);
    Cell* addVerticalLayout(float weight = 1.0f, Gravity gravity = Gravity::FILL);
    Cell* addHorizontalLayout(float weight = 1.0f, Gravity gravity = Gravity::FILL);
    Cell* addSpacer(float weight = 1.0f);
    Cell* addFixedSpacer(int16_t size);
};
```

#### VerticalLayout
```cpp
class VerticalLayout : public Container {
public:
    VerticalLayout(int16_t x, int16_t y, int16_t w, int16_t h,
                   int16_t margin = 10, int16_t spacing = 5);
    
    void addChild(Widget* w, float weight = 1.0f, Gravity gravity = Gravity::CENTER) override;
    void addSpacer(float weight = 1.0f);
    void addFixedSpacer(int16_t size);
    void setSpacing(int16_t spacing);
    void setMargin(int16_t margin);
    void setEqualSpacing(bool equal);
    
    // Helper methods (return Cell* objects)
    Cell* addLabel(const String& text, uint16_t color = 0xFFFF,
                   uint8_t textSize = 1, float weight = 1.0f, Gravity gravity = Gravity::CENTER);
    Cell* addButton(const String& text, std::function<void()> onClick,
                    uint16_t bgColor = 0x07E0, uint16_t borderColor = 0x0000,
                    uint16_t textColor = 0xFFFF, float weight = 1.0f, Gravity gravity = Gravity::CENTER);
    Cell* addProgressBar(float progress = 0.0f, float weight = 1.0f, Gravity gravity = Gravity::FILL);
    Cell* addCheckBox(int16_t size = 20, float weight = 1.0f, Gravity gravity = Gravity::CENTER);
    Cell* addSlider(int16_t height = 20, float weight = 1.0f, Gravity gravity = Gravity::FILL);
    Cell* addGauge(int16_t size = 50, float weight = 1.0f, Gravity gravity = Gravity::CENTER);
    Cell* addHorizontalLine(uint16_t color = 0xFFFF, int16_t thickness = 1, float weight = 1.0f);
    Cell* addVerticalLayout(float weight = 1.0f, Gravity gravity = Gravity::FILL);
    Cell* addHorizontalLayout(float weight = 1.0f, Gravity gravity = Gravity::FILL);
    Cell* addSpacer(float weight = 1.0f);
    Cell* addFixedSpacer(int16_t size);
};
```

### Widget Classes

#### Label
```cpp
class Label : public Widget {
public:
    Label(int16_t x, int16_t y, const String& text, 
          uint16_t color = 0xFFFF, uint8_t textSize = 1);
    
    void setText(const String& text);
    void setTextSize(uint8_t size);
    void setTextColor(uint16_t color);
    const String& getText() const;
    int16_t getWidth() const override;
    int16_t getHeight() const override;
};
```

#### Button
```cpp
class Button : public Widget {
public:
    Button(int16_t x, int16_t y, int16_t w, int16_t h, 
           const String& text, uint8_t textSize = 1);
    
    void setColors(uint16_t bg, uint16_t border, uint16_t text);
    void setTextSize(uint8_t size);
    void setOnClick(std::function<void()> cb);
};
```

#### ProgressBar
```cpp
class ProgressBar : public Widget {
public:
    ProgressBar(int16_t x, int16_t y, int16_t w, int16_t h);
    
    void setProgress(float progress);
};
```

#### Slider
```cpp
class Slider : public Widget {
public:
    Slider(int16_t x, int16_t y, int16_t w, int16_t h);
    
    void setValue(float value);
    float getValue() const;
    void setOnChanged(std::function<void(float)> callback);
};
```

#### CheckBox
```cpp
class CheckBox : public Widget {
public:
    CheckBox(int16_t x, int16_t y, int16_t size);
    
    void setChecked(bool checked);
    bool isChecked() const;
    void setOnChanged(std::function<void(bool)> callback);
};
```

#### Gauge
```cpp
class Gauge : public Widget {
public:
    Gauge(int16_t x, int16_t y, int16_t size);
    
    void setValue(float value);
};
```

### Color and Theme Classes

#### Color
```cpp
class Color {
public:
    Color(uint16_t rgb565 = 0x0000);
    Color(uint8_t r, uint8_t g, uint8_t b);
    
    static uint16_t color565(uint8_t r, uint8_t g, uint8_t b);
    uint16_t getValue() const;
    operator uint16_t() const;
    
    uint8_t getRed() const;
    uint8_t getGreen() const;
    uint8_t getBlue() const;
    
    Color lighten(uint8_t amount) const;
    Color darken(uint8_t amount) const;
    Color blend(const Color& other, float ratio) const;
    Color invert() const;
    
    uint8_t getBrightness() const;
    bool isDark() const;
    bool isLight() const;
    Color getContrastColor() const;
    
    // Predefined colors
    static const Color Black, White, Red, Green, Blue, Yellow, Magenta, Cyan;
    static const Color Orange, Purple, Pink, Brown, Gray, LightGray, DarkGray;
    // Material Design colors
    static const Color MaterialRed, MaterialBlue, MaterialGreen, MaterialPurple;
    // ... and many more
};
```

#### Theme
```cpp
class Theme {
public:
    struct ColorScheme {
        Color primary, secondary, background, surface;
        Color text, textSecondary, success, warning, error, info;
        Color border, shadow;
    };
    
    // Predefined themes
    static const ColorScheme Light, Dark, Blue, Green, Red, Purple, Orange;
    static const ColorScheme Material, HighContrast;
    
    // Theme management
    static void setTheme(const ColorScheme& theme);
    static const ColorScheme& getCurrentTheme();
    
    // Quick access methods
    static Color getPrimary(), getSecondary(), getBackground(), getSurface();
    static Color getText(), getTextSecondary(), getSuccess(), getWarning();
    static Color getError(), getInfo(), getBorder(), getShadow();
    
    // Theme creation
    static ColorScheme createCustomTheme(Color primary, Color secondary = Color::Gray,
                                        Color background = Color::White,
                                        Color surface = Color::LightGray,
                                        Color text = Color::Black,
                                        Color textSecondary = Color::DarkGray);
    static ColorScheme createDarkTheme(Color primary);
    static ColorScheme createLightTheme(Color primary);
};
```

## Best Practices

### 1. **Use Theme Colors for Consistency**
```cpp
// Good: Use theme colors
button->setColors(Theme::getPrimary(), Theme::getBorder(), Theme::getText());

// Avoid: Hard-coded colors
button->setColors(0x001F, 0x0000, 0xFFFF);
```

### 2. **Plan Your Layout Hierarchy**
```cpp
// Design your layout structure before implementation
// Main Layout (Vertical)
// ├── Header (Horizontal)
// ├── Content Area (Horizontal)
// │   ├── Left Panel (Vertical)
// │   └── Right Panel (Vertical)
// └── Footer (Horizontal)
```

### 3. **Use Appropriate Weights**
```cpp
// Use weights for proportional layouts
layout->addButton("Small", callback, 1.0f);   // 1/6 of space
layout->addButton("Medium", callback, 2.0f);  // 2/6 of space
layout->addButton("Large", callback, 3.0f);   // 3/6 of space
```

### 4. **Leverage the Cell System for Precise Positioning**
```cpp
// Use gravity for precise widget positioning
Cell* titleCell = layout->addLabel("Title", 0xFFFF, 2, 1.0f, Gravity::TOP_CENTER);
titleCell->setPadding(10); // Add padding for better spacing

// Use FILL gravity for widgets that should expand
Cell* progressCell = layout->addProgressBar(0.5f, 1.0f, Gravity::FILL);

// Use different gravities for visual variety
Cell* leftBtn = layout->addButton("Left", callback, 1.0f, Gravity::CENTER_LEFT);
Cell* centerBtn = layout->addButton("Center", callback, 1.0f, Gravity::CENTER);
Cell* rightBtn = layout->addButton("Right", callback, 1.0f, Gravity::CENTER_RIGHT);
```

### 5. **Use Spacers for Alignment**
```cpp
// Create visual separation and alignment
layout->addLabel("Left", 1.0f);
layout->addSpacer(2.0f);  // Push content apart
layout->addLabel("Right", 1.0f);
```

### 6. **Consider Screen Size**
```cpp
// Ensure layouts work on different screen sizes
// Use relative positioning and flexible layouts
// Test on actual hardware
```

### 7. **Use Semantic Colors**
```cpp
// Use semantic colors for different states
Color successColor = Theme::getSuccess();  // For success actions
Color warningColor = Theme::getWarning();  // For warnings
Color errorColor = Theme::getError();      // For errors
```

### 8. **Optimize for Performance**
```cpp
// Minimize widget creation in loops
// Use efficient layout algorithms
// Consider memory usage on constrained devices
```

### 9. **Handle Touch Events Properly**
```cpp
// Provide visual feedback for touch events
// Use appropriate touch areas
// Consider accessibility
```

## Contributing

We welcome contributions to SimpleUI! Here's how you can help:

### Reporting Issues
- Use the issue tracker to report bugs
- Include detailed reproduction steps
- Provide hardware and software information

### Suggesting Features
- Use the issue tracker for feature requests
- Describe the use case and benefits
- Consider implementation complexity

### Code Contributions
- Fork the repository
- Create a feature branch
- Follow the existing code style
- Add tests if applicable
- Submit a pull request

### Development Setup
1. Clone the repository
2. Install dependencies
3. Set up your development environment
4. Run tests
5. Make your changes
6. Submit a pull request

## License

This library is licensed under the MIT License. See the LICENSE file for details.

## Support

- **Documentation**: This README and example projects
- **Issues**: Use the issue tracker for bugs and feature requests
- **Examples**: Check the examples directory for usage patterns
- **Community**: Join discussions in the project repository

## Changelog

### Version 1.1.0
- **Cell System with Gravity**: Added comprehensive gravity-based positioning system
- **Enhanced Layout Control**: Fine-grained control over widget positioning within containers
- **Padding Support**: Built-in padding control for better spacing
- **Improved API**: All container methods now return Cell objects for better control
- **Backward Compatibility**: Maintains compatibility with existing code

### Version 1.0.0
- Initial release with core widgets
- Flexible layout system
- Color and theme system
- Touch support
- Comprehensive documentation

---

**SimpleUI** - Making embedded UI development simple and beautiful! 🎨✨ 