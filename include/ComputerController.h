#pragma once

// Standard library includes
#include <stdint.h>

// ESP32 core includes
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <SPI.h>

// Third-party library includes
#include <UniversalTelegramBot.h>
#include <WiFiManager.h>
#include <ESP32Time.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <SimpleTimer.h>

// Project includes
#include "Globals.h"
#include "CommandHandler.h"
#include "DisplayManager.h"
#include "ButtonController.h"

class ComputerController final {
public:    
    ComputerController();
    void setup();
    void loop();
    void reset();
    
    UniversalTelegramBot& getTelegramBot() { return telegramBot; }
    WiFiClientSecure& getTelegramClient() { return telegramClient; }
    void handleCommand(const String& command);
    void sendTelegramMessage(const String& message);
    void updateDisplay();

private:
    // Command channels
    WiFiClientSecure telegramClient;
    UniversalTelegramBot telegramBot;

    // Command handler
    CommandHandler* commandHandler;

    // Timers
    SimpleTimer<> wifiCheckTimer;
    SimpleTimer<> debugTimer;
    SimpleTimer<> displayUpdateTimer;

    // Hardware controllers
    WiFiManager wifiManager;
    ESP32Time rtc;
    DisplayManager display;
    ButtonController button;

    bool isConnected;
    unsigned long lastTelegramCheck;
    unsigned long lastDisplayUpdate;
    static const unsigned long TELEGRAM_CHECK_INTERVAL = 1000; // 1 second
    static const unsigned long DISPLAY_UPDATE_INTERVAL = 1000; // 1 second

    void connectWiFi();
}; 