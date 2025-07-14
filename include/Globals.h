#pragma once

#include "Credentials.h"  // Include credentials first

// Standard C/C++ Library Headers
#include <stdint.h>

// Arduino Core & RTOS Headers
#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <freertos/task.h>

// ESP32 Core Libraries
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <esp_task_wdt.h>
#include <Preferences.h>

// Sensor Libraries
#include <DHT11Sensor.h>

// Common Utility & Helper Libraries
#include <LoopbackStream.h>
#include <PipedStream.h>
#include <SimpleBuzzer.h>
#include <SimpleTimer.h>
#include <StaticSerialCommands.h>
#include <ThreadSafeSerial.h>
#include <Utilities.h>
#include <RCSwitch.h>
#include <ezButton.h>

// Hardware Abstraction / Driver Libraries
// ST7796Driver - Custom TFT display driver for ST7796 displays

// High-Level Application & Service Libraries
#include <ArduinoJson.h>
#include <ESP32Time.h>
#include <HTTPClient.h>
#include <UniversalTelegramBot.h>
#include <WiFiManager.h>
#include <PubSubClient.h>
#include <NTPClient.h>

// Forward declarations
class CommandHandler;
class ButtonController;
class PowerResetController;
class LedController;
class FanController;
class RFStudyManager;
class RCSwitchReceiver;
class PersistentSettings;

// ESP32 Configuration
#define ESP32_CPU_FREQ 240000000L  // CPU frequency in Hz
#define ESP32_FLASH_FREQ 80000000L // Flash frequency in Hz

// Serial Configuration
#define SERIAL_BAUD_RATE 115200
#define SERIAL_CHECK_INTERVAL 10  // Check serial every 10ms

// Pin Definitions
#define POWER_BUTTON_PIN 34
#define RESET_BUTTON_PIN 39
#define POWER_RELAY_PIN 25
#define RESET_RELAY_PIN 26
#define PC_POWERED_ON_PIN 5
#define BUZZER_PIN 27
#define BUTTON_PIN SDA
#define LED_PIN 16
#define GPU_FAN_CONTROL_PIN 17
#define GPU_FAN_PWM_PIN 36
#define RF_INPUT_PIN 35
#define DHT11_PIN SCL

// Button beep timing constants
#define BUTTON_PRESS_BEEP_DURATION_MS 250  // Duration of button press beep
#define BUTTON_PRESS_BEEP_INTERVAL_MS 1000 // Interval between button press beeps

// Display Configuration
#define DISPLAY_CS 13
#define DISPLAY_DC 2
#define DISPLAY_RST 4
#define DISPLAY_MOSI 23
#define DISPLAY_SCLK 18
#define DISPLAY_SPI_FREQ 40000000  // Increased from 1MHz to 40MHz for much faster display updates
#define DISPLAY_BACKLIGHT_ON HIGH
#define DISPLAY_START_ROTATION 3 // default orientation 0-7

// Additional Display Features
// #define DISPLAY_DRIVER ST7796_DRIVER
// #define DISPLAY_SUPPORT_TRANSACTIONS
// #define DISPLAY_SMOOTH_FONT
// #define DISPLAY_INVERSION_ON

// GPU Fan Configuration
#define GPU_FAN_PWM_FREQ 25000     // PWM frequency in Hz
#define GPU_FAN_PWM_RESOLUTION 8    // PWM resolution in bits
#define GPU_FAN_PULSES_PER_REV 2    // Number of pulses per revolution
#define GPU_FAN_RPM_UPDATE_INTERVAL 1000  // RPM update interval in milliseconds
#define GPU_FAN_MIN_RPM 0          // Minimum RPM value
#define GPU_FAN_MAX_RPM 5000       // Maximum RPM value

// Task Configuration
#define PERIPHERAL_TASK_STACK_SIZE 4096  // Stack size for peripheral task
#define PERIPHERAL_TASK_PRIORITY 1        // Priority of peripheral task
#define PERIPHERAL_TASK_CORE APP_CPU_NUM  // Core to run peripheral task on

