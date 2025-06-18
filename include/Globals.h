#pragma once

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
#include <TFT_eSPI.h>

// High-Level Application & Service Libraries
#include <ArduinoJson.h>
#include <ESP32Time.h>
#include <HTTPClient.h>
#include <UniversalTelegramBot.h>
#include <WiFiManager.h>

// Forward declarations
class CommandHandler;
class DisplayManager;
class ButtonController;
class PowerResetController;
class LedController;
class FanController;
class RFStudyManager;
class RCSwitchReceiver;
class PersistentSettings;

// ESP32 Configuration
static constexpr uint32_t ESP32_CPU_FREQ = 240000000L;  // CPU frequency in Hz
static constexpr uint32_t ESP32_FLASH_FREQ = 80000000L; // Flash frequency in Hz

// Serial Configuration
static constexpr uint32_t SERIAL_BAUD_RATE = 115200;
static constexpr uint32_t SERIAL_CHECK_INTERVAL = 10;  // Check serial every 10ms

// Pin Definitions
static constexpr uint8_t POWER_BUTTON_PIN = 34;
static constexpr uint8_t RESET_BUTTON_PIN = 39;
static constexpr uint8_t POWER_RELAY_PIN = 25;
static constexpr uint8_t RESET_RELAY_PIN = 26;
static constexpr uint8_t PC_POWERED_ON_PIN = 5;
static constexpr uint8_t BUZZER_PIN = 27;
static constexpr uint8_t BUTTON_PIN = SDA;
static constexpr uint8_t LED_PIN = 16;
static constexpr uint8_t GPU_FAN_CONTROL_PIN = 17;
static constexpr uint8_t GPU_FAN_PWM_PIN = 36;
static constexpr uint8_t RF_INPUT_PIN = 35;
static constexpr uint8_t DHT11_PIN = SCL;

// Button beep timing constants
static constexpr uint16_t BUTTON_PRESS_BEEP_DURATION_MS = 250;  // Duration of button press beep
static constexpr uint16_t BUTTON_PRESS_BEEP_INTERVAL_MS = 1000; // Interval between button press beeps

// Display Configuration
#define DISPLAY_WIDTH  240
#define DISPLAY_HEIGHT 240
#define DISPLAY_MISO 12
#define DISPLAY_MOSI 23
#define DISPLAY_SCLK 18
#define DISPLAY_CS -1
#define DISPLAY_DC 2
#define DISPLAY_RST 4
#define DISPLAY_SPI_FREQ 16000000
#define DISPLAY_BACKLIGHT_ON HIGH

// Additional Display Features
#define DISPLAY_DRIVER ST7789_DRIVER
#define DISPLAY_SUPPORT_TRANSACTIONS
#define DISPLAY_SMOOTH_FONT
#define DISPLAY_CGRAM_OFFSET
#define DISPLAY_INVERSION_ON
#define DISPLAY_PARENT_OFFSET

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

// GPU Fan Configuration
static constexpr uint32_t GPU_FAN_PWM_FREQ = 25000;     // PWM frequency in Hz
static constexpr uint8_t GPU_FAN_PWM_RESOLUTION = 8;    // PWM resolution in bits
static constexpr uint8_t GPU_FAN_PULSES_PER_REV = 2;    // Number of pulses per revolution
static constexpr uint32_t GPU_FAN_RPM_UPDATE_INTERVAL = 1000;  // RPM update interval in milliseconds
static constexpr uint16_t GPU_FAN_MIN_RPM = 0;          // Minimum RPM value
static constexpr uint16_t GPU_FAN_MAX_RPM = 5000;       // Maximum RPM value

// Task Configuration
static constexpr uint32_t PERIPHERAL_TASK_STACK_SIZE = 4096;  // Stack size for peripheral task
static constexpr uint8_t PERIPHERAL_TASK_PRIORITY = 1;        // Priority of peripheral task
static constexpr uint8_t PERIPHERAL_TASK_CORE = APP_CPU_NUM;  // Core to run peripheral task on

