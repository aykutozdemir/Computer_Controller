#include "WebServerManager.h"
#include "ComputerController.h"
#include "PersistentSettings.h"
#include <esp_log.h>

// Define log tag
static const char* TAG = "WebServerManager";

WebServerManager::WebServerManager(ComputerController& controller) 
    : server(WEB_SERVER_PORT)
    , controller(controller) {
    ESP_LOGI(TAG, "WebServerManager initialized");
}

void WebServerManager::begin() {
    ESP_LOGI(TAG, "Starting web server on port %d", WEB_SERVER_PORT);
    
    // Add CORS headers to all responses
    server.enableCORS(true);
    
    // Set up routes
    server.on("/", HTTP_GET, [this]() { 
        ESP_LOGI(TAG, "Root request received");
        handleRoot(); 
    });
    server.on("/status", HTTP_GET, [this]() { 
        ESP_LOGI(TAG, "Status request received");
        handleStatus(); 
    });
    server.on("/settings", HTTP_GET, [this]() { 
        ESP_LOGI(TAG, "Settings GET request received");
        handleSettings(); 
    });
    server.on("/settings", HTTP_POST, [this]() { 
        ESP_LOGI(TAG, "Settings POST request received");
        handleUpdateSettings(); 
    });
    server.on("/control", HTTP_POST, [this]() { 
        ESP_LOGI(TAG, "Control request received");
        handleControl(); 
    });
    server.on("/sse", HTTP_GET, [this]() { 
        ESP_LOGI(TAG, "SSE request received");
        handleSSE(); 
    });
    server.on("/test", HTTP_GET, [this]() { 
        ESP_LOGI(TAG, "Test request received");
        handleTest(); 
    });
    server.onNotFound([this]() { handleNotFound(); });
    
    // Start the server
    server.begin();
    ESP_LOGI(TAG, "Web server started successfully");
}

void WebServerManager::loop() {
    server.handleClient();
}

