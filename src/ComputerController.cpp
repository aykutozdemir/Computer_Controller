#include "ComputerController.h" 
#include "Credentials.h"
#include <Wire.h>

// Define log tag
static const char* TAG = "ComputerController";

ComputerController::ComputerController() :
    telegramBot(BOT_TOKEN, telegramClient),
    commandHandler(nullptr),
    webServer(nullptr),
    wifiCheckTimer(WIFI_CHECK_INTERVAL),
    debugTimer(DEBUG_OUTPUT_INTERVAL),
    displayUpdateTimer(DISPLAY_UPDATE_INTERVAL),
    rfCheckTimer(RF_CHECK_INTERVAL),
    relayTimer(0),  // Initialize with 0 interval to prevent immediate start
    display(),
    buzzer(BUZZER_PIN),
    buttons(BUTTON_PIN, buzzer),
    rfReceiver(RF_INPUT_PIN),
    rfStudyManager(rfReceiver, settings),
    isConnected(false),
    led(LED_PIN),
    settings(PersistentSettings::getInstance()),
    currentRelayState(RelayState::IDLE),
    gpuFan(GPU_FAN_CONTROL_PIN, GPU_FAN_PWM_PIN, GPU_FAN_PWM_FREQ, GPU_FAN_PWM_RESOLUTION),
    dht11(DHT11_PIN)
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

void ComputerController::reset()
{
    ESP_LOGI(TAG, "Performing full ESP32 reset...");

    // Disconnect WiFi, clear settings, and turn off WiFi radio
    WiFi.disconnect(true, true); // disconnect, erase credentials
    WiFi.mode(WIFI_OFF);
    delay(1000);

    ESP_LOGI(TAG, "Restarting ESP32 in 1 second...");
    Serial.flush();
    ESP.restart();
}

void ComputerController::setup()
{        
    ESP_LOGI(TAG, "Initializing ComputerController");
    ESP_LOGI(TAG, "Setting up ComputerController");
    
    // Initialize display with error checking
    try {
        display.begin();
        display.clear();
        display.setTextSize(DISPLAY_TEXT_SIZE_SMALL);
        display.setTextColor(DisplayColors::WHITE);
        display.setCursor(DISPLAY_CURSOR_X, DISPLAY_CURSOR_Y_START);
        display.println("Starting...");
        ESP_LOGI(TAG, "Display initialized successfully");
    } catch (...) {
        ESP_LOGE(TAG, "Display initialization failed!");
    }
    
    // Add delay after display initialization
    delay(DISPLAY_INIT_DELAY_MS);
    
    // Initialize buzzer
    buzzer.begin();
    ESP_LOGI(TAG, "Buzzer initialized");
    
    // Initialize buttons
    buttons.begin();
    ESP_LOGI(TAG, "Buttons initialized");

    // Initialize PowerResetController
    powerReset.begin();
    ESP_LOGI(TAG, "PowerResetController initialized");
    
    // Initialize RF receiver
    rfReceiver.begin();
    ESP_LOGI(TAG, "RF receiver initialized");

    // Initialize LED controller
    led.begin();
    ESP_LOGI(TAG, "LED controller initialized");
    
    // Initialize WiFi
    connectWiFi();
    
    // Initialize RTC
    rtc.setTime(0);
    ESP_LOGI(TAG, "RTC initialized");
    
    // Initialize CommandHandler
    commandHandler = new CommandHandler(this);
    commandHandler->setup();
    ESP_LOGI(TAG, "CommandHandler initialized");
    
    // Initialize WebServerManager
    webServer = new WebServerManager(*this);
    webServer->begin();
    ESP_LOGI(TAG, "WebServerManager initialized");
    
    // Initialize MQTT manager
    mqttManager.begin();
    ESP_LOGI(TAG, "MQTT manager initialized");
    
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

    // Initial display update
    updateDisplay();

    // Set ESP32 log level to show only important logs
    esp_log_level_set("*", ESP_LOG_INFO);  // Set all tags to info level
    esp_log_level_set("ssl_client", ESP_LOG_INFO);  // Set all tags to info level

    // Post setup
    buzzer.beepPattern(2, BUZZER_PATTERN_INTERVAL_MS, BUZZER_PATTERN_INTERVAL_MS);

    gpuFan.begin();

    Wire.end();

    // Initialise environmental sensors
    if (dht11.begin()) {
        ESP_LOGI(TAG, "DHT11 sensor initialised");
    } else {
        ESP_LOGW(TAG, "DHT11 sensor not found or failed to initialise");
    }
}

