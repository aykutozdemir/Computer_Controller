#include "ComputerController.h"
#include "ButtonController.h"
#include "Globals.h"
#include "CommandHandler.h"
#include <SimpleTimer.h>
#include "esp_log.h"

// ESP32 core includes
#include <WiFi.h>

// Third-party library includes
#include <WiFiManager.h>
#include <StaticSerialCommands.h>
#include "esp_task_wdt.h"

// Define log tag
static const char* TAG = "ComputerController";

ComputerController::ComputerController() :
    telegramBot(BOT_TOKEN, telegramClient),
    wifiCheckTimer(10000),
    debugTimer(10000),
    displayUpdateTimer(DISPLAY_UPDATE_INTERVAL),
    button(14),  // Initialize button on pin 14
    isConnected(false),
    lastTelegramCheck(0)
{
    ESP_LOGI(TAG, "Initializing ComputerController");
    commandHandler = new CommandHandler(this);
}

void ComputerController::reset()
{
    ESP_LOGI(TAG, "Performing full ESP32 reset...");

    // Disconnect WiFi, clear settings, and turn off WiFi radio
    WiFi.disconnect(true, true); // disconnect, erase credentials
    WiFi.mode(WIFI_OFF);
    delay(1000);

    ESP_LOGI(TAG, "Restarting ESP32...");
    Serial.flush();
    ESP.restart();
}

void ComputerController::setup()
{
    // Initialize Serial for debugging first
    Serial.begin(115200);
    while (!Serial) {
        delay(10);
    }
    
    // Set ESP32 log level to show all logs
    esp_log_level_set("*", ESP_LOG_VERBOSE);  // Set all tags to verbose level
    esp_log_level_set("ComputerController", ESP_LOG_VERBOSE);
    esp_log_level_set("CommandHandler", ESP_LOG_VERBOSE);
    
    ESP_LOGI(TAG, "Setting up ComputerController");
    
    // Initialize watchdog timer with a longer timeout and disable panic handler
    esp_task_wdt_init(60, false); // 60 second timeout, no panic handler
    esp_task_wdt_add(NULL);
    
    // Initialize display with error checking
    try {
        display.begin();
        display.clear();
        display.setTextSize(2);
        display.setTextColor(DISPLAY_WHITE);
        display.setCursor(10, 10);
        display.println("Starting...");
        ESP_LOGI(TAG, "Display initialized successfully");
    } catch (...) {
        ESP_LOGE(TAG, "Display initialization failed!");
    }
    
    // Add delay after display initialization
    delay(100);
    
    // Initialize button
    button.begin();
    ESP_LOGI(TAG, "Button initialized");
    
    // Initialize WiFi
    connectWiFi();
    
    // Initialize RTC
    rtc.setTime(0);
    ESP_LOGI(TAG, "RTC initialized");
    
    // Initialize CommandHandler
    if (commandHandler) {
        commandHandler->setup();
        ESP_LOGI(TAG, "CommandHandler initialized");
    } else {
        ESP_LOGE(TAG, "CommandHandler is null!");
    }
    
    // Initial display update
    updateDisplay();
    
    // Feed the watchdog
    esp_task_wdt_reset();
}

void ComputerController::loop()
{
    // Feed watchdog at the start of each loop
    esp_task_wdt_reset();
    
    // Update button state and check for presses
    button.loop();
    
    // Check button state and trace
    switch (button.state()) {
        case ButtonController::State::NO_PRESS:
            // No press detected, no need to trace
            break;
            
        case ButtonController::State::SHORT_PRESS:
            ESP_LOGI(TAG, "Button: Short press detected");
            break;
            
        case ButtonController::State::LONG_PRESS:
            ESP_LOGI(TAG, "Button: Long press detected");
            break;
            
        case ButtonController::State::VERY_LONG_PRESS:
            ESP_LOGI(TAG, "Button: Very long press detected");
            break;
    }
    
    // Update display using SimpleTimer
    if (displayUpdateTimer.isReady()) {
        displayUpdateTimer.reset();
        updateDisplay();
        esp_task_wdt_reset(); // Feed watchdog after display update
    }
}