void WebServerManager::handleRoot() {
    ESP_LOGI(TAG, "Handling root request");
    
    String html = "<!DOCTYPE html>"
        "<html lang=\"en\">"
        "<head>"
            "<meta charset=\"UTF-8\">"
            "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">"
            "<title>Computer Controller</title>"
            "<style>"
                "body { font-family: Arial, sans-serif; max-width: 800px; margin: 0 auto; padding: 20px; background-color: #f5f5f5; }"
                ".container { background: white; padding: 20px; border-radius: 8px; box-shadow: 0 2px 4px rgba(0,0,0,0.1); margin-bottom: 20px; }"
                "h1, h2 { color: #333; }"
                ".status-grid { display: grid; grid-template-columns: repeat(auto-fit, minmax(200px, 1fr)); gap: 15px; margin: 20px 0; }"
                ".status-item { background: #f8f9fa; padding: 15px; border-radius: 5px; border-left: 4px solid #007bff; }"
                ".status-label { font-weight: bold; color: #666; font-size: 0.9em; }"
                ".status-value { font-size: 1.2em; color: #333; margin-top: 5px; }"
                ".form-group { margin-bottom: 15px; }"
                "label { display: block; margin-bottom: 5px; font-weight: bold; }"
                "input[type=\"checkbox\"], input[type=\"number\"] { padding: 8px; border: 1px solid #ddd; border-radius: 4px; width: 100%; box-sizing: border-box; }"
                "button { background: #007bff; color: white; padding: 10px 20px; border: none; border-radius: 4px; cursor: pointer; }"
                "button:hover { background: #0056b3; }"
                ".nav-tabs { display: flex; border-bottom: 1px solid #ddd; margin-bottom: 20px; }"
                ".nav-tab { padding: 10px 20px; cursor: pointer; border: none; background: none; color: #666; }"
                ".nav-tab.active { color: #007bff; border-bottom: 2px solid #007bff; }"
                ".tab-content { display: none; }"
                ".tab-content.active { display: block; }"
                ".online { color: #28a745; }"
                ".offline { color: #dc3545; }"
            "</style>"
        "</head>"
        "<body>"
            "<h1>Computer Controller</h1>"
            "<div class=\"nav-tabs\">"
                "<button class=\"nav-tab active\" onclick=\"showTab('status')\">Status</button>"
                "<button class=\"nav-tab\" onclick=\"showTab('settings')\">Settings</button>"
            "</div>"
            "<div id=\"status-tab\" class=\"tab-content active\">"
                "<div class=\"container\">"
                    "<h2>System Status</h2>"
                    "<div class=\"status-grid\" id=\"status-grid\">"
                        "<div class=\"status-item\">"
                            "<div class=\"status-label\">WiFi Status</div>"
                            "<div class=\"status-value\" id=\"wifi-status\">Loading...</div>"
                        "</div>"
                        "<div class=\"status-item\">"
                            "<div class=\"status-label\">PC Power Status</div>"
                            "<div class=\"status-value\" id=\"pc-power\">Loading...</div>"
                        "</div>"
                        "<div class=\"status-item\">"
                            "<div class=\"status-label\">Temperature</div>"
                            "<div class=\"status-value\" id=\"temperature\">Loading...</div>"
                        "</div>"
                        "<div class=\"status-item\">"
                            "<div class=\"status-label\">Humidity</div>"
                            "<div class=\"status-value\" id=\"humidity\">Loading...</div>"
                        "</div>"
                        "<div class=\"status-item\">"
                            "<div class=\"status-label\">GPU Fan Speed</div>"
                            "<div class=\"status-value\" id=\"gpu-fan-speed\">Loading...</div>"
                        "</div>"
                        "<div class=\"status-item\">"
                            "<div class=\"status-label\">GPU Fan RPM</div>"
                            "<div class=\"status-value\" id=\"gpu-fan-rpm\">Loading...</div>"
                        "</div>"
                        "<div class=\"status-item\">"
                            "<div class=\"status-label\">Buzzer Enabled</div>"
                            "<div class=\"status-value\" id=\"buzzer-enabled\">Loading...</div>"
                        "</div>"
                        "<div class=\"status-item\">"
                            "<div class=\"status-label\">RF Enabled</div>"
                            "<div class=\"status-value\" id=\"rf-enabled\">Loading...</div>"
                        "</div>"
                        "<div class=\"status-item\">"
                            "<div class=\"status-label\">Free Heap</div>"
                            "<div class=\"status-value\" id=\"free-heap\">Loading...</div>"
                        "</div>"
                        "<div class=\"status-item\">"
                            "<div class=\"status-label\">Uptime</div>"
                            "<div class=\"status-value\" id=\"uptime\">Loading...</div>"
                        "</div>"
                    "</div>"
                    "<h3>Power Control</h3>"
                    "<div style=\"display: flex; gap: 10px; margin: 20px 0;\">"
                        "<button onclick=\"powerControl('power')\" style=\"background: #28a745;\">Power On/Off</button>"
                        "<button onclick=\"powerControl('reset')\" style=\"background: #ffc107; color: #000;\">Reset PC</button>"
                        "<button onclick=\"powerControl('beep')\" style=\"background: #17a2b8;\">Test Buzzer</button>"
                    "</div>"
                "</div>"
            "</div>"
            "<div id=\"settings-tab\" class=\"tab-content\">"
                "<div class=\"container\">"
                    "<h2>Settings</h2>"
                    "<form id=\"settings-form\">"
                        "<div class=\"form-group\">"
                            "<label for=\"buzzer-enabled-setting\">Buzzer Enabled:</label>"
                            "<input type=\"checkbox\" id=\"buzzer-enabled-setting\" name=\"buzzerEnabled\">"
                        "</div>"
                        "<div class=\"form-group\">"
                            "<label for=\"rf-enabled-setting\">RF Enabled:</label>"
                            "<input type=\"checkbox\" id=\"rf-enabled-setting\" name=\"rfEnabled\">"
                        "</div>"
                        "<div class=\"form-group\">"
                            "<label for=\"child-lock-setting\">Child Lock:</label>"
                            "<input type=\"checkbox\" id=\"child-lock-setting\" name=\"childLock\">"
                        "</div>"
                        "<div class=\"form-group\">"
                            "<label for=\"gpu-fan-speed-setting\">GPU Fan Speed (%):</label>"
                            "<input type=\"number\" id=\"gpu-fan-speed-setting\" name=\"gpuFanSpeed\" min=\"0\" max=\"100\">"
                        "</div>"
                        "<button type=\"submit\">Save Settings</button>"
                    "</form>"
                "</div>"
            "</div>"
            "<script>"
                "let statusUpdateInterval;"
                "function showTab(tabName) {"
                    "document.querySelectorAll('.tab-content').forEach(tab => { tab.classList.remove('active'); });"
                    "document.querySelectorAll('.nav-tab').forEach(tab => { tab.classList.remove('active'); });"
                    "document.getElementById(tabName + '-tab').classList.add('active');"
                    "event.target.classList.add('active');"
                "}"
                "function loadSettings() {"
                    "fetch('/settings')"
                    ".then(response => response.json())"
                    ".then(data => {"
                        "document.getElementById('buzzer-enabled-setting').checked = data.buzzerEnabled;"
                        "document.getElementById('rf-enabled-setting').checked = data.rfEnabled;"
                        "document.getElementById('child-lock-setting').checked = data.childLock;"
                        "document.getElementById('gpu-fan-speed-setting').value = data.gpuFanSpeed;"
                    "})"
                    ".catch(error => console.error('Error loading settings:', error));"
                "}"
                "function updateStatus(data) {"
                    "console.log('Updating status with data:', data);"
                    "try {"
                        "document.getElementById('wifi-status').textContent = data.wifiConnected ? 'Connected' : 'Disconnected';"
                        "document.getElementById('wifi-status').className = 'status-value ' + (data.wifiConnected ? 'online' : 'offline');"
                        "document.getElementById('pc-power').textContent = data.pcPoweredOn ? 'ON' : 'OFF';"
                        "document.getElementById('pc-power').className = 'status-value ' + (data.pcPoweredOn ? 'online' : 'offline');"
                        "document.getElementById('temperature').textContent = isNaN(data.temperature) ? 'N/A' : data.temperature + '°C';"
                        "document.getElementById('humidity').textContent = isNaN(data.humidity) ? 'N/A' : data.humidity + '%';"
                        "document.getElementById('gpu-fan-speed').textContent = data.gpuFanSpeed + '%';"
                        "document.getElementById('gpu-fan-rpm').textContent = data.gpuFanRpm + ' RPM';"
                        "document.getElementById('buzzer-enabled').textContent = data.buzzerEnabled ? 'Yes' : 'No';"
                        "document.getElementById('rf-enabled').textContent = data.rfEnabled ? 'Yes' : 'No';"
                        "document.getElementById('free-heap').textContent = Math.round(data.freeHeap / 1024) + ' KB';"
                        "document.getElementById('uptime').textContent = Math.round(data.uptime / 1000) + 's';"
                    "} catch (error) {"
                        "console.error('Error updating status:', error);"
                    "}"
                "}"
                "function powerControl(action) {"
                    "fetch('/control', {"
                        "method: 'POST',"
                        "headers: { 'Content-Type': 'application/json' },"
                        "body: JSON.stringify({action: action})"
                    "})"
                    ".then(response => response.json())"
                    ".then(data => {"
                        "if (data.success) { console.log('Control action executed:', action); }"
                        "else { alert('Error executing control action: ' + data.error); }"
                    "})"
                    ".catch(error => {"
                        "console.error('Error:', error);"
                        "alert('Error executing control action');"
                    "});"
                "}"
                "function loadStatus() {"
                    "console.log('Loading status...');"
                    "fetch('/status')"
                    ".then(response => {"
                        "console.log('Status response received:', response.status);"
                        "if (!response.ok) { throw new Error('HTTP ' + response.status); }"
                        "return response.json();"
                    "})"
                    ".then(data => {"
                        "console.log('Status data received:', data);"
                        "updateStatus(data);"
                        "console.log('Status updated successfully');"
                    "})"
                    ".catch(error => {"
                        "console.error('Error loading status:', error);"
                        "document.getElementById('wifi-status').textContent = 'Error loading status';"
                        "document.getElementById('wifi-status').className = 'status-value offline';"
                    "});"
                "}"
                "function startStatusUpdates() {"
                    "console.log('Starting status updates...');"
                    "loadStatus();"
                    "statusUpdateInterval = setInterval(loadStatus, 5000);"
                    "console.log('Status update interval set to 5 seconds');"
                "}"
                "document.getElementById('settings-form').addEventListener('submit', function(e) {"
                    "e.preventDefault();"
                    "const formData = new FormData(e.target);"
                    "const settings = {"
                        "buzzerEnabled: formData.get('buzzerEnabled') === 'on',"
                        "rfEnabled: formData.get('rfEnabled') === 'on',"
                        "childLock: formData.get('childLock') === 'on',"
                        "gpuFanSpeed: parseInt(formData.get('gpuFanSpeed')) || 0"
                    "};"
                    "fetch('/settings', {"
                        "method: 'POST',"
                        "headers: { 'Content-Type': 'application/json' },"
                        "body: JSON.stringify(settings)"
                    "})"
                    ".then(response => response.json())"
                    ".then(data => {"
                        "if (data.success) { alert('Settings saved successfully!'); }"
                        "else { alert('Error saving settings: ' + data.error); }"
                    "})"
                    ".catch(error => {"
                        "console.error('Error:', error);"
                        "alert('Error saving settings');"
                    "});"
                "});"
                "loadSettings();"
                "startStatusUpdates();"
                "console.log('Page loaded, initialization complete');"
            "</script>"
        "</body>"
        "</html>";
    
    server.send(200, "text/html", html);
}

