#pragma once

#include "ComputerController.h"
#include "Globals.h"
#include "SimpleUI/SimpleUI.h"
#include "SimpleUI/Theme.h"
#include "StringChangeDetector.h"

// Status constants with ON/OFF values that can be colored
namespace StatusConstants {
    extern const char* WIFI_ON;
    extern const char* WIFI_OFF;
    extern const char* MQTT_ON;
    extern const char* MQTT_OFF;
    extern const char* TG_ON;
    extern const char* TG_OFF;
    extern const char* WS_ON;
    extern const char* WS_OFF;

    // Label prefixes for left panel
    constexpr const char* PC_PREFIX = "PC: ";
    constexpr const char* TEMP_PREFIX = "Temp: ";
    constexpr const char* HUMIDITY_PREFIX = "Humidity: ";
    constexpr const char* FAN_PREFIX = "Fan: ";
    constexpr const char* IP_PREFIX = "IP: ";
}

class ComputerController;

class ComputerControllerUI {
public:
    enum class Page {
        AP_MODE,
        CONNECTING,
        CONNECTED,
        ERROR
    };
    
    enum class UITheme {
        LIGHT,
        DARK,
        BLUE_THEME,  // Renamed to avoid macro conflict
        GREEN_THEME, // Renamed to avoid macro conflict
        CUSTOM
    };

private:
    ComputerController& _controller;
    ICacheDisplay* _displayInterface;
    SimpleUIApp* _uiApp;
    
    // Current page state
    Page _currentPage;
    
    // Message system
    Label* _messageLabel;
    uint32_t _messageStartTime;
    uint32_t _messageDuration;
    bool _messageActive;
    
    // AP mode status label
    Label* _apStatusLabel;
    
    // Current theme
    UITheme _currentTheme;
    
    // String change detectors for efficient updates
    StringChangeDetector _timeChangeDetector;
    StringChangeDetector _pcStatusChangeDetector;
    StringChangeDetector _tempChangeDetector;
    StringChangeDetector _humidityChangeDetector;
    StringChangeDetector _fanSpeedChangeDetector;
    StringChangeDetector _ipChangeDetector;
    StringChangeDetector _wifiStatusChangeDetector;
    StringChangeDetector _mqttStatusChangeDetector;
    StringChangeDetector _telegramStatusChangeDetector;
    StringChangeDetector _webserverStatusChangeDetector;
    
    // Connected page widgets for updates
    struct ConnectedPageWidgets {
        Label* timeLabel = nullptr;
        Label* pcStatus = nullptr;
        Label* tempValue = nullptr;
        Label* humidityValue = nullptr;
        Label* fanValue = nullptr;
        Label* ipAddress = nullptr;
        Label* wifiStatus = nullptr;
        Label* mqttStatus = nullptr;
        Label* telegramStatus = nullptr;
        Label* webserverStatus = nullptr;
    } _connectedPageWidgets;
    


public:
    ComputerControllerUI(ComputerController& controller);
    ~ComputerControllerUI();
    
    // Core UI lifecycle
    void begin();
    void update();
    void handleTouch(int16_t x, int16_t y, bool pressed);
    
    // Message system
    void showMessage(const String& message, uint32_t color = COLOR_TEXT_PRIMARY, uint32_t durationMs = 3000);
    
    // Page management
    void switchToPage(Page newPage);
    Page getCurrentPage() const { return _currentPage; }
    
    // Theme management
    void setTheme(UITheme theme);
    UITheme getCurrentTheme() const { return _currentTheme; }
    void setCustomTheme(const Theme::ColorScheme& customTheme);
    
    // Theme-aware color getters
    uint16_t getThemeColor(const char* colorType) const;
    uint16_t getPrimaryColor() const;
    uint16_t getSecondaryColor() const;
    uint16_t getBackgroundColor() const;
    uint16_t getTextColor() const;
    uint16_t getBorderColor() const;
    uint16_t getSuccessColor() const;
    uint16_t getWarningColor() const;
    uint16_t getErrorColor() const;
    
    // Theme-aware text size getters
    uint8_t getTitleSize() const;
    uint8_t getHeaderSize() const;
    uint8_t getBodySize() const;
    uint8_t getButtonSize() const;
    uint8_t getStatusSize() const;
    uint8_t getLabelSize() const;
    uint8_t getValueSize() const;
    uint8_t getCaptionSize() const;

    // AP mode connection status
    void updateAPConnectionStatus(bool deviceConnected);

private:
    // UI Methods
    void createMessageSystem();
    void setupTheme();
    void setupLayouts();
    void updateMessage();
    
    // Page creation methods
    void createAPModePage();
    void createConnectingPage();
    void createConnectedPage();
    void createErrorPage();
    bool updateCurrentPage();
    bool updateConnectedPageWithDetectors();  // Alternative using StringChangeDetector
    
    // Helper functions
    String formatTemperature(float temp);
    String formatHumidity(float h);
    String formatTime();
    String formatFanSpeed(uint8_t s);
    String formatFanRPM(uint16_t r);
    uint32_t getStatusColor(bool powered);
    

    
    // Layout Helper Methods
    VerticalLayout* createVerticalLayout(int16_t x, int16_t y, int16_t w, int16_t h, 
                                        int16_t margin = 10, int16_t spacing = 5);
    HorizontalLayout* createHorizontalLayout(int16_t x, int16_t y, int16_t w, int16_t h,
                                           int16_t margin = 10, int16_t spacing = 5);
    
    // UI Helper Methods
    void addLine(VerticalLayout* layout, int16_t displayWidth, uint16_t color = COLOR_BORDER, uint8_t thickness = 1);
    
    // Cache management
    void preserveCache();
    void testCachePersistence();
    
    // Complex GUI drawing
    void drawComplexGUI();
}; 