#include "DisplayManager.h"

DisplayManager::DisplayManager() : tft() {
    // Constructor can be empty if TFT_eSPI constructor does all init
}

void DisplayManager::begin() {
    tft.init();
    tft.setRotation(0); // Set default rotation
    tft.fillScreen(DisplayColors::BLACK);
    tft.setTextColor(DisplayColors::WHITE, DisplayColors::BLACK);
    tft.setTextSize(1);
    tft.setCursor(0, 0);
}

void DisplayManager::update() {
    // Placeholder for future use (e.g., animations, periodic updates)
}

void DisplayManager::showMessage(const char* message) {
    tft.fillScreen(DisplayColors::BLACK);
    tft.setCursor(0, 0);
    tft.setTextColor(DisplayColors::WHITE);
    tft.setTextSize(2);
    tft.println(message);
}

void DisplayManager::showError(const char* error) {
    tft.fillScreen(DisplayColors::BLACK);
    tft.setCursor(0, 0);
    tft.setTextColor(DisplayColors::RED);
    tft.setTextSize(2);
    tft.println("Error:");
    tft.setTextSize(1);
    tft.println(error);
}

void DisplayManager::showSuccess(const char* message) {
    tft.fillScreen(DisplayColors::BLACK);
    tft.setCursor(0, 0);
    tft.setTextColor(DisplayColors::GREEN);
    tft.setTextSize(2);
    tft.println(message);
}

void DisplayManager::clear() {
    tft.fillScreen(DisplayColors::BLACK);
    tft.setCursor(0,0); // Reset cursor after clearing
}

void DisplayManager::setTextSize(uint8_t size) {
    tft.setTextSize(size);
}

void DisplayManager::setTextColor(uint16_t color) {
    tft.setTextColor(color);
}

void DisplayManager::setCursor(int16_t x, int16_t y) {
    tft.setCursor(x, y);
}

void DisplayManager::print(const char* text) {
    tft.print(text);
}

void DisplayManager::println(const char* text) {
    tft.println(text);
}

void DisplayManager::drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) { tft.drawRect(x, y, w, h, color); }
void DisplayManager::fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) { tft.fillRect(x, y, w, h, color); }
void DisplayManager::drawCircle(int16_t x, int16_t y, int16_t r, uint16_t color) { tft.drawCircle(x, y, r, color); }
void DisplayManager::fillCircle(int16_t x, int16_t y, int16_t r, uint16_t color) { tft.fillCircle(x, y, r, color); }
void DisplayManager::drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color) { tft.drawLine(x0, y0, x1, y1, color); }
void DisplayManager::drawPixel(int16_t x, int16_t y, uint16_t color) { tft.drawPixel(x, y, color); }

void DisplayManager::display() {
    // For TFT_eSPI, drawing is usually direct.
    // If using sprites (TFT_eSprite), this is where you'd push the sprite:
    // tft.pushSprite(x, y);
    // For now, this can be a no-op or removed if not using sprites.
}