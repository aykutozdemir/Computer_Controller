#include "ComputerController.h"
#include "Credentials.h"
#include <Wire.h>
#include <math.h> // for isnan
#include <esp_task_wdt.h>
#include <WiFi.h>
#include "SimpleUI/SimpleUI.h"
#include "RootCA.h"
#include "ComputerControllerUI.h"
#include <esp_wifi.h>
#include "ICacheDisplay.h"
#include "CacheDisplay.h"
#include "DirectDisplay.h"

// Define log tag
static const char* TAG = "ComputerController";

// =====================================================================
// CONSTRUCTOR & LIFECYCLE METHODS
// =====================================================================

ComputerController::ComputerController() :
    telegramBot(BOT_TOKEN, telegramClient),
    commandHandler(nullptr),
    wifiCheckTimer(WIFI_CHECK_INTERVAL),
    debugTimer(DEBUG_OUTPUT_INTERVAL),
    displayUpdateTimer(DISPLAY_UPDATE_INTERVAL),
    rfCheckTimer(RF_CHECK_INTERVAL),
    relayTimer(0),  // Initialize with 0 interval to prevent immediate start
    display(nullptr),
    buzzer(BUZZER_PIN),
    button(BUTTON_PIN, buzzer),
    rfReceiver(RF_INPUT_PIN),
    rfStudyManager(rfReceiver, settings),
    isConnected(false),
    led(LED_PIN),
    settings(PersistentSettings::getInstance()),
    currentRelayState(RelayState::IDLE),
    wasInSetupMode(false),
    gpuFan(GPU_FAN_CONTROL_PIN, GPU_FAN_PWM_PIN, GPU_FAN_PWM_FREQ, GPU_FAN_PWM_RESOLUTION),
    dht11(DHT11_PIN),
    ui(nullptr),
    webServerManager(nullptr)
{
    ESP_LOGI(TAG, "Initializing ComputerController with bot token: %s", BOT_TOKEN);
    
    // Initialize relay pins
    pinMode(POWER_RELAY_PIN, OUTPUT);
    pinMode(RESET_RELAY_PIN, OUTPUT);
    digitalWrite(POWER_RELAY_PIN, HIGH);
    digitalWrite(RESET_RELAY_PIN, HIGH);

    // Initialize PC power status pin
    pinMode(PC_POWERED_ON_PIN, INPUT_PULLUP);

    // Set initial buzzer state from persistent settings
    buzzer.setEnabled(settings.isBuzzerEnabled());
    
    // Initialize fan to off state
    setGpuFanSpeed(0);
}

