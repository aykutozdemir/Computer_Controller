#include "ComputerControllerUI.h"
#include "ComputerController.h"
#include "esp_log.h"
#include "ICacheDisplay.h"

// Define StatusConstants
namespace StatusConstants {
    const char* WIFI_ON = "WiFi: ON";
    const char* WIFI_OFF = "WiFi: OFF";
    const char* MQTT_ON = "MQTT: ON";
    const char* MQTT_OFF = "MQTT: OFF";
    const char* TG_ON = "Telegram: ON";
    const char* TG_OFF = "Telegram: OFF";
    const char* WS_ON = "WebServer: ON";
    const char* WS_OFF = "WebServer: OFF";
}

// Forward declaration of GFXUIAdapter from ComputerController
// Remove GFXUIAdapter class definition entirely

ComputerControllerUI* g_ui = nullptr;

static const char* TAG = "CC_UI";

// ---------------------------------------------------------------------------
// Constructor / Destructor
// ---------------------------------------------------------------------------
ComputerControllerUI::ComputerControllerUI(ComputerController& controller)
    : _controller(controller),
      _displayInterface(nullptr),
      _uiApp(nullptr),
      _currentPage(Page::CONNECTING),
      _messageLabel(nullptr), _messageStartTime(0), _messageDuration(0), _messageActive(false),
      _currentTheme(UITheme::DARK)  // Default to dark theme
{
    g_ui = this;
    // Initialize display interface if display is available
    ICacheDisplay* disp = &_controller.getDisplay();
    if (disp) {
        _displayInterface = disp;
        _uiApp = new SimpleUIApp(_displayInterface);
        ESP_LOGI(TAG, "Display interface and UI app initialized");
    } else {
        ESP_LOGW(TAG, "No display available for UI");
    }
    
    // Set initial theme
    setTheme(_currentTheme);
    
    // Initialize string change detectors with callbacks
    _timeChangeDetector.setOnChange([this](const String& newTime) {
        if (_connectedPageWidgets.timeLabel) {
            ESP_LOGD(TAG, "Time changed to: %s", newTime.c_str());
            _connectedPageWidgets.timeLabel->setText(newTime);
        }
    });
    
    _pcStatusChangeDetector.setOnChange([this](const String& newStatus) {
        if (_connectedPageWidgets.pcStatus) {
            _connectedPageWidgets.pcStatus->setText(newStatus);
            bool isPowered = (newStatus.indexOf("ON") != -1);
            _connectedPageWidgets.pcStatus->setTextColor(isPowered ? COLOR_STATUS_ONLINE : COLOR_STATUS_OFFLINE);
        }
    });
    
    _tempChangeDetector.setOnChange([this](const String& newTemp) {
        if (_connectedPageWidgets.tempValue) {
            _connectedPageWidgets.tempValue->setText(newTemp);
        }
    });
    
    _humidityChangeDetector.setOnChange([this](const String& newHumidity) {
        if (_connectedPageWidgets.humidityValue) {
            _connectedPageWidgets.humidityValue->setText(newHumidity);
        }
    });
    
    _fanSpeedChangeDetector.setOnChange([this](const String& newFanSpeed) {
        if (_connectedPageWidgets.fanValue) {
            _connectedPageWidgets.fanValue->setText(newFanSpeed);
        }
    });
    
    _ipChangeDetector.setOnChange([this](const String& newIP) {
        if (_connectedPageWidgets.ipAddress) {
            _connectedPageWidgets.ipAddress->setText(newIP);
        }
    });
    
    _wifiStatusChangeDetector.setOnChange([this](const String& newWifiStatus) {
        if (_connectedPageWidgets.wifiStatus) {
            ESP_LOGD(TAG, "WiFi status changed to: %s", newWifiStatus.c_str());
            _connectedPageWidgets.wifiStatus->setText(newWifiStatus);
            bool isConnected = (newWifiStatus == StatusConstants::WIFI_ON);
            _connectedPageWidgets.wifiStatus->setTextColor(isConnected ? COLOR_STATUS_ONLINE : COLOR_STATUS_OFFLINE);
        }
    });
    
    _mqttStatusChangeDetector.setOnChange([this](const String& newMqttStatus) {
        if (_connectedPageWidgets.mqttStatus) {
            _connectedPageWidgets.mqttStatus->setText(newMqttStatus);
            bool isConnected = (newMqttStatus == StatusConstants::MQTT_ON);
            _connectedPageWidgets.mqttStatus->setTextColor(isConnected ? COLOR_STATUS_ONLINE : COLOR_STATUS_OFFLINE);
        }
    });
    
    _telegramStatusChangeDetector.setOnChange([this](const String& newTelegramStatus) {
        if (_connectedPageWidgets.telegramStatus) {
            _connectedPageWidgets.telegramStatus->setText(newTelegramStatus);
            bool isActive = (newTelegramStatus == StatusConstants::TG_ON);
            _connectedPageWidgets.telegramStatus->setTextColor(isActive ? COLOR_STATUS_ONLINE : COLOR_STATUS_OFFLINE);
        }
    });
    
    _webserverStatusChangeDetector.setOnChange([this](const String& newWebserverStatus) {
        if (_connectedPageWidgets.webserverStatus) {
            _connectedPageWidgets.webserverStatus->setText(newWebserverStatus);
            bool isRunning = (newWebserverStatus == StatusConstants::WS_ON);
            _connectedPageWidgets.webserverStatus->setTextColor(isRunning ? COLOR_STATUS_ONLINE : COLOR_STATUS_OFFLINE);
        }
    });
}