// Timer Intervals (in milliseconds)
#define WIFI_CHECK_INTERVAL 5000     // WiFi status check interval
#define DEBUG_OUTPUT_INTERVAL 1000   // Debug output interval
#define DISPLAY_UPDATE_INTERVAL 100  // Display refresh interval
#define RF_CHECK_INTERVAL 50         // RF receiver check interval
#define RELAY_TIMER_INTERVAL 2000     // Relay press duration
#define MESSAGE_CHECK_INTERVAL 300   // Telegram message check interval (faster polling)
#define RF_REPEAT_DELAY 300         // Minimum delay between RF code repeats
#define WIFI_CONFIG_TIMEOUT 180000   // WiFi config portal timeout (3 minutes)
#define WIFI_CONNECT_TIMEOUT 20000   // WiFi connection timeout (20 seconds)
#define TELEGRAM_TIMEOUT 4000        // Telegram client timeout (4 seconds)
#define READ_TIMEOUT_MS 3000         // Timeout for reading command responses (3 second)
#define TELEGRAM_MAX_MESSAGE 4096      // Maximum length of a Telegram message
#define DISPLAY_INIT_DELAY_MS 50     // Delay after display initialization

// Button Press Durations (in milliseconds)
#define SHORT_PRESS_DURATION 50    // Minimum duration for a valid press
#define LONG_PRESS_DURATION 3000   // 3 second for long press
#define VERY_LONG_PRESS_DURATION 10000 // 10 seconds for very long press
#define DEBOUNCE_TIME 50           // Debounce time in milliseconds

// LED timing constants
#define LED_CONNECTING_BLINK_ON_MS 500   // LED on duration during connecting state
#define LED_CONNECTING_BLINK_OFF_MS 500  // LED off duration during connecting state

// Persistent Storage Configuration
#define NVS_NAMESPACE "compCtrl"      // Namespace for NVS storage
#define NVS_KEY_CHILD_LOCK "child_lock" // Key for storing child lock state
#define NVS_KEY_BUZZER_ENABLED "buzzer_enabled" // Key for storing buzzer state
#define NVS_KEY_RF_ENABLED "rf_enabled"
#define NVS_KEY_RF_BUTTON_CODE "rf_button_code" // Key for storing RF button code

// WiFi Configuration
#define WIFI_AP_NAME "ComputerController"
#define WIFI_AP_PASSWORD "12345678"  // 8 character minimum for WPA2
#define WIFI_CONFIG_PORTAL_TIMEOUT 180  // 3 minute timeout for configuration
#define WIFI_CONNECT_TIMEOUT_SECONDS 20 // 20 second connection timeout

// MQTT Configuration
#define MQTT_BROKER MQTT_BROKER_HOST  // From Credentials.h
#define MQTT_PORT 8883  // TLS port for HiveMQ Cloud
#define MQTT_CLIENT_ID "ComputerController_ESP32"
#define MQTT_USERNAME_CRED MQTT_USERNAME  // From Credentials.h
#define MQTT_PASSWORD_CRED MQTT_PASSWORD  // From Credentials.h
#define MQTT_KEEPALIVE_SECONDS 60  // Renamed to avoid conflict with PubSubClient
#define MQTT_STATUS_INTERVAL 5000  // Publish status every 5 seconds

// MQTT Topics
#define MQTT_TOPIC_STATUS "computer-controller/status"
#define MQTT_TOPIC_CONTROL "computer-controller/control"
#define MQTT_TOPIC_SETTINGS "computer-controller/settings"
#define MQTT_TOPIC_EVENTS "computer-controller/events"

// NTP Configuration for Istanbul Time
#define NTP_SERVER_1 "pool.ntp.org"
#define NTP_SERVER_2 "time.nist.gov"
#define NTP_SERVER_3 "time.google.com"
#define NTP_GMT_OFFSET_SEC (3 * 3600)  // Istanbul is UTC+3
#define NTP_DAYLIGHT_OFFSET_SEC 0    // No DST in Istanbul
#define NTP_UPDATE_INTERVAL 3600000 // Update time every hour
#define NTP_TIMEOUT 10000           // 10 second timeout for NTP sync

