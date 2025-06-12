#pragma once

// Standard C/C++ Library Headers
#include <stdint.h>

// Arduino Core & RTOS Headers
#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <freertos/task.h>

// ESP32 Core Libraries
#include <SPI.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <esp_task_wdt.h>

// Common Utility & Helper Libraries
#include <LoopbackStream.h>
#include <PipedStream.h>
#include <SimpleBuzzer.h>
#include <SimpleTimer.h>
#include <StaticSerialCommands.h>
#include <ThreadSafeSerial.h>
#include <Utilities.h>

// Hardware Abstraction / Driver Libraries
#include <TFT_eSPI.h>

// High-Level Application & Service Libraries
#include <ArduinoJson.h>
#include <ESP32Time.h>
#include <HTTPClient.h>
#include <UniversalTelegramBot.h>
#include <WiFiManager.h>

// Telegram Bot Configuration
#define BOT_TOKEN "7108147347:AAE30iRARSYCjJa4lX7noKd_OHUDDZ-VBYI"
#define CHAT_ID "5889803417"

// Serial Configuration
static constexpr uint32_t SERIAL_BAUD_RATE = 115200;

// Pin Definitions
static constexpr uint8_t POWER_BUTTON_PIN = 34;
static constexpr uint8_t RESET_BUTTON_PIN = 39;
static constexpr uint8_t POWER_RELAY_PIN = 25;
static constexpr uint8_t RESET_RELAY_PIN = 26;
static constexpr uint8_t BUZZER_PIN = 27;
static constexpr uint8_t BUTTON_PIN = 14;
static constexpr uint8_t LED_PIN = 16;
static constexpr uint8_t GPU_FAN_CONTROL_PIN = 17;
static constexpr uint8_t GPU_FAN_PWM_INPUT = 36;
static constexpr uint8_t RF_INPUT_PIN = 35;

// Display Resolution
#define DISPLAY_WIDTH  240
#define DISPLAY_HEIGHT 240

// Display Colors (RGB565)
namespace DisplayColors {
    static constexpr uint16_t BLACK   = 0x0000;
    static constexpr uint16_t WHITE   = 0xFFFF;
    static constexpr uint16_t RED     = 0xF800;
    static constexpr uint16_t GREEN   = 0x07E0;
    static constexpr uint16_t BLUE    = 0x001F;
    static constexpr uint16_t YELLOW  = RED | GREEN;
    static constexpr uint16_t CYAN    = GREEN | BLUE;
    static constexpr uint16_t MAGENTA = RED | BLUE;
}

// Constants
static constexpr uint16_t MESSAGE_CHECK_INTERVAL = 1000;
static constexpr uint16_t RELAY_BUTTON_PRESS_DURATION = 500;

// Button Press Durations (in milliseconds)
static constexpr uint16_t SHORT_PRESS_DURATION = 50;    // Minimum duration for a valid press
static constexpr uint16_t LONG_PRESS_DURATION = 1000;   // 1 second for long press
static constexpr uint16_t VERY_LONG_PRESS_DURATION = 3000; // 3 seconds for very long press
static constexpr uint16_t DEBOUNCE_TIME = 50;           // Debounce time in milliseconds

// Utility Functions
inline void usDelay(uint32_t us) {
    delayMicroseconds(us);
}