ComputerControllerUI::~ComputerControllerUI() {
    g_ui = nullptr;
}

// ---------------------------------------------------------------------------
// Public API
// ---------------------------------------------------------------------------
void ComputerControllerUI::begin() {
    if (!_uiApp) {
        ESP_LOGW(TAG, "UI app not initialized, cannot begin");
        return;
    }
    
    ESP_LOGI(TAG, "UI begin");
    createMessageSystem();
    setupTheme();
    setupLayouts();
    
    // Start with connecting page
    switchToPage(Page::CONNECTING);
}

void ComputerControllerUI::update() {
    if (!_uiApp) return;
    
    static uint32_t lastUpdateTime = 0;
    static uint32_t lastDrawTime = 0;
    static bool lastHasChanges = false;
    uint32_t currentTime = millis();
    
    // Log updates every 5 seconds to avoid spam
    if (currentTime - lastUpdateTime > 5000) {
        ESP_LOGD(TAG, "UI update - current page: %d, last changes: %s", (int)_currentPage, lastHasChanges ? "true" : "false");
        lastUpdateTime = currentTime;
    }
    
    updateMessage();
    
    // Only update page data every 200ms to reduce processing load (increased from 100ms)
    if (currentTime - lastDrawTime > 200) {
        bool hasChanges = updateCurrentPage();
        lastDrawTime = currentTime;
        
        // Only draw if there were actual changes AND the app is dirty
        if (hasChanges && _uiApp->isDirty()) {
            ESP_LOGD(TAG, "UI changes detected and app is dirty, drawing...");
            _uiApp->draw();
        } else if (hasChanges) {
            ESP_LOGD(TAG, "UI changes detected but app is not dirty, skipping draw");
        }
        
        lastHasChanges = hasChanges;
    }
}

void ComputerControllerUI::handleTouch(int16_t x, int16_t y, bool pressed) {
    if (!_uiApp) return;
    _uiApp->handleTouch(x, y, pressed);
}

// ---------------------------------------------------------------------------
// Message system
// ---------------------------------------------------------------------------
void ComputerControllerUI::showMessage(const String& message, uint32_t color, uint32_t durationMs) {
    if (!_messageLabel) return;
    _messageLabel->setText(message);
    _messageLabel->setVisible(true);
    _messageLabel->setTextColor(color);
    _messageStartTime = millis();
    _messageDuration = durationMs;
    _messageActive = true;
}

void ComputerControllerUI::updateMessage() {
    if (!_messageActive) return;
    if (millis() - _messageStartTime > _messageDuration) {
        _messageActive = false;
        if (_messageLabel) _messageLabel->setVisible(false);
    }
}

// ---------------------------------------------------------------------------
// UI Methods
// ---------------------------------------------------------------------------
void ComputerControllerUI::createMessageSystem() {
    if (!_uiApp) return;

    // If a previous label existed it has already been deleted by _uiApp->clear().
    // Simply allocate a fresh one and store the pointer.
    _messageLabel = new Label(10, 220, "");
    _messageLabel->setVisible(false);
    _uiApp->addWidget(_messageLabel);
}

void ComputerControllerUI::setupTheme() {
    // Basic theme setup - can be expanded later
    ESP_LOGI(TAG, "Setting up UI theme");
}

void ComputerControllerUI::setupLayouts() {
    // Basic layout setup - can be expanded later
    ESP_LOGI(TAG, "Setting up UI layouts");
}

// ---------------------------------------------------------------------------
// Page Management
// ---------------------------------------------------------------------------
void ComputerControllerUI::switchToPage(Page newPage) {
    if (_currentPage == newPage) return;
    
    ESP_LOGI(TAG, "Switching from page %d to page %d", (int)_currentPage, (int)newPage);
    
    // ------------------------------------------------------------------
    // Clear current page widgets and wipe the display to background color
    // so that stale pixels do not linger between page transitions.
    // ------------------------------------------------------------------
    if (_uiApp) {
        _uiApp->clear();
    }

    if (_displayInterface) {
        _displayInterface->fillScreen(COLOR_BACKGROUND);
    }
    
    _currentPage = newPage;
    
    // Create new page widgets
    switch (_currentPage) {
        case Page::AP_MODE:
            createAPModePage();
            break;
        case Page::CONNECTING:
            createConnectingPage();
            break;
        case Page::CONNECTED:
            createConnectedPage();
            break;
        case Page::ERROR:
            createErrorPage();
            break;
    }
}