// Buzzer Configuration
#define BUZZER_BEEP_DURATION_MS 100     // Duration of a single beep
#define BUZZER_PATTERN_INTERVAL_MS 250  // Interval between beeps in a pattern
#define BUZZER_FACTORY_RESET_BEEPS 3    // Number of beeps for factory reset
#define BUZZER_FACTORY_RESET_DURATION_MS 200  // Duration of factory reset beeps
#define BUZZER_FACTORY_RESET_INTERVAL_MS 100  // Interval between factory reset beeps

// Display Text Configuration
#define DISPLAY_TEXT_SIZE_SMALL 1
#define DISPLAY_TEXT_SIZE_MEDIUM 2
#define DISPLAY_TEXT_SIZE_LARGE 3
#define DISPLAY_CURSOR_X 20
#define DISPLAY_CURSOR_Y_START 20
// Vertical distance between consecutive text lines on the TFT screen. 40 px
// looked too loose with the current 12-point FreeSans font, so shrink to 24 px
// for a denser layout.
#define DISPLAY_LINE_SPACING 24

// RF Configuration
#define RF_RECEIVE_TOLERANCE 85      // Increased to 85 for maximum sensitivity
#define RF_MIN_SIGNAL_STRENGTH 1    // Keep at 1 for immediate detection
#define RF_SIGNAL_VALIDATION_COUNT 1 // Keep at 1 for immediate response
#define RF_NOISE_FILTER_DELAY 10    // Reduced to 10ms for maximum responsiveness
#define RF_SIGNAL_TIMEOUT 15000     // Increased timeout for better reliability

// Display Font Metrics (for built-in 5x7 font)
#define FONT_BASE_CHAR_W 6   // 5 px glyph + 1 px spacing
#define FONT_BASE_CHAR_H 8   // 7 px glyph + 1 px spacing

// =====================================================================
// COLOR DEFINITIONS (RGB565 format)
// =====================================================================

// Basic Colors
#define COLOR_BLACK 0x0000
#define COLOR_WHITE 0xFFFF
#define COLOR_RED 0xF800
#define COLOR_GREEN 0x07E0
#define COLOR_BLUE 0x001F
#define COLOR_YELLOW 0xFFE0
#define COLOR_MAGENTA 0xF81F
#define COLOR_CYAN 0x07FF
#define COLOR_ORANGE 0xFD20
#define COLOR_PURPLE 0x8010
#define COLOR_PINK 0xFC18
#define COLOR_BROWN 0xA145
#define COLOR_GRAY 0x8410
#define COLOR_LIGHT_GRAY 0xC618
#define COLOR_DARK_GRAY 0x4208

// Material Design Colors
#define COLOR_MATERIAL_RED 0xF800           // Red 500
#define COLOR_MATERIAL_PINK 0xF81F          // Pink 500
#define COLOR_MATERIAL_PURPLE 0x8010        // Purple 500
#define COLOR_MATERIAL_DEEP_PURPLE 0x6010   // Deep Purple 500
#define COLOR_MATERIAL_INDIGO 0x4010        // Indigo 500
#define COLOR_MATERIAL_BLUE 0x001F          // Blue 500
#define COLOR_MATERIAL_LIGHT_BLUE 0x049F    // Light Blue 500
#define COLOR_MATERIAL_CYAN 0x07FF          // Cyan 500
#define COLOR_MATERIAL_TEAL 0x07E0          // Teal 500
#define COLOR_MATERIAL_GREEN 0x07E0         // Green 500
#define COLOR_MATERIAL_LIGHT_GREEN 0x87E0   // Light Green 500
#define COLOR_MATERIAL_LIME 0x87E0          // Lime 500
#define COLOR_MATERIAL_YELLOW 0xFFE0        // Yellow 500

// UI Theme Colors
#define COLOR_BACKGROUND 0x0000             // Black background
#define COLOR_TEXT_PRIMARY 0xFFFF           // White text
#define COLOR_TEXT_SECONDARY 0xFFE0         // Yellow text
#define COLOR_TEXT_SUCCESS 0x07E0           // Green text
#define COLOR_TEXT_ERROR 0xF800             // Red text
#define COLOR_TEXT_WARNING 0xFFE0           // Yellow text
#define COLOR_BORDER 0xFFFF                 // White border
#define COLOR_BUTTON_PRIMARY 0x07E0         // Green button
#define COLOR_BUTTON_SECONDARY 0xFFE0       // Yellow button
#define COLOR_BUTTON_DANGER 0xF800          // Red button
#define COLOR_BUTTON_BORDER 0x0000          // Black button border
#define COLOR_BUTTON_TEXT 0x0000            // Black button text