// Static task function to run peripheral loops
void ComputerController::peripheralTaskRunner(void* pvParameters) {
    ComputerController* instance = static_cast<ComputerController*>(pvParameters);
    ESP_LOGI(TAG, "Peripheral task started on core %d", xPortGetCoreID());
    for (;;) {
        instance->buttons.loop();
        instance->powerReset.loop();
        instance->handlePowerResetButtons();
        instance->updateRelayState();
        instance->buzzer.loop();
        instance->led.loop();
        instance->gpuFan.loop();
        instance->dht11.loop();

        // Process RF study
        instance->rfStudyManager.process();

        // Check RF receiver for normal operation
        instance->handleRFInput(); // handleRFInput contains its own timer and listening checks
        vTaskDelay(pdMS_TO_TICKS(1)); 
    }
}

void ComputerController::loop()
{
    if (commandHandler) {
        commandHandler->loop();
    }

    // Handle web server
    if (webServer) {
        webServer->loop();
    }

    // Handle MQTT manager
    mqttManager.loop();

    // Check button state and trace
    // This logging remains in the main loop. It reads the state updated by peripheralTaskRunner.
    // Ensure ButtonController::state() is safe for concurrent access (typically true for simple getters).
    ButtonController::State buttonState = buttons.state();
    switch (buttonState) {
        case ButtonController::State::NO_PRESS:
            // No press detected, no need to trace
            break;
            
        case ButtonController::State::SHORT_PRESS:
            ESP_LOGI(TAG, "Button: Short press detected");
            break;
            
        case ButtonController::State::LONG_PRESS:
            ESP_LOGI(TAG, "Button: Long press detected");
            reset();
            break;
            
        case ButtonController::State::VERY_LONG_PRESS:
            ESP_LOGI(TAG, "Button: Very long press detected");
            handleFactoryReset();
            break;
    }
    
    
    // Update display using SimpleTimer
    if (displayUpdateTimer.isReady()) {
        displayUpdateTimer.reset();
        updateDisplay();
    }
}

void ComputerController::handleRFInput() {
    // Skip RF processing if RF is disabled or RF study is active
    if (!PersistentSettings::getInstance().isRFEnabled()) {
        return;
    }

    // Skip if RF study is active
    if (rfStudyManager.isListening()) {
        return;
    }

    // Check timer to control frequency of checking for *new* codes
    if (!rfCheckTimer.isReady()) {
        return;
    }
    rfCheckTimer.reset();

    // Poll the RF receiver for new data.
    // rfReceiver.read() returns true if a new, debounced code was received.
    if (rfReceiver.read()) {
        uint32_t receivedCode = rfReceiver.getButtonCode(); // Gets the code and clears the new data flag
        uint32_t storedCode = PersistentSettings::getInstance().getRfButtonCode();
        if (storedCode != 0 && receivedCode == storedCode) {
            ESP_LOGI(TAG, "RF code match detected, activating power relay");
            activatePowerRelay();
        }
    }
}

