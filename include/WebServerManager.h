#pragma once

#include "Globals.h"
#include <WebServer.h>
#include <ArduinoJson.h>

// Forward declaration
class ComputerController;

/**
 * @brief Web server manager for settings and status endpoints.
 * 
 * Provides a web interface for viewing system status and modifying settings.
 * Status updates are provided every second via Server-Sent Events (SSE).
 */
class WebServerManager {
public:
    /**
     * @brief Constructor for WebServerManager.
     * @param controller Reference to the main ComputerController instance.
     */
    explicit WebServerManager(ComputerController& controller);
    
    /**
     * @brief Destructor for WebServerManager.
     */
    ~WebServerManager() = default;
    
    /**
     * @brief Initializes the web server and sets up routes.
     */
    void begin();
    
    /**
     * @brief Main loop function for the web server.
     * Must be called repeatedly to handle client requests.
     */
    void loop();
    
    /**
     * @brief Gets the web server instance.
     * @return Reference to the WebServer object.
     */
    WebServer& getServer() { return server; }

private:
    WebServer server;                      ///< Web server instance
    ComputerController& controller;        ///< Reference to the main controller
    
    // Route handlers
    void handleRoot();
    void handleStatus();
    void handleSettings();
    void handleUpdateSettings();
    void handleControl();
    void handleSSE();
    void handleTest();
    void handleNotFound();
    
    // Helper methods
    void sendSettingsJSON();
    String getStatusJSON();
    String getSettingsJSON();
    void updateSettingsFromJSON(const JsonDocument& doc);
    void handleControlAction(const String& action);
    
#define WEB_SERVER_PORT 80           ///< Web server port
#define JSON_BUFFER_SIZE 2048          ///< JSON buffer size for responses
}; 