void ComputerController::updateDisplay()
{
    ESP_LOGI(TAG, "Updating display");
    display.clear();
    
    // Display WiFi status
    display.setTextSize(2);
    display.setTextColor(isConnected ? DISPLAY_GREEN : DISPLAY_YELLOW);
    display.setCursor(10, 10);
    display.print("WiFi: ");
    display.println(isConnected ? "Connected" : "Config Mode");
    
    if (isConnected) {
        // Display time
        display.setTextSize(2);
        display.setTextColor(DISPLAY_WHITE);
        display.setCursor(10, 45);
        display.print("Time: ");
        display.println(rtc.getTime().c_str());
        
        // Display IP address
        display.setCursor(10, 70);
        display.print("IP: ");
        display.println(WiFi.localIP().toString().c_str());
        
        // Display RSSI (signal strength)
        char rssiStr[10];
        snprintf(rssiStr, sizeof(rssiStr), "%d dBm", WiFi.RSSI());
        display.setCursor(10, 95);
        display.print("Signal: ");
        display.println(rssiStr);
        
        // Display uptime
        char uptimeStr[20];
        snprintf(uptimeStr, sizeof(uptimeStr), "%lu s", millis() / 1000);
        display.setCursor(10, 120);
        display.print("Uptime: ");
        display.println(uptimeStr);
    } else {
        // Show configuration instructions
        display.setTextSize(2);
        display.setTextColor(DISPLAY_WHITE);
        display.setCursor(10, 45);
        display.println("Connect to:");
        display.setCursor(10, 70);
        display.println("SSID: ComputerController");
        display.setCursor(10, 95);
        display.println("Password: 12345678");
        display.setCursor(10, 120);
        display.println("Then open:");
        display.setCursor(10, 145);
        display.println("http://192.168.4.1");
    }
}

void ComputerController::connectWiFi()
{
    ESP_LOGI(TAG, "Connecting to WiFi...");
    
    // Clear display and show initial message
    display.clear();
    display.setTextSize(3);
    display.setTextColor(DISPLAY_WHITE);
    display.setCursor(10, 10);
    display.println("WiFi Setup");
    display.setTextSize(2);
    display.setCursor(10, 45);
    display.println("Connecting to WiFi...");
    
    // Configure WiFiManager with custom settings
    wifiManager.setConfigPortalTimeout(180); // 3 minute timeout for configuration
    wifiManager.setConnectTimeout(20);      // 20 second connection timeout
    wifiManager.setBreakAfterConfig(true);  // Exit after configuration
    
    // Set custom AP name and password
    const char* apName = "ComputerController";
    const char* apPassword = "12345678";  // 8 character minimum for WPA2
    
    // Configure WiFiManager with custom AP settings
    wifiManager.setHostname(apName);
    wifiManager.setTitle("Computer Controller Setup");
    
    // Try to connect
    if (wifiManager.autoConnect(apName, apPassword)) {
        isConnected = true;
        display.clear();
        display.setTextSize(3);
        display.setTextColor(DISPLAY_GREEN);
        display.setCursor(10, 10);
        display.println("WiFi Connected!");
        display.setTextSize(2);
        display.setCursor(10, 45);
        display.print("IP: ");
        display.println(WiFi.localIP().toString().c_str());
        ESP_LOGI(TAG, "WiFi connected successfully");
        
        // Configure SSL client for Telegram
        telegramClient.setInsecure(); // Skip certificate verification
        telegramClient.setTimeout(10000); // 10 second timeout
        
        // Feed watchdog after successful connection
        esp_task_wdt_reset();
    } else {
        isConnected = false;
        display.clear();
        display.setTextSize(3);
        display.setTextColor(DISPLAY_YELLOW);
        display.setCursor(10, 10);
        display.println("WiFi Config");
        display.setTextSize(2);
        display.setCursor(10, 45);
        display.println("Connect to:");
        ESP_LOGE(TAG, "WiFi connection failed");
    }
}