void ComputerController::updateDisplay()
{
    display.clear();
    
    // Display WiFi status
    display.setTextSize(DISPLAY_TEXT_SIZE_SMALL);
    display.setTextColor(isConnected ? DisplayColors::GREEN : DisplayColors::YELLOW);
    display.setCursor(DISPLAY_CURSOR_X, DISPLAY_CURSOR_Y_START);
    display.print("WiFi: ");
    display.println(isConnected ? "Connected" : "Config Mode");
    
    if (isConnected) {
        // Display time
        display.setTextSize(DISPLAY_TEXT_SIZE_SMALL);
        display.setTextColor(DisplayColors::WHITE);
        display.setCursor(DISPLAY_CURSOR_X, DISPLAY_CURSOR_Y_START + DISPLAY_LINE_SPACING);
        display.print("Time: ");
        display.println(rtc.getTime().c_str());
        
        // Display IP address
        display.setCursor(DISPLAY_CURSOR_X, DISPLAY_CURSOR_Y_START + DISPLAY_LINE_SPACING * 2);
        display.print("IP: ");
        display.println(WiFi.localIP().toString().c_str());
        
        // Display RSSI (signal strength)
        char rssiStr[10];
        snprintf(rssiStr, sizeof(rssiStr), "%d dBm", WiFi.RSSI());
        display.setCursor(DISPLAY_CURSOR_X, DISPLAY_CURSOR_Y_START + DISPLAY_LINE_SPACING * 3);
        display.print("Signal: ");
        display.println(rssiStr);
        
        // Display uptime
        char uptimeStr[20];
        snprintf(uptimeStr, sizeof(uptimeStr), "%lu s", millis() / 1000);
        display.setCursor(DISPLAY_CURSOR_X, DISPLAY_CURSOR_Y_START + DISPLAY_LINE_SPACING * 4);
        display.print("Uptime: ");
        display.println(uptimeStr);
    } else {
        // Show configuration instructions
        display.setTextSize(DISPLAY_TEXT_SIZE_SMALL);
        display.setTextColor(DisplayColors::WHITE);
        display.setCursor(DISPLAY_CURSOR_X, DISPLAY_CURSOR_Y_START + DISPLAY_LINE_SPACING);
        display.println("Connect to:");
        display.setCursor(DISPLAY_CURSOR_X, DISPLAY_CURSOR_Y_START + DISPLAY_LINE_SPACING * 2);
        display.print("SSID: ");
        display.println(WIFI_AP_NAME);
        display.setCursor(DISPLAY_CURSOR_X, DISPLAY_CURSOR_Y_START + DISPLAY_LINE_SPACING * 3);
        display.print("Password: ");
        display.println(WIFI_AP_PASSWORD);
        display.setCursor(DISPLAY_CURSOR_X, DISPLAY_CURSOR_Y_START + DISPLAY_LINE_SPACING * 4);
        display.println("Then open:");
        display.setCursor(DISPLAY_CURSOR_X, DISPLAY_CURSOR_Y_START + DISPLAY_LINE_SPACING * 5);
        display.println("http://192.168.4.1");
    }
}