bool ComputerControllerUI::updateCurrentPage() {
    switch (_currentPage) {
        case Page::AP_MODE:
            // Update AP mode page if needed
            return false;
        case Page::CONNECTING:
            // Update connecting page if needed
            return false;
        case Page::CONNECTED:
            // Update connected page with real-time data using StringChangeDetector for maximum performance
            return updateConnectedPageWithDetectors();
        case Page::ERROR:
            // Update error page if needed
            return false;
    }
    return false;
}

// ---------------------------------------------------------------------------
// Page Creation Methods
// ---------------------------------------------------------------------------
void ComputerControllerUI::createAPModePage() {
    if (!_uiApp) return;

    ESP_LOGI(TAG, "Creating AP Mode page");

    // Display dimensions
    const int16_t displayWidth  = _controller.getDisplay().getWidth();
    const int16_t displayHeight = _controller.getDisplay().getHeight();

    // Root panel covering the whole screen
    Panel* mainPanel = new Panel(0, 0, displayWidth, displayHeight);
    mainPanel->setColors(COLOR_BACKGROUND, COLOR_TEXT_PRIMARY); // Black background, white text
    _uiApp->addWidget(mainPanel);

    // Vertical layout that will stack all rows. We keep equalSpacing disabled so
    // fixed-height widgets keep their natural size and a flexible spacer can
    // absorb the remaining space.
    VerticalLayout* mainLayout = createVerticalLayout(0, 0, displayWidth, displayHeight, UI_MARGIN_MEDIUM, UI_SPACING_MEDIUM);
    mainLayout->setEqualSpacing(false);
    mainPanel->addChild(mainLayout);

    // ---------------------------------------------------------------------
    // Title row
    // ---------------------------------------------------------------------
    mainLayout->addLabel("WiFi Setup Mode", getPrimaryColor(), getTitleSize(), 0.0f);

    // Horizontal separator line
    addLine(mainLayout, displayWidth);

    // ---------------------------------------------------------------------
    // WiFi status row (Horizontal layout)
    // ---------------------------------------------------------------------
    HorizontalLayout* wifiLayout = createHorizontalLayout(0, 0, displayWidth - (UI_MARGIN_MEDIUM * 2), LABEL_HEIGHT_MEDIUM, 0, UI_SPACING_MEDIUM);
    wifiLayout->setEqualSpacing(false);
    mainLayout->addChild(wifiLayout);

    wifiLayout->addLabel("WiFi:", COLOR_TEXT_PRIMARY, TEXT_SIZE_HEADER, 0.0f);
    wifiLayout->addLabel("AP Mode", COLOR_STATUS_ONLINE, TEXT_SIZE_HEADER, 0.0f);

    // ---------------------------------------------------------------------
    // Network name row
    // ---------------------------------------------------------------------
    HorizontalLayout* networkLayout = createHorizontalLayout(0, 0, displayWidth - (UI_MARGIN_MEDIUM * 2), LABEL_HEIGHT_MEDIUM, 0, UI_SPACING_MEDIUM);
    networkLayout->setEqualSpacing(false);
    mainLayout->addChild(networkLayout);

    networkLayout->addLabel("Network:", COLOR_TEXT_PRIMARY, TEXT_SIZE_HEADER, 0.0f);
    networkLayout->addLabel("ComputerController", COLOR_STATUS_ONLINE, TEXT_SIZE_HEADER, 0.0f);

    // ---------------------------------------------------------------------
    // Password row
    // ---------------------------------------------------------------------
    HorizontalLayout* passwordLayout = createHorizontalLayout(0, 0, displayWidth - (UI_MARGIN_MEDIUM * 2), LABEL_HEIGHT_MEDIUM, 0, UI_SPACING_MEDIUM);
    passwordLayout->setEqualSpacing(false);
    mainLayout->addChild(passwordLayout);

    passwordLayout->addLabel("Password:", COLOR_TEXT_PRIMARY, TEXT_SIZE_HEADER, 0.0f);
    passwordLayout->addLabel("12345678", COLOR_TEXT_WARNING, TEXT_SIZE_HEADER, 0.0f);

    // ---------------------------------------------------------------------
    // IP address row
    // ---------------------------------------------------------------------
    HorizontalLayout* ipLayout = createHorizontalLayout(0, 0, displayWidth - (UI_MARGIN_MEDIUM * 2), LABEL_HEIGHT_MEDIUM, 0, UI_SPACING_MEDIUM);
    ipLayout->setEqualSpacing(false);
    mainLayout->addChild(ipLayout);

    ipLayout->addLabel("Visit:", COLOR_TEXT_PRIMARY, TEXT_SIZE_HEADER, 0.0f);
    ipLayout->addLabel("192.168.4.1", COLOR_STATUS_ONLINE, TEXT_SIZE_HEADER, 0.0f);

    // ---------------------------------------------------------------------
    // Status message row (store pointer for later updates)
    // ---------------------------------------------------------------------
    _apStatusLabel = new Label(UI_MARGIN_MEDIUM, 0, "Waiting for connection...", COLOR_STATUS_CONNECTING, TEXT_SIZE_HEADER);
    mainLayout->addChild(_apStatusLabel, 0.0f);

    // ---------------------------------------------------------------------
    // Flexible spacer to push content up; status message will sit at bottom
    // ---------------------------------------------------------------------
    mainLayout->addSpacer();
}