void ComputerController::setup()
{        
    ESP_LOGI(TAG, "Starting ComputerController setup");
    
    // Feed watchdog at start of setup
    esp_task_wdt_reset();
    yield();
    
    // =====================================================================
    // DISPLAY SYSTEM INITIALIZATION (BEFORE ANY NETWORK CODE)
    // =====================================================================
    
    ESP_LOGI(TAG, "=== STEP 1: DISPLAY INITIALIZATION (BEFORE NETWORK) ===");
    ESP_LOGI(TAG, "Initializing display system before any network code...");
    
    // Add extra delay before display initialization to ensure stable power
    delay(100);
    esp_task_wdt_reset();
    yield();
    
    try {
        // 1. Create the real hardware GFX driver
        Arduino_GFX* realGfx = new Arduino_ST7796(
            new Arduino_ESP32SPI(DISPLAY_DC, DISPLAY_CS, DISPLAY_SCLK, DISPLAY_MOSI, -1, HSPI, DISPLAY_SPI_FREQ),
            DISPLAY_RST, 0, false, DISPLAY_WIDTH, DISPLAY_HEIGHT, 0, 0, 0, 0);
        ESP_LOGI(TAG, "Display driver created successfully");

        // 2. Initialize the real hardware display first
        ESP_LOGI(TAG, "Initializing real hardware display...");
        if (!realGfx->begin()) {
            ESP_LOGE(TAG, "Real hardware display initialization failed");
            delete realGfx;
            display = nullptr;
        } else {
            ESP_LOGI(TAG, "Real hardware display initialized successfully");

            // Use DirectDisplay for direct LCD access (no cache)
            display = new DirectDisplay(realGfx);
            ESP_LOGI(TAG, "Direct display interface created successfully");
            // Configure the display
            display->setRotation(DISPLAY_START_ROTATION);
            display->fillScreen(COLOR_BACKGROUND);
            ESP_LOGI(TAG, "Display configured successfully!");
            
            ESP_LOGI(TAG, "=== DISPLAY INITIALIZATION COMPLETE ===");
        }
    } catch (...) {
        ESP_LOGE(TAG, "Exception during display initialization - continuing without display");
        display = nullptr;
    }
    
    esp_task_wdt_reset();
    yield();
    delay(50); // Additional delay after display initialization
    
    // =====================================================================
    // HARDWARE CONTROLLERS INITIALIZATION
    // =====================================================================
    
    ESP_LOGI(TAG, "=== STEP 2: HARDWARE CONTROLLERS INITIALIZATION ===");
    ESP_LOGI(TAG, "Initializing hardware controllers...");
    
    // Initialize buzzer
    buzzer.begin();
    ESP_LOGI(TAG, "Buzzer initialized");
    esp_task_wdt_reset();
    yield();
    
    // Initialize buttons
    button.begin();
    ESP_LOGI(TAG, "Buttons initialized");
    esp_task_wdt_reset();
    yield();

    // Initialize PowerResetController
    powerReset.begin();
    ESP_LOGI(TAG, "PowerResetController initialized");
    esp_task_wdt_reset();
    yield();
    
    // Initialize RF receiver
    rfReceiver.begin();
    ESP_LOGI(TAG, "RF receiver initialized");
    esp_task_wdt_reset();
    yield();

    // Initialize LED controller
    led.begin();
    ESP_LOGI(TAG, "LED controller initialized");
    esp_task_wdt_reset();
    yield();

    // Initialize sensors
    dht11.begin();
    ESP_LOGI(TAG, "DHT11 sensor initialized");
    esp_task_wdt_reset();
    yield();

    // Initialize GPU fan controller
    gpuFan.begin();
    ESP_LOGI(TAG, "GPU fan controller initialized");
    esp_task_wdt_reset();
    yield();

    // =====================================================================
    // NETWORK & COMMUNICATION INITIALIZATION (AFTER DISPLAY)
    // =====================================================================
    
    ESP_LOGI(TAG, "=== STEP 3: NETWORK & COMMUNICATION INITIALIZATION ===");
    ESP_LOGI(TAG, "Initializing network and communication after display...");
    
    // Initialize WiFi
    connectWiFi();
    esp_task_wdt_reset();
    yield();

    // Initialize RTC and sync with NTP after WiFi connection
    rtc.setTime(0);
    ESP_LOGI(TAG, "RTC initialized");
    
    // Sync time with NTP if WiFi is connected
    if (WiFi.isConnected()) {
        syncTimeWithNTP();
    }
    esp_task_wdt_reset();
    yield();

    // Initialize CommandHandler
    commandHandler = new CommandHandler(this);
    commandHandler->setup();
    ESP_LOGI(TAG, "CommandHandler initialized");
    esp_task_wdt_reset();
    yield();

    // Initialize MQTT manager
    mqttManager.begin();
    ESP_LOGI(TAG, "MQTT manager initialized");
    esp_task_wdt_reset();
    yield();

    // RF study manager is initialized in constructor
    ESP_LOGI(TAG, "RF study manager initialized");
    esp_task_wdt_reset();
    yield();

    // =====================================================================
    // UI SYSTEM INITIALIZATION
    // =====================================================================
    
    ESP_LOGI(TAG, "=== STEP 4: UI SYSTEM INITIALIZATION ===");
    ESP_LOGI(TAG, "Initializing UI system...");
    
    // Initialize UI system if display is available
    if (display) {
        ui = new ComputerControllerUI(*this);
        ui->begin();
        ESP_LOGI(TAG, "UI system initialized");
        
        // UI system ready
        ESP_LOGI(TAG, "UI system ready");
    } else {
        ESP_LOGW(TAG, "Display not available, UI system not initialized");
    }
    esp_task_wdt_reset();
    yield();

    // =====================================================================
    // TASK CREATION
    // =====================================================================
    
    ESP_LOGI(TAG, "=== STEP 5: TASK CREATION ===");
    ESP_LOGI(TAG, "Creating peripheral handling task...");
    
    // Create peripheral handling task
    xTaskCreatePinnedToCore(
        peripheralTaskRunner,     // Task function
        "PeripheralTask",       // Name of the task
        PERIPHERAL_TASK_STACK_SIZE,  // Stack size in words
        this,                   // Task input parameter
        PERIPHERAL_TASK_PRIORITY,    // Priority of the task
        NULL,                   // Task handle
        PERIPHERAL_TASK_CORE    // Core where the task should run
    );
    esp_task_wdt_reset();
    yield();

    // =====================================================================
    // FINAL SETUP STEPS
    // =====================================================================
    
    // Set ESP32 log level to show only important logs
    esp_log_level_set("*", ESP_LOG_INFO);  // Set all tags to info level
    esp_log_level_set("ssl_client", ESP_LOG_INFO);  // Set all tags to info level

    // Post setup
    buzzer.beepPattern(2, BUZZER_PATTERN_INTERVAL_MS, BUZZER_PATTERN_INTERVAL_MS);
    esp_task_wdt_reset();
    yield();

    Wire.end();
    esp_task_wdt_reset();
    yield();

    ESP_LOGI(TAG, "ComputerController setup completed successfully");
    esp_task_wdt_reset();
    yield();
}

