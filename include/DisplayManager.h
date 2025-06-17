#pragma once

#include "Globals.h"

/**
 * @brief Manages the display output using the TFT_eSPI library.
 * 
 * This class provides a simplified interface for common display operations
 * such as showing messages, drawing shapes, and controlling text properties.
 */
class DisplayManager {
public:
    /**
     * @brief Constructor for DisplayManager.
     * Initializes the underlying TFT_eSPI object.
     */
    DisplayManager();

    /**
     * @brief Initializes the display.
     * Must be called once during setup.
     */
    void begin();

    /**
     * @brief Updates the display content.
     * (Currently a placeholder, could be used for animations or periodic updates).
     */
    void update();

    /**
     * @brief Shows a general message on the display.
     * @param message The message string to display.
     */
    void showMessage(const char* message);

    /**
     * @brief Shows an error message on the display, typically with a distinct color.
     * @param error The error message string to display.
     */
    void showError(const char* error);

    /**
     * @brief Shows a success message on the display, typically with a distinct color.
     * @param message The success message string to display.
     */
    void showSuccess(const char* message);

    /**
     * @brief Clears the entire display.
     */
    void clear();

    /**
     * @brief Sets the text size for subsequent print operations.
     * @param size The text size (1 is smallest).
     */
    void setTextSize(uint8_t size);

    /**
     * @brief Sets the text color for subsequent print operations.
     * @param color The color in RGB565 format.
     */
    void setTextColor(uint16_t color);

    /**
     * @brief Sets the cursor position for subsequent print operations.
     * @param x The x-coordinate.
     * @param y The y-coordinate.
     */
    void setCursor(int16_t x, int16_t y);

    /**
     * @brief Prints text to the display at the current cursor position.
     * @param text The text string to print.
     */
    void print(const char* text);

    /**
     * @brief Prints text to the display followed by a newline.
     * @param text The text string to print.
     */
    void println(const char* text);

    // --- Drawing functions ---
    void drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
    void fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
    void drawCircle(int16_t x, int16_t y, int16_t r, uint16_t color);
    void fillCircle(int16_t x, int16_t y, int16_t r, uint16_t color);
    void drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color);
    void drawPixel(int16_t x, int16_t y, uint16_t color);

    /**
     * @brief Pushes the current framebuffer to the display.
     * (Note: TFT_eSPI usually draws directly, this might be for sprite-based rendering if used).
     * If not using sprites, this method might be redundant or could be removed.
     */
    void display();

private:
    TFT_eSPI tft; ///< Instance of the TFT_eSPI library.
}; 