// Header bar colors
#define COLOR_HEADER_BG 0x4208              // Dark gray (RGB565)
#define COLOR_HEADER_FG COLOR_TEXT_PRIMARY  // Use primary text color for header text

// Status Colors
#define COLOR_STATUS_ONLINE 0x07E0          // Green for online status
#define COLOR_STATUS_OFFLINE 0xF800         // Red for offline status
#define COLOR_STATUS_CONNECTING 0xFFE0      // Yellow for connecting status
#define COLOR_STATUS_WARNING 0xFFE0         // Yellow for warning status
#define COLOR_STATUS_ERROR 0xF800           // Red for error status

// =====================================================================
// TEXT SIZE DEFINITIONS
// =====================================================================

// Standard Text Sizes (already defined above)
// #define DISPLAY_TEXT_SIZE_SMALL 1
// #define DISPLAY_TEXT_SIZE_MEDIUM 2  
// #define DISPLAY_TEXT_SIZE_LARGE 3

// UI Text Sizes
#define TEXT_SIZE_TITLE 3                   // Large title text
#define TEXT_SIZE_HEADER 2                  // Medium header text
#define TEXT_SIZE_BODY 1                    // Small body text
#define TEXT_SIZE_BUTTON 2                  // Button text size
#define TEXT_SIZE_STATUS 1                  // Status text size
#define TEXT_SIZE_LABEL 1                   // Label text size
#define TEXT_SIZE_VALUE 1                   // Value text size

// =====================================================================
// UI LAYOUT CONSTANTS
// =====================================================================

// Margins and Spacing
#define UI_MARGIN_SMALL 5                   // Small margin
#define UI_MARGIN_MEDIUM 10                 // Medium margin
#define UI_MARGIN_LARGE 20                  // Large margin
#define UI_SPACING_SMALL 5                  // Small spacing
#define UI_SPACING_MEDIUM 10                // Medium spacing
#define UI_SPACING_LARGE 20                 // Large spacing

// Button Dimensions
#define BUTTON_HEIGHT_SMALL 30              // Small button height
#define BUTTON_HEIGHT_MEDIUM 40             // Medium button height
#define BUTTON_HEIGHT_LARGE 50              // Large button height
#define BUTTON_WIDTH_SMALL 80               // Small button width
#define BUTTON_WIDTH_MEDIUM 100             // Medium button width
#define BUTTON_WIDTH_LARGE 120              // Large button width
#define BUTTON_WIDTH_FULL 300               // Full button width

// Label Dimensions
#define LABEL_HEIGHT_SMALL 15               // Small label height
#define LABEL_HEIGHT_MEDIUM 20              // Medium label height
#define LABEL_HEIGHT_LARGE 25               // Large label height

// Header and Title Bar Dimensions
#define TITLE_BAR_HEIGHT 40                 // Height of title bar

// Progress Bar Dimensions
#define PROGRESS_BAR_HEIGHT 20              // Progress bar height
#define PROGRESS_BAR_WIDTH 300              // Progress bar width

// =====================================================================
// MESSAGE DURATIONS
// =====================================================================

#define MESSAGE_DURATION_SHORT 1000         // Short message duration (1 second)
#define MESSAGE_DURATION_MEDIUM 3000        // Medium message duration (3 seconds)
#define MESSAGE_DURATION_LONG 5000          // Long message duration (5 seconds)

// Utility Functions
inline void usDelay(uint32_t us) {
    delayMicroseconds(us);
}

// Compile-time software version string (updates automatically on every build)
const char SOFTWARE_VERSION[] = __DATE__ " " __TIME__;

// Device identification
const char DEVICE_NAME[] = "ComputerController";

// Display Section Divider
#define DISPLAY_SECTION_DIVIDER_X (DISPLAY_WIDTH / 2) // Mid-screen x coordinate for two-column layout