void WebServerManager::handleStatus() {
    ESP_LOGI(TAG, "Handling status request");
    
    try {
        String json = getStatusJSON();
        ESP_LOGD(TAG, "Sending status response: %s", json.c_str());
        
        // Add CORS headers
        server.sendHeader("Access-Control-Allow-Origin", "*");
        server.sendHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
        server.sendHeader("Access-Control-Allow-Headers", "Content-Type");
        
        server.send(200, "application/json", json);
    } catch (const std::exception& e) {
        ESP_LOGE(TAG, "Exception in handleStatus: %s", e.what());
        server.send(500, "application/json", "{\"error\":\"Internal server error\"}");
    } catch (...) {
        ESP_LOGE(TAG, "Unknown exception in handleStatus");
        server.send(500, "application/json", "{\"error\":\"Internal server error\"}");
    }
}

void WebServerManager::handleSettings() {
    ESP_LOGI(TAG, "Handling settings request");
    sendSettingsJSON();
}

void WebServerManager::handleUpdateSettings() {
    ESP_LOGI(TAG, "Handling settings update request");
    
    if (server.hasArg("plain")) {
        String body = server.arg("plain");
        ESP_LOGI(TAG, "Received settings update: %s", body.c_str());
        
        DynamicJsonDocument doc(JSON_BUFFER_SIZE);
        DeserializationError error = deserializeJson(doc, body);
        
        if (error) {
            ESP_LOGE(TAG, "JSON parsing failed: %s", error.c_str());
            server.send(400, "application/json", "{\"success\":false,\"error\":\"Invalid JSON\"}");
            return;
        }
        
        updateSettingsFromJSON(doc);
        server.send(200, "application/json", "{\"success\":true}");
    } else {
        server.send(400, "application/json", "{\"success\":false,\"error\":\"No data received\"}");
    }
}