void ComputerControllerUI::createConnectingPage() {
    if (!_uiApp) return;
    
    ESP_LOGI(TAG, "Creating Connecting page");
    
    // Create main panel
    Panel* mainPanel = new Panel(0, 0, _controller.getDisplay().getWidth(), _controller.getDisplay().getHeight());
    mainPanel->setColors(COLOR_BACKGROUND, COLOR_TEXT_PRIMARY); // Black background, white text
    _uiApp->addWidget(mainPanel);
    
    // Title
    Label* titleLabel = new Label(UI_MARGIN_MEDIUM, UI_MARGIN_LARGE, "Connecting to WiFi", getPrimaryColor());
    titleLabel->setTextSize(getTitleSize());
    mainPanel->addChild(titleLabel);
    
    // WiFi status
    Label* wifiStatus = new Label(UI_MARGIN_MEDIUM, 60, "WiFi: Connecting", COLOR_STATUS_CONNECTING); // Yellow WiFi status
    wifiStatus->setTextSize(TEXT_SIZE_HEADER);
    mainPanel->addChild(wifiStatus);
    
    // Connecting animation text
    Label* connectingText = new Label(UI_MARGIN_MEDIUM, 120, "Connecting...", COLOR_STATUS_CONNECTING);
    connectingText->setTextSize(TEXT_SIZE_HEADER);
    mainPanel->addChild(connectingText);
    
    // Progress indicator
    ProgressBar* progressBar = new ProgressBar(UI_MARGIN_MEDIUM, 160, PROGRESS_BAR_WIDTH, PROGRESS_BAR_HEIGHT);
    // Note: ProgressBar doesn't have setColors method, colors are set in constructor
    progressBar->setProgress(50); // Indeterminate progress
    mainPanel->addChild(progressBar);
    
    // Status message
    Label* statusLabel = new Label(UI_MARGIN_MEDIUM, 200, "Please wait...", COLOR_TEXT_PRIMARY);
    statusLabel->setTextSize(TEXT_SIZE_BODY);
    mainPanel->addChild(statusLabel);
}

