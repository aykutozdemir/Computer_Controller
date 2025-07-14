#define SIMPLEUI_TAG "Container"
#include "SimpleUI/SimpleUIDebug.h"
#include "SimpleUI/Container.h"
#include "SimpleUI/Cell.h"
#include "SimpleUI/Label.h"
#include "SimpleUI/Button.h"
#include "SimpleUI/ProgressBar.h"
#include "SimpleUI/CheckBox.h"
#include "SimpleUI/Slider.h"
#include "SimpleUI/Gauge.h"
#include "SimpleUI/HorizontalLine.h"
#include "SimpleUI/VerticalLayout.h"
#include "SimpleUI/HorizontalLayout.h"
#include "SimpleUI/Spacer.h"

Container::Container(int16_t x, int16_t y, int16_t w, int16_t h)
    : Widget(x, y, w, h), _margin(10), _spacing(5), _equalSpacing(true) {
    _type = WidgetType::WIDGET; // Container is base type
    SUI_LOGI("ctor: (%d,%d) %dx%d", x, y, w, h);
}

Container::~Container() = default;

void Container::addChild(Widget *w, float weight, Gravity gravity) {
    if (!w) return;
    w->setDisplayInterface(_display);
    w->setParent(this);
    Cell* cell = new Cell(w, gravity, weight);
    _cells.push_back(cell);
    SUI_LOGD("addChild: type=%d weight=%.2f gravity=%d cells=%d", static_cast<int>(w->getType()), weight, static_cast<int>(gravity), (int)_cells.size());
    
    // Mark layout as dirty when adding children
    markLayoutDirty();
}

void Container::addCell(Cell *cell) {
    if (!cell || !cell->getWidget()) return;
    cell->getWidget()->setDisplayInterface(_display);
    cell->getWidget()->setParent(this);
    _cells.push_back(cell);
    
    // Mark layout as dirty when adding cells
    markLayoutDirty();
}

void Container::setCellWeight(int index, float weight) {
    if (index >= 0 && index < _cells.size()) {
        _cells[index]->setWeight(weight);
        markLayoutDirty();
    }
}

void Container::setCellGravity(int index, Gravity gravity) {
    if (index >= 0 && index < _cells.size()) {
        _cells[index]->setGravity(gravity);
        markLayoutDirty();
    }
}

void Container::updateLayoutIfNeeded() {
    if (_layoutDirty && !_inLayoutCalculation) {
        SUI_LOGD("updateLayoutIfNeeded: layout is dirty, recalculating");
        recalculateLayout();
        markLayoutClean();
    }
}

void Container::draw() {
    if (!_visible) return;
    
    // Only update layout if this container's layout is dirty
    // Don't recalculate just because child widgets are dirty
    if (_layoutDirty && !_inLayoutCalculation) {
        SUI_LOGD("Container::draw: layout is dirty, recalculating");
        recalculateLayout();
        markLayoutClean();
    }
    
    // Draw all children (they will handle their own dirty state)
    for (auto *cell : _cells) {
        if (cell) cell->draw();
    }
}

void Container::handleTouch(int16_t x, int16_t y, bool pressed) {
    for (auto *cell : _cells) {
        if (cell) cell->handleTouch(x, y, pressed);
    }
}

Cell* Container::addLabel(const String& text, uint16_t color, uint8_t textSize, float weight, Gravity gravity) {
    Label* label = new Label(0, 0, text, color);
    label->setTextSize(textSize);
    Cell* cell = new Cell(label, gravity, weight);
    addCell(cell);
    return cell;
}

Cell* Container::addButton(const String& text, std::function<void()> onClick, 
                          uint16_t bgColor, uint16_t borderColor, uint16_t textColor, float weight, Gravity gravity, uint8_t textSize) {
    // Calculate initial size based on text (with fallback if no display interface)
    int16_t initialWidth = text.length() * WidgetConstants::TEXT_CHAR_WIDTH * textSize + WidgetConstants::BUTTON_PADDING_HORIZONTAL;
    int16_t initialHeight = WidgetConstants::TEXT_BASELINE_HEIGHT * textSize + WidgetConstants::BUTTON_PADDING_VERTICAL;
    
    Button* button = new Button(0, 0, initialWidth, initialHeight, text, textSize);
    button->setColors(bgColor, borderColor, textColor);
    button->setOnClick(onClick);
    Cell* cell = new Cell(button, gravity, weight);
    addCell(cell);
    return cell;
}

Cell* Container::addProgressBar(float progress, float weight, Gravity gravity) {
    ProgressBar* progressBar = new ProgressBar(0, 0, _w - (2 * _margin), WidgetConstants::MIN_PROGRESS_BAR_HEIGHT);
    progressBar->setProgress(progress);
    Cell* cell = new Cell(progressBar, gravity, weight);
    addCell(cell);
    return cell;
}

Cell* Container::addCheckBox(int16_t size, float weight, Gravity gravity) {
    CheckBox* checkBox = new CheckBox(0, 0, size);
    Cell* cell = new Cell(checkBox, gravity, weight);
    addCell(cell);
    return cell;
}

Cell* Container::addSlider(int16_t height, float weight, Gravity gravity) {
    Slider* slider = new Slider(0, 0, _w - (2 * _margin), height);
    Cell* cell = new Cell(slider, gravity, weight);
    addCell(cell);
    return cell;
}

Cell* Container::addGauge(int16_t size, float weight, Gravity gravity) {
    Gauge* gauge = new Gauge(0, 0, size);
    Cell* cell = new Cell(gauge, gravity, weight);
    addCell(cell);
    return cell;
}

Cell* Container::addHorizontalLine(uint16_t color, int16_t thickness, float weight) {
    // Create horizontal line that expands to fill available width
    // Use a large default width that will be constrained by the layout system
    int16_t lineWidth = 1000; // Large width, will be constrained by parent container
    HorizontalLine* line = new HorizontalLine(0, 0, lineWidth, color, thickness);
    Cell* cell = new Cell(line, Gravity::FILL, weight); // Use FILL gravity to expand
    addCell(cell);
    return cell;
}

Cell* Container::addVerticalLayout(float weight, Gravity gravity) {
    VerticalLayout* layout = new VerticalLayout(0, 0, 0, 0);
    Cell* cell = new Cell(layout, gravity, weight);
    addCell(cell);
    return cell;
}

Cell* Container::addHorizontalLayout(float weight, Gravity gravity) {
    HorizontalLayout* layout = new HorizontalLayout(0, 0, 0, 0);
    Cell* cell = new Cell(layout, gravity, weight);
    addCell(cell);
    return cell;
}

Cell* Container::addSpacer(float weight) {
    Spacer* spacer = new Spacer(0, weight);
    Cell* cell = new Cell(spacer, Gravity::CENTER, weight);
    addCell(cell);
    return cell;
}

Cell* Container::addFixedSpacer(int16_t size) {
    Spacer* spacer = new Spacer(size, 0.0f);
    Cell* cell = new Cell(spacer, Gravity::CENTER, 0.0f);
    addCell(cell);
    return cell;
}

Container* Container::getContainerFromWidget(const Widget* widget) {
    if (!widget) return nullptr;
    Widget* parent = widget->getParent();
    if (!parent) return nullptr;
    
    return static_cast<Container*>(parent);
} 