void WebServerManager::handleControl() {
    ESP_LOGI(TAG, "Handling control request");
    
    if (server.hasArg("plain")) {
        String body = server.arg("plain");
        ESP_LOGI(TAG, "Received control command: %s", body.c_str());
        
        DynamicJsonDocument doc(JSON_BUFFER_SIZE);
        DeserializationError error = deserializeJson(doc, body);
        
        if (error) {
            ESP_LOGE(TAG, "JSON parsing failed: %s", error.c_str());
            server.send(400, "application/json", "{\"success\":false,\"error\":\"Invalid JSON\"}");
            return;
        }
        
        if (doc.containsKey("action")) {
            String action = doc["action"];
            handleControlAction(action);
            server.send(200, "application/json", "{\"success\":true}");
        } else {
            server.send(400, "application/json", "{\"success\":false,\"error\":\"No action specified\"}");
        }
    } else {
        server.send(400, "application/json", "{\"success\":false,\"error\":\"No data received\"}");
    }
}

void WebServerManager::handleSSE() {
    ESP_LOGI(TAG, "SSE endpoint called - redirecting to status");
    // Redirect SSE requests to regular status endpoint for simplicity
    handleStatus();
}

void WebServerManager::handleTest() {
    ESP_LOGI(TAG, "Test endpoint called");
    
    String html = "Test page is working!";
    
    server.send(200, "text/html", html);
}

void WebServerManager::handleNotFound() {
    ESP_LOGI(TAG, "404 - Not found: %s", server.uri().c_str());
    server.send(404, "text/plain", "Not found");
}

void WebServerManager::sendSettingsJSON() {
    String json = getSettingsJSON();
    server.send(200, "application/json", json);
}