void ComputerControllerUI::createConnectedPage() {
    if (!_uiApp) return;

    const int16_t scrW = _controller.getDisplay().getWidth();
    const int16_t scrH = _controller.getDisplay().getHeight();

    // Main vertical layout covering the whole screen
    VerticalLayout* mainLayout = new VerticalLayout(0, 0, scrW, scrH, 0);
    mainLayout->setEqualSpacing(false);
    _uiApp->addWidget(mainLayout);

    // -------- Title Bar with "Computer Controller" and Time --------
    
    // Horizontal layout for title bar
    Cell* titleLayoutCell = mainLayout->addHorizontalLayout(0.0f, Gravity::FILL);

    // Get the HorizontalLayout from the cell
    HorizontalLayout* titleLayout = static_cast<HorizontalLayout*>(titleLayoutCell->getWidget());
    titleLayout->setEqualSpacing(false);

    // Title label (left-aligned) - "Computer Controller" with primary color
    Cell* titleCell = titleLayout->addLabel("Computer Controller", getPrimaryColor(), getTitleSize(), 0.0f, Gravity::CENTER_LEFT);

    // Time label (right-aligned, fixed width)
    Cell* timeCell = titleLayout->addLabel("00:00:00", getTextColor(), getHeaderSize(), 0.0f, Gravity::CENTER_RIGHT);

    // Store widget references for updates
    _connectedPageWidgets.timeLabel = static_cast<Label*>(timeCell->getWidget());

    // -------- Horizontal Line --------
    mainLayout->addHorizontalLine(getBorderColor(), 2);

    // -------- Two Rectangle Layouts Section --------
    Cell* rectanglesLayoutCell = mainLayout->addHorizontalLayout(5.0f, Gravity::FILL);
    HorizontalLayout* rectanglesLayout = static_cast<HorizontalLayout*>(rectanglesLayoutCell->getWidget());
    rectanglesLayout->setMargin(0);
    rectanglesLayout->setEqualSpacing(true);

    // Left rectangle panel with border - System Info Panel
    Panel* leftPanel = new Panel(0, 0, 0, 0); // Size will be determined by layout
    leftPanel->setColors(getBackgroundColor(), getBorderColor()); // Theme-aware colors
    leftPanel->setBorderThickness(2); // 2 pixel border thickness
    rectanglesLayout->addChild(leftPanel, 1.0f, Gravity::FILL);
    
    // Create a vertical layout inside the left panel for system information
    VerticalLayout* systemLayout = new VerticalLayout(0, 0, 0, 0, 10, 8); // 10px margin, 8px spacing
    systemLayout->setEqualSpacing(false);
    leftPanel->addChild(systemLayout, 1.0f, Gravity::FILL);
    
    // System info title
    systemLayout->addLabel("System Info", getPrimaryColor(), getHeaderSize(), 0.0f, Gravity::CENTER_LEFT);
    
    // Horizontal separator line
    systemLayout->addHorizontalLine(getBorderColor(), 2);
    
    // PC Status
    Cell* pcStatusCell = systemLayout->addLabel(String(StatusConstants::PC_PREFIX) + "OFF", getTextColor(), getLabelSize(), 0.0f, Gravity::CENTER_LEFT);
    _connectedPageWidgets.pcStatus = static_cast<Label*>(pcStatusCell->getWidget());
    
    // Temperature
    Cell* tempCell = systemLayout->addLabel(String(StatusConstants::TEMP_PREFIX) + "--", getTextColor(), getLabelSize(), 0.0f, Gravity::CENTER_LEFT);
    _connectedPageWidgets.tempValue = static_cast<Label*>(tempCell->getWidget());
    
    // Humidity
    Cell* humidityCell = systemLayout->addLabel(String(StatusConstants::HUMIDITY_PREFIX) + "--", getTextColor(), getLabelSize(), 0.0f, Gravity::CENTER_LEFT);
    _connectedPageWidgets.humidityValue = static_cast<Label*>(humidityCell->getWidget());
    
    // Fan Speed
    Cell* fanCell = systemLayout->addLabel(String(StatusConstants::FAN_PREFIX) + "--", getTextColor(), getLabelSize(), 0.0f, Gravity::CENTER_LEFT);
    _connectedPageWidgets.fanValue = static_cast<Label*>(fanCell->getWidget());
    
    // IP Address
    Cell* ipCell = systemLayout->addLabel(String(StatusConstants::IP_PREFIX) + "N/A", getTextColor(), getLabelSize(), 0.0f, Gravity::CENTER_LEFT);
    _connectedPageWidgets.ipAddress = static_cast<Label*>(ipCell->getWidget());
    
    // Flexible spacer to push content to top
    systemLayout->addSpacer();
    
    // Right rectangle panel with border - Status Panel
    Panel* rightPanel = new Panel(0, 0, 0, 0); // Size will be determined by layout
    rightPanel->setColors(getBackgroundColor(), getBorderColor()); // Theme-aware colors
    rightPanel->setBorderThickness(2); // 2 pixel border thickness
    rectanglesLayout->addChild(rightPanel, 1.0f, Gravity::FILL);
    
    // Create a vertical layout inside the right panel for status items
    VerticalLayout* statusLayout = new VerticalLayout(0, 0, 0, 0, 10, 8); // 10px margin, 8px spacing
    statusLayout->setEqualSpacing(false);
    rightPanel->addChild(statusLayout, 1.0f, Gravity::FILL);
    
    // Status title
    statusLayout->addLabel("System Status", getPrimaryColor(), getHeaderSize(), 0.0f, Gravity::CENTER_LEFT);
    
    // Horizontal separator line
    statusLayout->addHorizontalLine(getBorderColor(), 2);

    // WiFi status - use compact label
    Cell* wifiLabelCell = statusLayout->addLabel(StatusConstants::WIFI_OFF, getTextColor(), getLabelSize(), 0.0f, Gravity::CENTER_LEFT);
    _connectedPageWidgets.wifiStatus = static_cast<Label*>(wifiLabelCell->getWidget());
    
    // MQTT status
    Cell* mqttLabelCell = statusLayout->addLabel(StatusConstants::MQTT_OFF, getTextColor(), getLabelSize(), 0.0f, Gravity::CENTER_LEFT);
    _connectedPageWidgets.mqttStatus = static_cast<Label*>(mqttLabelCell->getWidget());
    
    // Telegram status
    Cell* telegramLabelCell = statusLayout->addLabel(StatusConstants::TG_OFF, getTextColor(), getLabelSize(), 0.0f, Gravity::CENTER_LEFT);
    _connectedPageWidgets.telegramStatus = static_cast<Label*>(telegramLabelCell->getWidget());
    
    // WebServer status
    Cell* webserverLabelCell = statusLayout->addLabel(StatusConstants::WS_OFF, getTextColor(), getLabelSize(), 0.0f, Gravity::CENTER_LEFT);
    _connectedPageWidgets.webserverStatus = static_cast<Label*>(webserverLabelCell->getWidget());
    
    // Flexible spacer to push content to top
    statusLayout->addSpacer();

    // -------- Bottom Buttons Section --------
    
    Cell* buttonsLayoutCell = mainLayout->addHorizontalLayout(0.0f, Gravity::FILL);
    HorizontalLayout* buttonsLayout = static_cast<HorizontalLayout*>(buttonsLayoutCell->getWidget());

    // Left button - Power button with green background
    buttonsLayout->addButton("Power", []() {
        ESP_LOGI("CC_UI", "Power pressed");
    }, getSuccessColor(), getBorderColor(), COLOR_BLACK, 1.0f, Gravity::CENTER, getButtonSize());

    // Right button - Reset button with yellow background
    buttonsLayout->addButton("Reset", []() {
        ESP_LOGI("CC_UI", "Reset pressed");
    }, getWarningColor(), getBorderColor(), COLOR_BLACK, 1.0f, Gravity::CENTER, getButtonSize());
}

