#pragma once

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>
#include <StaticSerialCommands.h>
#include <SimpleTimer.h>
#include <Traceable.h>
#include <TraceHelper.h>
#include <TraceLevel.h>
#include <TFT_eSPI.h>

// Telegram Bot Configuration
#define BOT_TOKEN "7108147347:AAE30iRARSYCjJa4lX7noKd_OHUDDZ-VBYI"
#define CHAT_ID "5889803417"

// Pin Definitions
#define POWER_RELAY_PIN 34 // GPIO pin for power button relay
#define RESET_RELAY_PIN 35 // GPIO pin for reset button relay

// Display Configuration
#define ST7789_DRIVER

// Display Pin Definitions
#define TFT_MISO 19
#define TFT_MOSI 23
#define TFT_SCLK 18
#define TFT_CS   -1  // Not connected
#define TFT_DC   2   // Data/Command pin
#define TFT_RST  4   // Reset pin
#define TFT_BL   5   // Backlight pin

// Display Resolution
#define DISPLAY_WIDTH  240
#define DISPLAY_HEIGHT 240

// Display Settings
#define SPI_FREQUENCY  10000000
#define SPI_READ_FREQUENCY  10000000
#define SPI_TOUCH_FREQUENCY  2500000
#define SUPPORT_TRANSACTIONS
#define SMOOTH_FONT
#define CGRAM_OFFSET
#define TFT_INVERSION_ON
#define TFT_PARENT_OFFSET
#define TFT_BACKLIGHT_ON HIGH

// Display Colors
#define DISPLAY_BLACK 0x0000
#define DISPLAY_WHITE 0xFFFF
#define DISPLAY_RED   0xF800
#define DISPLAY_GREEN 0x07E0
#define DISPLAY_BLUE  0x001F
#define DISPLAY_YELLOW (DISPLAY_RED | DISPLAY_GREEN)
#define DISPLAY_CYAN   (DISPLAY_GREEN | DISPLAY_BLUE)
#define DISPLAY_MAGENTA (DISPLAY_RED | DISPLAY_BLUE)

// Constants
static constexpr uint16_t MESSAGE_CHECK_INTERVAL = 1000;
static constexpr uint16_t RELAY_BUTTON_PRESS_DURATION = 500;