String WebServerManager::getStatusJSON() {
    DynamicJsonDocument doc(JSON_BUFFER_SIZE);
    
    try {
        doc["wifiConnected"] = WiFi.status() == WL_CONNECTED;
        doc["pcPoweredOn"] = controller.isPCPoweredOn();
        float temperature = controller.getAmbientTemperature();
        if (!isnan(temperature)) {
            char tempStr[8];
            snprintf(tempStr, sizeof(tempStr), "%.2f", temperature);
            doc["temperature"] = String(tempStr);
        }
        float humidity = controller.getRelativeHumidity();
        if (!isnan(humidity)) {
            char humStr[8];
            snprintf(humStr, sizeof(humStr), "%.2f", humidity);
            doc["humidity"] = String(humStr);
        }
        doc["gpuFanSpeed"] = controller.getGpuFanSpeed();
        doc["gpuFanRpm"] = controller.getGpuFanRPM();
        doc["gpuFanEnabled"] = controller.isGpuFanEnabled();
        doc["buzzerEnabled"] = controller.getBuzzer().isEnabled();
        
        PersistentSettings& settings = PersistentSettings::getInstance();
        doc["rfEnabled"] = settings.isRFEnabled();
        doc["childLock"] = settings.isChildLockEnabled();
        
        doc["uptime"] = millis();
        doc["freeHeap"] = ESP.getFreeHeap();
        doc["timestamp"] = millis();
        
        String json;
        serializeJson(doc, json);
        
        ESP_LOGD(TAG, "Generated status JSON: %s", json.c_str());
        return json;
    } catch (const std::exception& e) {
        ESP_LOGE(TAG, "Exception in getStatusJSON: %s", e.what());
        return "{\"error\":\"Failed to generate status\"}";
    } catch (...) {
        ESP_LOGE(TAG, "Unknown exception in getStatusJSON");
        return "{\"error\":\"Failed to generate status\"}";
    }
}

String WebServerManager::getSettingsJSON() {
    DynamicJsonDocument doc(JSON_BUFFER_SIZE);
    
    PersistentSettings& settings = PersistentSettings::getInstance();
    doc["buzzerEnabled"] = settings.isBuzzerEnabled();
    doc["rfEnabled"] = settings.isRFEnabled();
    doc["childLock"] = settings.isChildLockEnabled();
    doc["gpuFanSpeed"] = controller.getGpuFanSpeed();
    
    String json;
    serializeJson(doc, json);
    return json;
}

void WebServerManager::updateSettingsFromJSON(const JsonDocument& doc) {
    PersistentSettings& settings = PersistentSettings::getInstance();
    
    if (doc.containsKey("buzzerEnabled")) {
        bool buzzerEnabled = doc["buzzerEnabled"];
        settings.setBuzzerEnabled(buzzerEnabled);
        controller.getBuzzer().setEnabled(buzzerEnabled);
        ESP_LOGI(TAG, "Buzzer enabled: %s", buzzerEnabled ? "true" : "false");
    }
    
    if (doc.containsKey("rfEnabled")) {
        bool rfEnabled = doc["rfEnabled"];
        settings.setRFEnabled(rfEnabled);
        ESP_LOGI(TAG, "RF enabled: %s", rfEnabled ? "true" : "false");
    }
    
    if (doc.containsKey("childLock")) {
        bool childLock = doc["childLock"];
        settings.setChildLockEnabled(childLock);
        ESP_LOGI(TAG, "Child lock: %s", childLock ? "true" : "false");
    }
    
    if (doc.containsKey("gpuFanSpeed")) {
        uint8_t fanSpeed = doc["gpuFanSpeed"];
        controller.setGpuFanSpeed(fanSpeed);
        ESP_LOGI(TAG, "GPU fan speed set to: %d%%", fanSpeed);
    }
}

void WebServerManager::handleControlAction(const String& action) {
    ESP_LOGI(TAG, "Executing control action: %s", action.c_str());
    
    if (action == "power") {
        controller.activatePowerRelay();
        ESP_LOGI(TAG, "Power relay activated");
    } else if (action == "reset") {
        controller.activateResetRelay();
        ESP_LOGI(TAG, "Reset relay activated");
    } else if (action == "beep") {
        controller.getBuzzer().beepPattern(2, 200, 200);
        ESP_LOGI(TAG, "Buzzer test executed");
    } else {
        ESP_LOGW(TAG, "Unknown control action: %s", action.c_str());
    }
}