ComputerController::~ComputerController() {
    ESP_LOGI(TAG, "ComputerController destructor called");
    
    // Clean up web server manager
    if (webServerManager) {
        delete webServerManager;
        webServerManager = nullptr;
    }
    
    // Clean up command handler
    if (commandHandler) {
        delete commandHandler;
        commandHandler = nullptr;
    }
    
    // Clean up UI
    if (ui) {
        delete ui;
        ui = nullptr;
    }
    
    // Clean up display
    if (display) {
        delete display;
        display = nullptr;
    }
}

void ComputerController::loop()
{
    // Feed watchdog more frequently when in WiFi setup mode or connection attempts
    static uint32_t lastWatchdogFeed = 0;
    uint32_t now = millis();
    
    // Feed watchdog every 500ms in AP mode, every 1 second during connection attempts, or every 2 seconds normally
    uint32_t watchdogInterval = 2000; // Default 2 seconds
    if (WiFi.getMode() == WIFI_MODE_AP) {
        watchdogInterval = 500; // 500ms in AP mode
    } else if (!WiFi.isConnected() && WiFi.getMode() == WIFI_MODE_STA) {
        watchdogInterval = 1000; // 1 second during connection attempts
    }
    
    if (now - lastWatchdogFeed > watchdogInterval) {
        esp_task_wdt_reset();
        lastWatchdogFeed = now;
    }
    
    // =====================================================================
    // COMMAND PROCESSING
    // =====================================================================
    
    if (commandHandler) {
        commandHandler->loop();
    }
    
    // =====================================================================
    // WIFI CONNECTION MANAGEMENT
    // =====================================================================
    
    // Check WiFi connection status and handle disconnections
    static bool wasConnected = false;
    bool currentlyConnected = WiFi.isConnected();
    
    if (currentlyConnected != wasConnected) {
        if (currentlyConnected) {
            ESP_LOGI(TAG, "WiFi connected - IP: %s", WiFi.localIP().toString().c_str());
            led.setStatus(LedController::Status::CONNECTED);
            
            // Switch to connected page if UI is available
            if (ui) {
                ui->switchToPage(ComputerControllerUI::Page::CONNECTED);
                ESP_LOGI(TAG, "Switched to CONNECTED page");
            }
            
            // Sync time with NTP after WiFi connection
            syncTimeWithNTP();
            
            // Initialize network services after connection
            if (!mqttManager.isConnectedToBroker()) {
                mqttManager.connectToBroker();
            }
            
            if (webServerManager) {
                webServerManager->begin();
            }
        } else {
            if (led.getStatus() != LedController::Status::OFF) {
                led.setStatus(LedController::Status::OFF);
                ESP_LOGI(TAG, "WiFi disconnected - LED set to OFF");
            }
            
            // Switch to AP mode page if in setup mode, otherwise error page
            if (ui) {
                // Check if we're in setup mode (either AP mode or was recently in setup mode)
                bool inSetupMode = (WiFi.getMode() == WIFI_MODE_AP) || wasInSetupMode;
                
                if (inSetupMode) {
                    if (ui->getCurrentPage() != ComputerControllerUI::Page::AP_MODE) {
                        ui->switchToPage(ComputerControllerUI::Page::AP_MODE);
                        ESP_LOGI(TAG, "Switched to AP_MODE page (setup mode detected)");
                    }
                } else {
                    if (ui->getCurrentPage() != ComputerControllerUI::Page::ERROR) {
                        ui->switchToPage(ComputerControllerUI::Page::ERROR);
                        ESP_LOGI(TAG, "Switched to ERROR page (not in setup mode)");
                    }
                }
            }
        }
        wasConnected = currentlyConnected;
    }
    
    // Only process network services when WiFi is connected
    if (WiFi.isConnected()) {
        // Handle MQTT manager
        mqttManager.loop();
        
        // Handle web server if initialized
        if (webServerManager) {
            webServerManager->loop();
        }
    }
    
    // Process WiFiManager if in setup mode (regardless of connection)
    bool currentlyInAPMode = (WiFi.getMode() == WIFI_MODE_AP);
    
    // Handle WiFi mode transitions
    handleWiFiModeTransition();
    
    if (currentlyInAPMode || wasInSetupMode) {
        // Process WiFiManager more frequently in AP mode for better responsiveness
        static uint32_t lastWiFiManagerProcess = 0;
        uint32_t now = millis();
        
        // Process WiFiManager every 100ms in AP mode for better responsiveness
        if (now - lastWiFiManagerProcess >= 100) {
            wifiManager.process();
            lastWiFiManagerProcess = now;
        }
        
        wasInSetupMode = true; // Mark that we're in setup mode

        // -----------------------------------------------------------------
        // Notify UI when a device (e.g. phone) connects to the ESP32
        // -----------------------------------------------------------------
        static int lastStationCount = 0;
        int currentStationCount = WiFi.softAPgetStationNum();
        if (currentStationCount != lastStationCount) {
            ESP_LOGI(TAG, "SoftAP station count changed: %d -> %d", lastStationCount, currentStationCount);
            lastStationCount = currentStationCount;
            if (ui) {
                ui->updateAPConnectionStatus(currentStationCount > 0);
            }
        }

        // Check if WiFi setup was completed and we're now connected
        if (wasInSetupMode && WiFi.isConnected()) {
            ESP_LOGI(TAG, "WiFi setup completed - switching to connected status");
            wasInSetupMode = false;
        }
        
        // Additional yield in AP mode to prevent blocking
        yield();
    } else {
        // Reset setup mode flag when not in AP mode and not recently in setup mode
        // Add a delay before resetting to prevent flickering
        static uint32_t lastSetupModeExit = 0;
        uint32_t now = millis();
        
        if (wasInSetupMode) {
            if (lastSetupModeExit == 0) {
                lastSetupModeExit = now;
            } else if (now - lastSetupModeExit > 5000) { // Wait 5 seconds before resetting
                wasInSetupMode = false;
                lastSetupModeExit = 0;
                ESP_LOGI(TAG, "Setup mode flag reset after timeout");
            }
        }
    }
    
    // =====================================================================
    // DISPLAY UPDATES
    // =====================================================================
    
    // Update display using SimpleTimer
    if (displayUpdateTimer.isReady()) {
        displayUpdateTimer.reset();
        // Update UI if available
        if (ui) {
            ui->update();
            
            // Update UI periodically
            static uint32_t lastWifiUpdate = 0;
            if (millis() - lastWifiUpdate > 5000) { // Update every 5 seconds
                lastWifiUpdate = millis();
                // UI updates handled by the UI system itself
            }
        }
        

    }

    // Additional yield to prevent blocking
    yield();
}



