#include "ComputerController.h" 

// ESP32 core includes
#include <WiFi.h>

// Third-party library includes
#include <WiFiManager.h>

// Define log tag
static const char* TAG = "ComputerController";

ComputerController::ComputerController() :
    telegramBot(BOT_TOKEN, telegramClient),
    commandHandler(nullptr),
    wifiCheckTimer(5000),
    debugTimer(1000),
    displayUpdateTimer(100),
    rfCheckTimer(50),
    relayTimer(500),
    display(),
    buzzer(BUZZER_PIN),
    buttons(BUTTON_PIN, buzzer),
    rfReceiver(RF_INPUT_PIN),
    isConnected(false),
    settings(PersistentSettings::getInstance()),
    currentRelayState(RelayState::IDLE)
{
    // Initialize relay pins
    pinMode(POWER_RELAY_PIN, OUTPUT);
    pinMode(RESET_RELAY_PIN, OUTPUT);
    digitalWrite(POWER_RELAY_PIN, HIGH);
    digitalWrite(RESET_RELAY_PIN, HIGH);
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
        display.setTextSize(2);
        display.setTextColor(DisplayColors::WHITE);
        display.setCursor(10, 10);
        display.println("Starting...");
        ESP_LOGI(TAG, "Display initialized successfully");
    } catch (...) {
        ESP_LOGE(TAG, "Display initialization failed!");
    }
    
    // Add delay after display initialization
    delay(100);
    
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
    
    // Initialize WiFi
    connectWiFi();
    
    // Initialize RTC
    rtc.setTime(0);
    ESP_LOGI(TAG, "RTC initialized");
    
    // Initialize CommandHandler
    commandHandler = new CommandHandler(this);
    commandHandler->setup();
    ESP_LOGI(TAG, "CommandHandler initialized");
    
    // Initial display update
    updateDisplay();

    // Set ESP32 log level to show only important logs
    esp_log_level_set("*", ESP_LOG_INFO);  // Set all tags to info level
    esp_log_level_set("ssl_client", ESP_LOG_INFO);  // Set all tags to info level
}

void ComputerController::loop()
{
    // Update button state and check for presses
    buttons.loop();

    if (commandHandler) {
        commandHandler->loop();
    }

    // Update physical power/reset buttons
    powerReset.loop();
    
    // Handle power and reset buttons
    handlePowerResetButtons();
    
    // Update relay state
    updateRelayState();
    
    // Check button state and trace
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
            break;
            
        case ButtonController::State::VERY_LONG_PRESS:
            ESP_LOGI(TAG, "Button: Very long press detected");
            break;
    }
    
    // Check RF receiver
    if (rfCheckTimer.isReady()) {
        rfCheckTimer.reset();
        handleRFInput();
    }
    
    // Update display using SimpleTimer
    if (displayUpdateTimer.isReady()) {
        displayUpdateTimer.reset();
        updateDisplay();
    }
}

void ComputerController::handleRFInput()
{
    // Read RF receiver state
    bool rfValue = rfReceiver.read();
    
    // Check if the value has changed
    if (rfReceiver.hasChanged()) {
        ESP_LOGI(TAG, "RF value changed to: %s", rfValue ? "HIGH" : "LOW");
        
        // Check if we received a new button code
        if (rfReceiver.isNewButtonCode()) {
            uint32_t buttonCode = rfReceiver.getButtonCode();
            ESP_LOGI(TAG, "Received button code: 0x%X", buttonCode);
            ESP_LOGI(TAG, "Button Code (decimal): %lu", buttonCode);
        }
    }
}