void ComputerControllerUI::createErrorPage() {
    if (!_uiApp) return;

    ESP_LOGI(TAG, "Creating Error page");

    // Display dimensions
    const int16_t displayWidth  = _controller.getDisplay().getWidth();
    const int16_t displayHeight = _controller.getDisplay().getHeight();

    // Root panel covering the whole screen
    Panel* mainPanel = new Panel(0, 0, displayWidth, displayHeight);
    mainPanel->setColors(COLOR_BACKGROUND, COLOR_TEXT_PRIMARY); // Black background, white text
    _uiApp->addWidget(mainPanel);

    // Vertical layout that will stack all rows. We keep equalSpacing disabled so
    // fixed-height widgets keep their natural size and a flexible spacer can
    // absorb the remaining space.
    VerticalLayout* mainLayout = createVerticalLayout(0, 0, displayWidth, displayHeight, UI_MARGIN_MEDIUM, UI_SPACING_MEDIUM);
    mainLayout->setEqualSpacing(false);
    mainPanel->addChild(mainLayout);

    // ---------------------------------------------------------------------
    // Title row
    // ---------------------------------------------------------------------
    mainLayout->addLabel("ERROR", getErrorColor(), getTitleSize(), 0.0f); // Red title

    // Horizontal separator line
    addLine(mainLayout, displayWidth);

    // ---------------------------------------------------------------------
    // Error description rows
    // ---------------------------------------------------------------------
    mainLayout->addLabel("Connection Error", COLOR_TEXT_ERROR, TEXT_SIZE_HEADER, 0.0f);
    mainLayout->addLabel("Failed to connect to WiFi", COLOR_TEXT_PRIMARY, TEXT_SIZE_HEADER, 0.0f);
    mainLayout->addLabel("Check your network settings", COLOR_TEXT_PRIMARY, TEXT_SIZE_HEADER, 0.0f);

    // ---------------------------------------------------------------------
    // Flexible spacer to push the retry button to the bottom
    // ---------------------------------------------------------------------
    mainLayout->addSpacer();

    // ---------------------------------------------------------------------
    // Retry button row (centered horizontally)
    // ---------------------------------------------------------------------
    Button* retryButton = new Button(0, 0, BUTTON_WIDTH_FULL, BUTTON_HEIGHT_MEDIUM, "Retry");
    retryButton->setTextSize(TEXT_SIZE_BUTTON);
    retryButton->setColors(COLOR_BUTTON_PRIMARY, COLOR_BUTTON_BORDER, COLOR_BUTTON_TEXT); // Green button, black border, white text
    retryButton->setOnClick([this]() {
        switchToPage(Page::CONNECTING);
    });
    mainLayout->addChild(retryButton, 0.0f);
}