void ComputerController::connectWiFi()
{
    ESP_LOGI(TAG, "Connecting to WiFi...");
    
    // Set LED to connecting state
    led.setStatus(LedController::Status::CONNECTING);
    
    // Clear display and show initial message
    display.clear();
    display.setTextSize(DISPLAY_TEXT_SIZE_LARGE);
    display.setTextColor(DisplayColors::WHITE);
    display.setCursor(DISPLAY_CURSOR_X, DISPLAY_CURSOR_Y_START);
    display.println("WiFi Setup");
    display.setTextSize(DISPLAY_TEXT_SIZE_SMALL);
    display.setCursor(DISPLAY_CURSOR_X, DISPLAY_CURSOR_Y_START + DISPLAY_LINE_SPACING);
    display.println("Connecting to WiFi...");
    
    // Configure WiFiManager with custom settings
    wifiManager.setConfigPortalTimeout(WIFI_CONFIG_PORTAL_TIMEOUT);
    wifiManager.setConnectTimeout(WIFI_CONNECT_TIMEOUT_SECONDS);
    wifiManager.setBreakAfterConfig(true);  // Exit after configuration
    
    // Set custom AP name and password
    const char* apName = WIFI_AP_NAME;
    const char* apPassword = WIFI_AP_PASSWORD;
    
    // Configure WiFiManager with custom AP settings
    wifiManager.setHostname(apName);
    wifiManager.setTitle("Computer Controller Setup");
    
    // Try to connect
    if (wifiManager.autoConnect(apName, apPassword)) {
        isConnected = true;
        led.setStatus(LedController::Status::CONNECTED); // WiFi is connected
        display.clear();
        display.setTextSize(DISPLAY_TEXT_SIZE_LARGE);
        display.setTextColor(DisplayColors::GREEN);
        display.setCursor(DISPLAY_CURSOR_X, DISPLAY_CURSOR_Y_START);
        display.println("WiFi Connected!");
        display.setTextSize(DISPLAY_TEXT_SIZE_SMALL);
        display.setCursor(DISPLAY_CURSOR_X, DISPLAY_CURSOR_Y_START + DISPLAY_LINE_SPACING);
        display.print("IP: ");
        display.println(WiFi.localIP().toString().c_str());
        ESP_LOGI(TAG, "WiFi connected successfully");
        
        // Configure SSL client for Telegram
        telegramClient.setInsecure(); // Skip certificate verification
        telegramClient.setTimeout(TELEGRAM_TIMEOUT);
    } else {
        isConnected = false;
        // LED remains in CONNECTING state (blinking) for AP/Config mode
        display.clear();
        display.setTextSize(DISPLAY_TEXT_SIZE_LARGE);
        display.setTextColor(DisplayColors::YELLOW);
        display.setCursor(DISPLAY_CURSOR_X, DISPLAY_CURSOR_Y_START);
        display.println("WiFi Config");
        display.setTextSize(DISPLAY_TEXT_SIZE_SMALL);
        display.setCursor(DISPLAY_CURSOR_X, DISPLAY_CURSOR_Y_START + DISPLAY_LINE_SPACING);
        display.println("Connect to:");
        ESP_LOGE(TAG, "WiFi connection failed");
    }
}

void ComputerController::handlePowerResetButtons()
{
    // Only handle new button presses if we're in IDLE state
    if (currentRelayState == RelayState::IDLE) {
        // Handle power button
        if (powerReset.isPowerPressed()) {
            buzzer.beep(BUZZER_BEEP_DURATION_MS); // always beep on power button press
            if (!PersistentSettings::getInstance().isChildLockEnabled()) {
                ESP_LOGI(TAG, "Power button pressed");
                currentRelayState = RelayState::POWER_PRESSING;
                setPowerRelay(true);
            } else {
                ESP_LOGW(TAG, "Power button press ignored, child lock enabled.");
            }
        }
        // Handle reset button
        else if (powerReset.isResetPressed()) {
            buzzer.beep(BUZZER_BEEP_DURATION_MS); // always beep on reset button press
            if (!PersistentSettings::getInstance().isChildLockEnabled()) {
                ESP_LOGI(TAG, "Reset button pressed");
                currentRelayState = RelayState::RESET_PRESSING;
                setResetRelay(true);
            } else {
                ESP_LOGW(TAG, "Reset button press ignored, child lock enabled.");
            }
        }
    }
}