void ComputerController::reset()
{
    ESP_LOGI(TAG, "Performing full ESP32 reset...");

    // Disconnect WiFi, clear settings, and turn off WiFi radio
    WiFi.disconnect(true, false); // disconnect, keep credentials
    WiFi.mode(WIFI_OFF);
    delay(1000);

    ESP_LOGI(TAG, "Restarting ESP32 in 1 second...");
    Serial.flush();
    ESP.restart();
}

// =====================================================================
// HARDWARE CONTROL METHODS
// =====================================================================

void ComputerController::activatePowerRelay() {
    if (currentRelayState != RelayState::IDLE) {
        ESP_LOGW(TAG, "Relay already active, ignoring power activation");
        return;
    }
    
    setPowerRelay(true);
    currentRelayState = RelayState::POWER_ACTIVE;
    relayTimer.setInterval(RELAY_TIMER_INTERVAL);
    relayTimer.reset();
    ESP_LOGI(TAG, "Power relay activated for %d ms", RELAY_TIMER_INTERVAL);
    
    // Publish MQTT event
    mqttManager.publishEvent("power_relay_activated", "true");
}

void ComputerController::activateResetRelay() {
    if (currentRelayState != RelayState::IDLE) {
        ESP_LOGW(TAG, "Relay already active, ignoring reset activation");
        return;
    }
    
    setResetRelay(true);
    currentRelayState = RelayState::RESET_ACTIVE;
    relayTimer.setInterval(RELAY_TIMER_INTERVAL);
    relayTimer.reset();
    ESP_LOGI(TAG, "Reset relay activated for %d ms", RELAY_TIMER_INTERVAL);
    
    // Publish MQTT event
    mqttManager.publishEvent("reset_relay_activated", "true");
}