// Alternative implementation using StringChangeDetector utility
bool ComputerControllerUI::updateConnectedPageWithDetectors() {
    bool hasChanges = false;
    
    // Update time
    if (_connectedPageWidgets.timeLabel) {
        String currentTime = _controller.getCurrentTimeString();
        hasChanges |= _timeChangeDetector.checkAndUpdate(currentTime);
    }
    
    // Update PC status
    if (_connectedPageWidgets.pcStatus) {
        bool pcPowered = _controller.isPCPoweredOn();
        String currentStatus = String(StatusConstants::PC_PREFIX) + (pcPowered ? "ON" : "OFF");
        hasChanges |= _pcStatusChangeDetector.checkAndUpdate(currentStatus);
    }
    
    // Update temperature
    if (_connectedPageWidgets.tempValue) {
        String currentTemp = String(StatusConstants::TEMP_PREFIX) + formatTemperature(_controller.getAmbientTemperature());
        hasChanges |= _tempChangeDetector.checkAndUpdate(currentTemp);
    }
    
    // Update humidity
    if (_connectedPageWidgets.humidityValue) {
        String currentHumidity = String(StatusConstants::HUMIDITY_PREFIX) + formatHumidity(_controller.getRelativeHumidity());
        hasChanges |= _humidityChangeDetector.checkAndUpdate(currentHumidity);
    }
    
    // Update fan speed
    if (_connectedPageWidgets.fanValue) {
        String currentFanSpeed = String(StatusConstants::FAN_PREFIX) + formatFanSpeed(_controller.getGpuFanSpeed());
        hasChanges |= _fanSpeedChangeDetector.checkAndUpdate(currentFanSpeed);
    }
    
    // Update IP address
    if (_connectedPageWidgets.ipAddress) {
        String currentIP = String(StatusConstants::IP_PREFIX) + (WiFi.isConnected() ? WiFi.localIP().toString() : "N/A");
        hasChanges |= _ipChangeDetector.checkAndUpdate(currentIP);
    }
    
    // Update WiFi status
    if (_connectedPageWidgets.wifiStatus) {
        bool wifiConnected = WiFi.isConnected();
        String currentWifiStatus = wifiConnected ? StatusConstants::WIFI_ON : StatusConstants::WIFI_OFF;
        hasChanges |= _wifiStatusChangeDetector.checkAndUpdate(currentWifiStatus);
    }
    
    // Update MQTT status
    if (_connectedPageWidgets.mqttStatus) {
        bool mqttConnected = _controller.isMQTTConnected();
        String currentMqttStatus = mqttConnected ? StatusConstants::MQTT_ON : StatusConstants::MQTT_OFF;
        hasChanges |= _mqttStatusChangeDetector.checkAndUpdate(currentMqttStatus);
    }
    
    // Update Telegram status
    if (_connectedPageWidgets.telegramStatus) {
        // Check if Telegram bot is active by checking if WiFi is connected (prerequisite for Telegram)
        bool telegramActive = WiFi.isConnected();
        String currentTelegramStatus = telegramActive ? StatusConstants::TG_ON : StatusConstants::TG_OFF;
        hasChanges |= _telegramStatusChangeDetector.checkAndUpdate(currentTelegramStatus);
    }
    
    // Update WebServer status
    if (_connectedPageWidgets.webserverStatus) {
        // Check if WebServer is running by checking if WiFi is connected (prerequisite for WebServer)
        bool webserverRunning = WiFi.isConnected();
        String currentWebserverStatus = webserverRunning ? StatusConstants::WS_ON : StatusConstants::WS_OFF;
        hasChanges |= _webserverStatusChangeDetector.checkAndUpdate(currentWebserverStatus);
    }
    
    // Note: hasChanges tracks if any widgets were updated for potential future optimization
    // Currently all updates are processed, but this could be used to skip redraws when nothing changed
    return hasChanges;
}

// ---------------------------------------------------------------------------
// Helper functions
// ---------------------------------------------------------------------------
String ComputerControllerUI::formatTemperature(float temp) {
    if (isnan(temp)) return String("--");
    char buf[16]; snprintf(buf, sizeof(buf), "%.1fC", temp);
    return String(buf);
}

String ComputerControllerUI::formatHumidity(float h) {
    if (isnan(h)) return String("--");
    char buf[16]; snprintf(buf, sizeof(buf), "%.1f%%", h);
    return String(buf);
}

String ComputerControllerUI::formatTime() {
    return _controller.getCurrentTimeString();
}

String ComputerControllerUI::formatFanSpeed(uint8_t s) { 
    return String(s) + "%"; 
}

String ComputerControllerUI::formatFanRPM(uint16_t r) { 
    return String(r) + "RPM"; 
}

uint32_t ComputerControllerUI::getStatusColor(bool powered) { 
    return powered ? COLOR_STATUS_ONLINE : COLOR_STATUS_OFFLINE; 
}

// ---------------------------------------------------------------------------
// Layout Helper Methods
// ---------------------------------------------------------------------------
VerticalLayout* ComputerControllerUI::createVerticalLayout(int16_t x, int16_t y, int16_t w, int16_t h, 
                                                          int16_t margin, int16_t spacing) {
    return new VerticalLayout(x, y, w, h, margin, spacing);
}

HorizontalLayout* ComputerControllerUI::createHorizontalLayout(int16_t x, int16_t y, int16_t w, int16_t h,
                                                             int16_t margin, int16_t spacing) {
    return new HorizontalLayout(x, y, w, h, margin, spacing);
}

// ---------------------------------------------------------------------------
// UI Helper Methods
// ---------------------------------------------------------------------------
void ComputerControllerUI::addLine(VerticalLayout* layout, int16_t displayWidth, uint16_t color, uint8_t thickness) {
    if (!layout) return;
    
    // Calculate line width (full width minus panel margin)
    int16_t lineWidth = displayWidth - (UI_MARGIN_MEDIUM * 2);
    
    // Create horizontal line
    HorizontalLine* hLine = new HorizontalLine(UI_MARGIN_MEDIUM, 0, lineWidth, color, thickness);
    layout->addChild(hLine, 0.0f);
}

// ---------------------------------------------------------------------------
// Cache management
// ---------------------------------------------------------------------------
void ComputerControllerUI::preserveCache() {
    // Implementation for cache preservation
    ESP_LOGI(TAG, "Preserving LCD cache");
}

void ComputerControllerUI::testCachePersistence() {
    // Implementation for cache persistence testing
    ESP_LOGI(TAG, "Testing cache persistence");
}