void ComputerController::updateDisplay()
{
    display.clear();
    
    // Display WiFi status
    display.setTextSize(2);
    display.setTextColor(isConnected ? DisplayColors::GREEN : DisplayColors::YELLOW);
    display.setCursor(10, 10);
    display.print("WiFi: ");
    display.println(isConnected ? "Connected" : "Config Mode");
    
    if (isConnected) {
        // Display time
        display.setTextSize(2);
        display.setTextColor(DisplayColors::WHITE);
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
        display.setTextColor(DisplayColors::WHITE);
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
    display.setTextColor(DisplayColors::WHITE);
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
        display.setTextColor(DisplayColors::GREEN);
        display.setCursor(10, 10);
        display.println("WiFi Connected!");
        display.setTextSize(2);
        display.setCursor(10, 45);
        display.print("IP: ");
        display.println(WiFi.localIP().toString().c_str());
        ESP_LOGI(TAG, "WiFi connected successfully");
        
        // Configure SSL client for Telegram
        telegramClient.setInsecure(); // Skip certificate verification
        telegramClient.setTimeout(4000); // 4 second timeout
    } else {
        isConnected = false;
        display.clear();
        display.setTextSize(3);
        display.setTextColor(DisplayColors::YELLOW);
        display.setCursor(10, 10);
        display.println("WiFi Config");
        display.setTextSize(2);
        display.setCursor(10, 45);
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
            if (!PersistentSettings::getInstance().isChildLockEnabled()) {
                ESP_LOGI(TAG, "Power button pressed");
                currentRelayState = RelayState::POWER_PRESSING;
                setPowerRelay(true);
                relayTimer.reset();
            } else {
                ESP_LOGW(TAG, "Power button press ignored, child lock enabled.");
            }
        }
        // Handle reset button
        else if (powerReset.isResetPressed()) {
            if (!PersistentSettings::getInstance().isChildLockEnabled()) {
                ESP_LOGI(TAG, "Reset button pressed");
                currentRelayState = RelayState::RESET_PRESSING;
                setResetRelay(true);
                relayTimer.reset();
            } else {
                ESP_LOGW(TAG, "Reset button press ignored, child lock enabled.");
            }
        }
    }
}

void ComputerController::updateRelayState()
{
    // Check if relay timer has expired
    if (relayTimer.isReady()) {
        switch (currentRelayState) {
            case RelayState::POWER_PRESSING:
                setPowerRelay(false);
                currentRelayState = RelayState::IDLE;
                ESP_LOGI(TAG, "Power relay released");
                break;
                
            case RelayState::RESET_PRESSING:
                setResetRelay(false);
                currentRelayState = RelayState::IDLE;
                ESP_LOGI(TAG, "Reset relay released");
                break;
                
            case RelayState::IDLE:
                // Nothing to do
                break;
        }
    }
}

void ComputerController::setPowerRelay(bool state)
{
    digitalWrite(POWER_RELAY_PIN, state ? LOW : HIGH);
    ESP_LOGI(TAG, "Power relay %s (Pin State: %s)", state ? "ACTIVATED" : "DEACTIVATED", state ? "LOW" : "HIGH");
}

void ComputerController::setResetRelay(bool state)
{
    digitalWrite(RESET_RELAY_PIN, state ? LOW : HIGH);
    ESP_LOGI(TAG, "Reset relay %s (Pin State: %s)", state ? "ACTIVATED" : "DEACTIVATED", state ? "LOW" : "HIGH");
}

void ComputerController::activatePowerRelay() {
    if (currentRelayState == RelayState::IDLE) {
        ESP_LOGI(TAG, "Activating power relay via command/external call");
        currentRelayState = RelayState::POWER_PRESSING;
        setPowerRelay(true); // Activate relay
        relayTimer.reset();  // Start timer for deactivation
    } else {
        ESP_LOGW(TAG, "Cannot activate power relay, another relay operation is in progress.");
    }
}

void ComputerController::activateResetRelay() {
    if (currentRelayState == RelayState::IDLE) {
        ESP_LOGI(TAG, "Activating reset relay via command/external call");
        currentRelayState = RelayState::RESET_PRESSING;
        setResetRelay(true); // Activate relay
        relayTimer.reset();  // Start timer for deactivation
    } else {
        ESP_LOGW(TAG, "Cannot activate reset relay, another relay operation is in progress.");
    }
}