void ComputerController::updateRelayState()
{
    // Only check timer if it's enabled (has a non-zero interval)
    if (relayTimer.isEnabled() && relayTimer.isReady()) {
        switch (currentRelayState) {
            case RelayState::POWER_PRESSING:
                setPowerRelay(false);
                currentRelayState = RelayState::IDLE;
                ESP_LOGI(TAG, "Power relay released after %d ms", RELAY_TIMER_INTERVAL);
                break;
                
            case RelayState::RESET_PRESSING:
                setResetRelay(false);
                currentRelayState = RelayState::IDLE;
                ESP_LOGI(TAG, "Reset relay released after %d ms", RELAY_TIMER_INTERVAL);
                break;
                
            case RelayState::IDLE:
                // Nothing to do
                break;
        }
    } else {
        // Debug: log when timer is not ready
        static unsigned long lastDebugTime = 0;
        if (currentRelayState != RelayState::IDLE && millis() - lastDebugTime > 1000) {
            ESP_LOGI(TAG, "Timer not ready yet, state: %d, interval: %d", (int)currentRelayState, relayTimer.getInterval());
            lastDebugTime = millis();
        }
    }
}

void ComputerController::setPowerRelay(bool state)
{
    relayTimer.setInterval(RELAY_TIMER_INTERVAL);
    relayTimer.reset();
    digitalWrite(POWER_RELAY_PIN, state ? LOW : HIGH);
    if (state) {
        buzzer.beep(BUZZER_BEEP_DURATION_MS); // Short beep when activating power relay
    }
    ESP_LOGI(TAG, "Power relay %s (Pin State: %s)", state ? "ACTIVATED" : "DEACTIVATED", state ? "LOW" : "HIGH");
}

void ComputerController::setResetRelay(bool state)
{
    relayTimer.setInterval(RELAY_TIMER_INTERVAL);
    relayTimer.reset();
    digitalWrite(RESET_RELAY_PIN, state ? LOW : HIGH);
    if (state) {
        buzzer.beep(BUZZER_BEEP_DURATION_MS); // Short beep when activating reset relay
    }
    ESP_LOGI(TAG, "Reset relay %s (Pin State: %s)", state ? "ACTIVATED" : "DEACTIVATED", state ? "LOW" : "HIGH");
}

void ComputerController::activatePowerRelay() {
    if (currentRelayState == RelayState::IDLE) {
        ESP_LOGI(TAG, "Activating power relay via command/external call");
        currentRelayState = RelayState::POWER_PRESSING;
        setPowerRelay(true);
        ESP_LOGI(TAG, "Resetting relay timer for %d ms", RELAY_TIMER_INTERVAL);
        ESP_LOGI(TAG, "Power relay activated, timer started for %d ms", RELAY_TIMER_INTERVAL);
        
        // Publish MQTT event
        mqttManager.publishEvent("power_relay_activated", "via_command");
    } else {
        ESP_LOGW(TAG, "Cannot activate power relay, another relay operation is in progress.");
    }
}

void ComputerController::activateResetRelay() {
    if (currentRelayState == RelayState::IDLE) {
        ESP_LOGI(TAG, "Activating reset relay via command/external call");
        currentRelayState = RelayState::RESET_PRESSING;
        setResetRelay(true);
        ESP_LOGI(TAG, "Reset relay activated, timer started for %d ms", RELAY_TIMER_INTERVAL);
        
        // Publish MQTT event
        mqttManager.publishEvent("reset_relay_activated", "via_command");
    } else {
        ESP_LOGW(TAG, "Cannot activate reset relay, another relay operation is in progress.");
    }
}

bool ComputerController::setGpuFanSpeed(uint8_t speed) {
    if (speed > 100) {
        return false;
    }
    
    gpuFan.setSpeed(speed);
    return true;
}

uint8_t ComputerController::getGpuFanSpeed() const {
    return gpuFan.getSpeed();
}

void ComputerController::handleFactoryReset() {
    ESP_LOGI(TAG, "Factory reset triggered by button press");
    
    // Play triple beep to indicate factory reset
    buzzer.beepPattern(BUZZER_FACTORY_RESET_BEEPS, BUZZER_FACTORY_RESET_DURATION_MS, BUZZER_FACTORY_RESET_INTERVAL_MS);
    delay(300); // Wait for beeps to complete
    
    // Clear all settings
    PersistentSettings::getInstance().clearAll();
    
    // Reset the MCU
    ESP.restart();
}