#pragma once

#include <TFT_eSPI.h>
#include "Globals.h"

class DisplayManager : public Traceable {
public:
    DisplayManager();
    void begin();
    void update();
    void showMessage(const char* message);
    void showError(const char* error);
    void showSuccess(const char* message);
    void clear();
    void setTextSize(uint8_t size);
    void setTextColor(uint16_t color);
    void setCursor(int16_t x, int16_t y);
    void print(const char* text);
    void println(const char* text);
    void drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
    void fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
    void drawCircle(int16_t x, int16_t y, int16_t r, uint16_t color);
    void fillCircle(int16_t x, int16_t y, int16_t r, uint16_t color);
    void drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color);
    void drawPixel(int16_t x, int16_t y, uint16_t color);
    void display();

private:
    TFT_eSPI tft;
}; 