// ---------------------------------------------------------------------------
// Complex GUI drawing
// ---------------------------------------------------------------------------
void ComputerControllerUI::drawComplexGUI() {
    // Check if display is available
    if (!_uiApp) return;
    
    // Create a simple test GUI
    const int16_t displayWidth = _controller.getDisplay().getWidth();
    const int16_t displayHeight = _controller.getDisplay().getHeight();
    
    if (displayWidth > 0 && displayHeight > 0) {
        Panel *root = new Panel(0, 0, displayWidth, displayHeight);
        root->setDisplayInterface(&_controller.getDisplay());
        
        // Add some test widgets
        Label *title = new Label(10, 10, "Computer Controller", 0xFFFF, 0x0000);
        title->setDisplayInterface(&_controller.getDisplay());
        
        Button *testBtn = new Button(10, 50, 100, 30, "Test Button");
        testBtn->setDisplayInterface(&_controller.getDisplay());
        testBtn->setColors(0x07E0, 0x0000, 0xFFFF);
        
        _uiApp->addWidget(root);
        _uiApp->addWidget(title);
        _uiApp->addWidget(testBtn);
    }
}

// Add implementation after other helper methods
void ComputerControllerUI::updateAPConnectionStatus(bool deviceConnected) {
    //if (_currentPage != Page::AP_MODE || !_apStatusLabel) return;
    if (deviceConnected) {
        _apStatusLabel->setText("Device connected to controller");
        _apStatusLabel->setTextColor(COLOR_STATUS_ONLINE); // Green
    } else {
        _apStatusLabel->setText("Waiting for connection...");
        _apStatusLabel->setTextColor(COLOR_STATUS_CONNECTING); // Yellow
    }
} 

// ---------------------------------------------------------------------------
// Theme Management
// ---------------------------------------------------------------------------

void ComputerControllerUI::setTheme(UITheme theme) {
    _currentTheme = theme;
    
    switch (theme) {
        case UITheme::LIGHT:
            Theme::setTheme(Theme::Light);
            break;
        case UITheme::DARK:
            Theme::setTheme(Theme::Dark);
            break;
        case UITheme::BLUE_THEME:
            Theme::setTheme(Theme::Blue);
            break;
        case UITheme::GREEN_THEME:
            Theme::setTheme(Theme::Green);
            break;
        case UITheme::CUSTOM:
            // Custom theme should be set via setCustomTheme()
            break;
    }
    
    ESP_LOGI(TAG, "UI theme changed to %d", static_cast<int>(theme));
}

void ComputerControllerUI::setCustomTheme(const Theme::ColorScheme& customTheme) {
    _currentTheme = UITheme::CUSTOM;
    Theme::setTheme(customTheme);
    ESP_LOGI(TAG, "Custom UI theme applied");
}

uint16_t ComputerControllerUI::getThemeColor(const char* colorType) const {
    // Map theme colors to existing color constants for backward compatibility
    if (strcmp(colorType, "primary") == 0) return getPrimaryColor();
    if (strcmp(colorType, "secondary") == 0) return getSecondaryColor();
    if (strcmp(colorType, "background") == 0) return getBackgroundColor();
    if (strcmp(colorType, "text") == 0) return getTextColor();
    if (strcmp(colorType, "border") == 0) return getBorderColor();
    if (strcmp(colorType, "success") == 0) return getSuccessColor();
    if (strcmp(colorType, "warning") == 0) return getWarningColor();
    if (strcmp(colorType, "error") == 0) return getErrorColor();
    
    // Default fallback
    return COLOR_TEXT_PRIMARY;
}

uint16_t ComputerControllerUI::getPrimaryColor() const {
    return Theme::getPrimary().getValue();
}

uint16_t ComputerControllerUI::getSecondaryColor() const {
    return Theme::getSecondary().getValue();
}

uint16_t ComputerControllerUI::getBackgroundColor() const {
    return Theme::getBackground().getValue();
}

uint16_t ComputerControllerUI::getTextColor() const {
    return Theme::getText().getValue();
}

uint16_t ComputerControllerUI::getBorderColor() const {
    return Theme::getBorder().getValue();
}

uint16_t ComputerControllerUI::getSuccessColor() const {
    return Theme::getSuccess().getValue();
}

uint16_t ComputerControllerUI::getWarningColor() const {
    return Theme::getWarning().getValue();
}

uint16_t ComputerControllerUI::getErrorColor() const {
    return Theme::getError().getValue();
}

// ---------------------------------------------------------------------------
// Theme-aware text size getters
// ---------------------------------------------------------------------------

uint8_t ComputerControllerUI::getTitleSize() const {
    return Theme::getTitleSize();
}

uint8_t ComputerControllerUI::getHeaderSize() const {
    return Theme::getHeaderSize();
}

uint8_t ComputerControllerUI::getBodySize() const {
    return Theme::getBodySize();
}

uint8_t ComputerControllerUI::getButtonSize() const {
    return Theme::getButtonSize();
}

uint8_t ComputerControllerUI::getStatusSize() const {
    return Theme::getStatusSize();
}

uint8_t ComputerControllerUI::getLabelSize() const {
    return Theme::getLabelSize();
}

uint8_t ComputerControllerUI::getValueSize() const {
    return Theme::getValueSize();
}

uint8_t ComputerControllerUI::getCaptionSize() const {
    return Theme::getCaptionSize();
}