bool ComputerController::setGpuFanSpeed(uint8_t speed) {
    if (speed > 100) {
        ESP_LOGW(TAG, "Invalid fan speed: %d (max 100)", speed);
        return false;
    }
    
    gpuFan.setSpeed(speed);
    bool success = true;
    if (success) {
        ESP_LOGI(TAG, "GPU fan speed set to: %d%%", speed);
        
        // Publish MQTT event
        char speedStr[8];
        snprintf(speedStr, sizeof(speedStr), "%d", speed);
        mqttManager.publishEvent("gpu_fan_speed", speedStr);
    } else {
        ESP_LOGW(TAG, "Failed to set GPU fan speed to: %d%%", speed);
    }
    
    return success;
}

uint8_t ComputerController::getGpuFanSpeed() const {
    return gpuFan.getSpeed();
}

// =====================================================================
// TIME MANAGEMENT METHODS
// =====================================================================

bool ComputerController::syncTimeWithNTP() {
    ESP_LOGI(TAG, "Syncing time with NTP...");
    
    // Set timezone and NTP servers
    configTime(0, 0, "pool.ntp.org", "time.nist.gov", "time.google.com");
    
    // Wait for time sync with better error handling
    int retry = 0;
    const int maxRetries = 15; // Increased retry count
    
    while (retry < maxRetries) {
        time_t now = time(nullptr);
        struct tm timeinfo;
        
        if (localtime_r(&now, &timeinfo)) {
            // Check if we have a valid year (2024 or later)
            if (timeinfo.tm_year + 1900 >= 2024) {
                // Update RTC with the synced time
                rtc.setTime(now);
                ESP_LOGI(TAG, "NTP sync successful: %04d-%02d-%02d %02d:%02d:%02d", 
                         timeinfo.tm_year + 1900, timeinfo.tm_mon + 1, timeinfo.tm_mday,
                         timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
                return true;
            }
        }
        
        delay(1000);
        retry++;
        ESP_LOGI(TAG, "Waiting for NTP sync... retry %d/%d", retry, maxRetries);
        
        // Feed watchdog during NTP sync
        esp_task_wdt_reset();
    }
    
    ESP_LOGW(TAG, "NTP sync failed after %d retries", maxRetries);
    return false;
}

String ComputerController::getCurrentTimeString() {
    // Force time refresh by getting current timestamp and formatting manually
    time_t now = time(nullptr);
    struct tm timeinfo;
    localtime_r(&now, &timeinfo);
    
    char timeStr[9];
    snprintf(timeStr, sizeof(timeStr), "%02d:%02d:%02d", 
             timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
    return String(timeStr);
}


// =====================================================================
// PRIVATE METHODS - NETWORK MANAGEMENT
// =====================================================================

void ComputerController::handleWiFiModeTransition() {
    static wifi_mode_t lastMode = WIFI_MODE_NULL;
    wifi_mode_t currentMode = WiFi.getMode();
    
    if (currentMode != lastMode) {
        ESP_LOGI(TAG, "WiFi mode transition: %d -> %d", (int)lastMode, (int)currentMode);
        
        switch (currentMode) {
            case WIFI_MODE_AP:
                ESP_LOGI(TAG, "Entered AP mode");
                led.setStatus(LedController::Status::OFF);
                if (ui) {
                    ui->switchToPage(ComputerControllerUI::Page::AP_MODE);
                }
                break;
                
            case WIFI_MODE_STA:
                ESP_LOGI(TAG, "Entered Station mode");
                if (WiFi.isConnected()) {
                    led.setStatus(LedController::Status::CONNECTED);
                    if (ui) {
                        ui->switchToPage(ComputerControllerUI::Page::CONNECTED);
                    }
                } else {
                    led.setStatus(LedController::Status::CONNECTING);
                    if (ui) {
                        ui->switchToPage(ComputerControllerUI::Page::CONNECTING);
                    }
                }
                break;
                
            case WIFI_MODE_APSTA:
                ESP_LOGI(TAG, "Entered AP+Station mode");
                break;
                
            case WIFI_MODE_NULL:
                ESP_LOGI(TAG, "WiFi disabled");
                led.setStatus(LedController::Status::OFF);
                break;
        }
        
        lastMode = currentMode;
    }
}

bool ComputerController::setupAPMode() {
    ESP_LOGI(TAG, "Setting up WiFi AP mode...");
    
    // Disconnect any existing WiFi connections
    WiFi.disconnect(true, false);
    delay(1000);
    
    // Set WiFi mode to AP only
    WiFi.mode(WIFI_AP);
    delay(500);
    
    // Configure AP settings
    const char* apName = WIFI_AP_NAME;
    const char* apPassword = WIFI_AP_PASSWORD;
    
    // Set AP configuration
    WiFi.softAPConfig(IPAddress(192, 168, 4, 1), IPAddress(192, 168, 4, 1), IPAddress(255, 255, 255, 0));
    
    // Start AP with custom settings
    bool apStarted = WiFi.softAP(apName, apPassword, 1, 0, 4); // Channel 1, hidden=false, max_connections=4
    
    if (apStarted) {
        ESP_LOGI(TAG, "WiFi AP started successfully");
        ESP_LOGI(TAG, "AP Name: %s", apName);
        ESP_LOGI(TAG, "AP IP: %s", WiFi.softAPIP().toString().c_str());
        ESP_LOGI(TAG, "AP MAC: %s", WiFi.softAPmacAddress().c_str());
        
        // Set AP hostname
        WiFi.softAPsetHostname(apName);
        
        return true;
    } else {
        ESP_LOGE(TAG, "Failed to start WiFi AP");
        return false;
    }
}

bool ComputerController::tryDirectWiFiConnection() {
    ESP_LOGI(TAG, "Attempting direct WiFi connection...");
    
    // Get saved credentials from WiFiManager
    String ssid = wifiManager.getWiFiSSID();
    String password = wifiManager.getWiFiPass();
    
    if (ssid.length() == 0) {
        ESP_LOGW(TAG, "No saved WiFi credentials found");
        return false;
    }
    
    ESP_LOGI(TAG, "Attempting to connect to: %s", ssid.c_str());
    
    // Disconnect any existing connection
    WiFi.disconnect(true);
    delay(1000);
    
    // Set WiFi mode to station
    WiFi.mode(WIFI_STA);
    
    // Attempt connection with timeout
    uint32_t startTime = millis();
    const uint32_t timeout = 15000; // 15 second timeout
    
    WiFi.begin(ssid.c_str(), password.c_str());
    
    while (WiFi.status() != WL_CONNECTED && (millis() - startTime) < timeout) {
        // Feed watchdog every 500ms during connection attempt
        static uint32_t lastWatchdogFeed = 0;
        if (millis() - lastWatchdogFeed > 500) {
            esp_task_wdt_reset();
            lastWatchdogFeed = millis();
        }
        
        yield();
        delay(100);
        
        // Log connection status every 2 seconds
        if ((millis() - startTime) % 2000 < 100) {
            ESP_LOGI(TAG, "WiFi status: %d", WiFi.status());
        }
    }
    
    if (WiFi.status() == WL_CONNECTED) {
        ESP_LOGI(TAG, "Direct WiFi connection successful! IP: %s", WiFi.localIP().toString().c_str());
        return true;
    } else {
        ESP_LOGW(TAG, "Direct WiFi connection failed. Status: %d", WiFi.status());
        return false;
    }
}

bool ComputerController::tryStaConnect(uint16_t timeoutSeconds) {
    // Check credentials directly from NVS
    if (!hasStoredCredentials()) {
        ESP_LOGW(TAG, "No stored WiFi credentials – skipping STA connect");
        return false;
    }

    String ssid = WiFi.SSID(); // May still be empty on first boot, log generically
    ESP_LOGI(TAG, "Trying STA connect with stored credentials%s", ssid.length() ? (" to SSID: " + ssid).c_str() : "");

    WiFi.mode(WIFI_STA);
    delay(100);

    WiFi.begin(); // Use stored credentials

    uint32_t start = millis();
    while (WiFi.status() != WL_CONNECTED && (millis() - start) < (timeoutSeconds * 1000UL)) {
        esp_task_wdt_reset();
        delay(100);
    }

    if (WiFi.status() == WL_CONNECTED) {
        ESP_LOGI(TAG, "Connected. IP: %s", WiFi.localIP().toString().c_str());
        return true;
    } else {
        ESP_LOGW(TAG, "STA connection timed out");
        return false;
    }
}

bool ComputerController::hasStoredCredentials() {
    // Ensure WiFi driver is initialised so esp_wifi_get_config works
    if (WiFi.getMode() == WIFI_MODE_NULL) {
        WiFi.mode(WIFI_STA);
    }
    wifi_config_t cfg;
    if (esp_wifi_get_config(WIFI_IF_STA, &cfg) == ESP_OK) {
        return strlen(reinterpret_cast<const char*>(cfg.sta.ssid)) > 0;
    }
    return false;
}

void ComputerController::startConfigPortal() {
    const char* apName = WIFI_AP_NAME;
    const char* apPassword = WIFI_AP_PASSWORD;

    // Ensure AP mode
    WiFi.mode(WIFI_AP);

    // Non-blocking portal
    wifiManager.setConfigPortalBlocking(false);
    wifiManager.setBreakAfterConfig(true);

    portalActive = true;
    wasInSetupMode = true;

    wifiManager.startConfigPortal(apName, apPassword);

    ESP_LOGI(TAG, "Config portal started. Connect to %s (IP %s)", apName, WiFi.softAPIP().toString().c_str());
}

void ComputerController::connectWiFi() {
    ESP_LOGI(TAG, "WiFi init …");

    // Basic visual feedback
    led.setStatus(LedController::Status::CONNECTING);
    if (ui) ui->switchToPage(ComputerControllerUI::Page::CONNECTING);

    // First, try stored credentials
    if (tryStaConnect()) {
        isConnected = true;
        led.setStatus(LedController::Status::CONNECTED);
        if (ui) ui->switchToPage(ComputerControllerUI::Page::CONNECTED);
        // Configure Telegram client with proper certificate verification
        telegramClient.setCACert(GODADDY_ROOT_CA_G2_PEM);
        telegramClient.setTimeout(TELEGRAM_TIMEOUT);
        
        // Additional security settings for certificate verification
        // telegramClient.setInsecure(); // Uncomment only for development/testing
        
        // Log certificate configuration
        ESP_LOGI(TAG, "Telegram client configured with GoDaddy Root CA certificate");
        ESP_LOGI(TAG, "Certificate verification enabled");
        ESP_LOGI(TAG, "Buffer sizes: 4096 bytes");
        ESP_LOGI(TAG, "WiFi connected via stored creds");
        return;
    }

    // If credentials are saved but connection failed -> stay in STA mode and keep retrying.
    if (hasStoredCredentials()) {
        ESP_LOGW(TAG, "Stored credentials present but connection failed. Will keep retrying in STA mode; AP portal NOT started.");
        WiFi.setAutoReconnect(true);
        // Visual feedback
        led.setStatus(LedController::Status::OFF);
        if (ui) ui->switchToPage(ComputerControllerUI::Page::ERROR);
        return; // Do not start portal
    }

    // No credentials stored -> start portal
    ESP_LOGW(TAG, "No stored SSID. Starting AP configuration portal…");
    startConfigPortal();
}

// =====================================================================
// PRIVATE METHODS - HARDWARE CONTROL
// =====================================================================

void ComputerController::handleRFInput() {
    if (rfReceiver.isNewButtonCode()) {
        uint32_t code = rfReceiver.getButtonCode();
        ESP_LOGI(TAG, "RF Code received: %lu", code);
        
        // Check if this code matches our stored button code
        if (code == PersistentSettings::getInstance().getRfButtonCode()) {
            ESP_LOGI(TAG, "RF code matches stored button code - triggering power relay");
            activatePowerRelay();
        }
        
        // Publish MQTT event
        char codeStr[16];
        snprintf(codeStr, sizeof(codeStr), "%lu", code);
        mqttManager.publishEvent("rf_code_received", codeStr);
    }
}

void ComputerController::handlePowerResetButtons() {
    if (powerReset.isPowerPressed()) {
        ESP_LOGI(TAG, "Power button pressed via PowerResetController");
        activatePowerRelay();
    }
    
    if (powerReset.isResetPressed()) {
        ESP_LOGI(TAG, "Reset button pressed via PowerResetController");
        activateResetRelay();
    }
}

void ComputerController::setPowerRelay(bool state)
{
    digitalWrite(POWER_RELAY_PIN, !state); // Inverted logic
    ESP_LOGI(TAG, "Power relay set to: %s", state ? "ON" : "OFF");
}

void ComputerController::setResetRelay(bool state)
{
    digitalWrite(RESET_RELAY_PIN, !state); // Inverted logic
    ESP_LOGI(TAG, "Reset relay set to: %s", state ? "ON" : "OFF");
}

void ComputerController::updateRelayState() {
    switch (currentRelayState) {
        case RelayState::IDLE:
            // Do nothing
            break;
            
        case RelayState::POWER_ACTIVE:
            if (relayTimer.isReady()) {
                setPowerRelay(false);
                currentRelayState = RelayState::IDLE;
                ESP_LOGI(TAG, "Power relay deactivated");
            }
            break;
            
        case RelayState::RESET_ACTIVE:
            if (relayTimer.isReady()) {
                setResetRelay(false);
                currentRelayState = RelayState::IDLE;
                ESP_LOGI(TAG, "Reset relay deactivated");
            }
            break;
    }
}

// =====================================================================
// PRIVATE METHODS - SYSTEM MANAGEMENT
// =====================================================================

// Static task function to run peripheral loops
void ComputerController::peripheralTaskRunner(void* pvParameters) {
    ComputerController* instance = static_cast<ComputerController*>(pvParameters);
    ESP_LOGI(TAG, "Peripheral task started on core %d", xPortGetCoreID());
    
    for (;;) {
        // Reset watchdog at the start of each iteration
        // Feed more frequently when in WiFi setup mode or during connection attempts
        static uint32_t lastWatchdogFeed = 0;
        uint32_t now = millis();
        
        // Determine watchdog feeding interval based on WiFi state
        uint32_t watchdogInterval = 1000; // Default 1 second
        if (WiFi.getMode() == WIFI_MODE_AP) {
            watchdogInterval = 250; // 250ms in AP mode
        } else if (!WiFi.isConnected() && WiFi.getMode() == WIFI_MODE_STA) {
            watchdogInterval = 500; // 500ms during connection attempts
        }
        
        if (now - lastWatchdogFeed > watchdogInterval) {
            esp_task_wdt_reset();
            lastWatchdogFeed = now;
        }
        
        // Process hardware controllers
        instance->button.loop();
        instance->powerReset.loop();
        instance->handlePowerResetButtons();
        instance->updateRelayState();
        instance->buzzer.loop();
        instance->led.loop();
        instance->gpuFan.loop();
        
        // Try DHT11 sensor with error handling
        static int dht11ErrorCount = 0;
        static bool dht11Enabled = true;
        
        if (dht11Enabled) {
            try {
                instance->dht11.loop();
                dht11ErrorCount = 0; // Reset error count on success
            } catch (...) {
                dht11ErrorCount++;
                ESP_LOGW(TAG, "DHT11 sensor error, count: %d", dht11ErrorCount);
                
                // Disable DHT11 after 5 consecutive errors
                if (dht11ErrorCount >= 5) {
                    dht11Enabled = false;
                    ESP_LOGW(TAG, "DHT11 sensor disabled due to repeated errors");
                }
            }
        }

        // Process RF study
        instance->rfStudyManager.process();

        // Check RF receiver for normal operation
        instance->handleRFInput(); // handleRFInput contains its own timer and listening checks
        
        // Add small delay in AP mode to be more cooperative with WiFiManager
        if (WiFi.getMode() == WIFI_MODE_AP) {
            vTaskDelay(pdMS_TO_TICKS(10)); // 10ms delay
        }

        // =====================================================================
        // BUTTON STATE PROCESSING
        // =====================================================================
        
        // Check button state and trace
        // This logging remains in the main loop. It reads the state updated by peripheralTaskRunner.
        // Ensure ButtonController::state() is safe for concurrent access (typically true for simple getters).
        ButtonController::State buttonState = instance->button.state();
        switch (buttonState) {
            case ButtonController::State::NO_PRESS:
                // No press detected, no need to trace
                break;
                
            case ButtonController::State::SHORT_PRESS:
                ESP_LOGI(TAG, "Button: Short press detected");
                break;
                
            case ButtonController::State::LONG_PRESS:
                ESP_LOGI(TAG, "Button: Long press detected");
                instance->reset();
                break;
                
            case ButtonController::State::VERY_LONG_PRESS:
                ESP_LOGI(TAG, "Button: Very long press detected");
                instance->handleFactoryReset();
                break;
        }
        
        // Reset watchdog before delay
        esp_task_wdt_reset();
        vTaskDelay(pdMS_TO_TICKS(10)); // 10ms delay between iterations
    }
}

void ComputerController::handleFactoryReset() {
    ESP_LOGI(TAG, "Factory reset triggered");
    
    // Clear all persistent settings
    settings.clearAll();
    
    // Restart the device
    ESP.restart();
}