// Timer Intervals (in milliseconds)
static constexpr uint16_t WIFI_CHECK_INTERVAL = 5000;     // WiFi status check interval
static constexpr uint16_t DEBUG_OUTPUT_INTERVAL = 1000;   // Debug output interval
static constexpr uint16_t DISPLAY_UPDATE_INTERVAL = 100;  // Display refresh interval
static constexpr uint16_t RF_CHECK_INTERVAL = 50;         // RF receiver check interval
static constexpr uint16_t RELAY_TIMER_INTERVAL = 2000;     // Relay press duration
static constexpr uint16_t MESSAGE_CHECK_INTERVAL = 300;   // Telegram message check interval (faster polling)
static constexpr uint16_t RF_REPEAT_DELAY = 300;         // Minimum delay between RF code repeats
static constexpr uint32_t WIFI_CONFIG_TIMEOUT = 180000;   // WiFi config portal timeout (3 minutes)
static constexpr uint16_t WIFI_CONNECT_TIMEOUT = 20000;   // WiFi connection timeout (20 seconds)
static constexpr uint16_t TELEGRAM_TIMEOUT = 4000;        // Telegram client timeout (4 seconds)
static constexpr uint16_t READ_TIMEOUT_MS = 3000;         // Timeout for reading command responses (3 second)
static constexpr size_t TELEGRAM_MAX_MESSAGE = 4096;      // Maximum length of a Telegram message
static constexpr uint16_t DISPLAY_INIT_DELAY_MS = 50;     // Delay after display initialization

// Button Press Durations (in milliseconds)
static constexpr uint16_t SHORT_PRESS_DURATION = 50;    // Minimum duration for a valid press
static constexpr uint16_t LONG_PRESS_DURATION = 3000;   // 3 second for long press
static constexpr uint16_t VERY_LONG_PRESS_DURATION = 10000; // 10 seconds for very long press
static constexpr uint16_t DEBOUNCE_TIME = 50;           // Debounce time in milliseconds

// LED timing constants
static constexpr uint16_t LED_CONNECTING_BLINK_ON_MS = 500;   // LED on duration during connecting state
static constexpr uint16_t LED_CONNECTING_BLINK_OFF_MS = 500;  // LED off duration during connecting state

// Persistent Storage Configuration
static constexpr const char* NVS_NAMESPACE = "compCtrl";      // Namespace for NVS storage
static constexpr const char* NVS_KEY_CHILD_LOCK = "child_lock"; // Key for storing child lock state
static constexpr const char* NVS_KEY_BUZZER_ENABLED = "buzzer_enabled"; // Key for storing buzzer state
static constexpr const char* NVS_KEY_RF_ENABLED = "rf_enabled";
static constexpr const char* NVS_KEY_RF_BUTTON_CODE = "rf_button_code"; // Key for storing RF button code

// WiFi Configuration
static constexpr const char* WIFI_AP_NAME = "ComputerController";
static constexpr const char* WIFI_AP_PASSWORD = "12345678";  // 8 character minimum for WPA2
static constexpr uint16_t WIFI_CONFIG_PORTAL_TIMEOUT = 180;  // 3 minute timeout for configuration
static constexpr uint16_t WIFI_CONNECT_TIMEOUT_SECONDS = 20; // 20 second connection timeout

// Buzzer Configuration
static constexpr uint16_t BUZZER_BEEP_DURATION_MS = 100;     // Duration of a single beep
static constexpr uint16_t BUZZER_PATTERN_INTERVAL_MS = 250;  // Interval between beeps in a pattern
static constexpr uint16_t BUZZER_FACTORY_RESET_BEEPS = 3;    // Number of beeps for factory reset
static constexpr uint16_t BUZZER_FACTORY_RESET_DURATION_MS = 200;  // Duration of factory reset beeps
static constexpr uint16_t BUZZER_FACTORY_RESET_INTERVAL_MS = 100;  // Interval between factory reset beeps

// Display Text Configuration
static constexpr uint8_t DISPLAY_TEXT_SIZE_SMALL = 2;
static constexpr uint8_t DISPLAY_TEXT_SIZE_LARGE = 3;
static constexpr uint16_t DISPLAY_CURSOR_X = 10;
static constexpr uint16_t DISPLAY_CURSOR_Y_START = 10;
static constexpr uint16_t DISPLAY_LINE_SPACING = 25;

// RF Configuration
static constexpr uint8_t RF_RECEIVE_TOLERANCE = 80;      // Tolerance percentage for RF signal reception

// Utility Functions
inline void usDelay(uint32_t us) {
    delayMicroseconds(us);
}

// Compile-time software version string (updates automatically on every build)
inline constexpr char SOFTWARE_VERSION[] = __DATE__ " " __TIME__;

// Device identification
inline constexpr char DEVICE_NAME[] = "ComputerController";
