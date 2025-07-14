#include "MQTTManager.h"
#include "Globals.h"
#include "RootCA.h"
#include "ComputerController.h"

// External declaration for global ComputerController pointer
extern ComputerController* g_computerController;

// Define log tag for this file
static const char* TAG = "MQTTManager";

// Global instance for callback access
static MQTTManager* g_mqttManager = nullptr;

MQTTManager::MQTTManager() : mqttClient(wifiClient), isConnected(false) {
    g_mqttManager = this;
}

void MQTTManager::begin() {
    ESP_LOGI(TAG, "Initializing MQTT manager after display and WiFi initialization...");
    
    // Add delay to ensure WiFi is stable before MQTT setup
    delay(100);
    
    // Configure WiFi client for TLS – load Computer Controller Root CA certificate
    wifiClient.setCACert(ISRG_ROOT_X1_CA_PEM);
    
    // Configure MQTT client
    mqttClient.setServer(MQTT_BROKER, MQTT_PORT);
    mqttClient.setCallback(messageCallback);
    mqttClient.setKeepAlive(MQTT_KEEPALIVE_SECONDS);
    mqttClient.setSocketTimeout(30);
    
    // Initialize timer
    lastStatusTime = 0;
    
    ESP_LOGI(TAG, "MQTT manager initialized successfully");
}

void MQTTManager::loop() {
    if (!WiFi.isConnected()) {
        if (isConnected) {
            ESP_LOGW(TAG, "WiFi disconnected, disconnecting from broker");
            disconnect();
        }
        return;
    }
    
    if (!isConnected) {
        connectToBroker();
    } else {
        mqttClient.loop();
        
        // Check if it's time to publish status
        if (millis() - lastStatusTime >= MQTT_STATUS_INTERVAL) {
            publishStatusUpdate();
            lastStatusTime = millis();
        }
    }
}

bool MQTTManager::connectToBroker() {
    if (isConnected) {
        return true;
    }
    
    ESP_LOGI(TAG, "Connecting to broker %s:%d", MQTT_BROKER, MQTT_PORT);
    
    // Generate unique client ID with timestamp
    String clientId = String(MQTT_CLIENT_ID) + "_" + String(millis());
    
    if (mqttClient.connect(clientId.c_str(), MQTT_USERNAME_CRED, MQTT_PASSWORD_CRED)) {
        ESP_LOGI(TAG, "Connected to broker successfully");
        isConnected = true;
        
        // Subscribe to control topic
        if (mqttClient.subscribe(MQTT_TOPIC_CONTROL)) {
            ESP_LOGI(TAG, "Subscribed to control topic");
        } else {
            ESP_LOGE(TAG, "Failed to subscribe to control topic");
        }
        
        // Publish initial status
        publishStatusUpdate();
        
        return true;
    } else {
        ESP_LOGE(TAG, "Connection failed, rc=%d", mqttClient.state());
        return false;
    }
}

void MQTTManager::disconnect() {
    if (isConnected) {
        mqttClient.disconnect();
        isConnected = false;
        ESP_LOGI(TAG, "Disconnected from broker");
    }
}

void MQTTManager::publishStatusUpdate() {
    if (!isConnected) {
        return;
    }
    
    // Create status JSON
    DynamicJsonDocument doc(1024);
    doc["timestamp"] = millis();
    doc["uptime"] = millis() / 1000;
    doc["wifi_rssi"] = WiFi.RSSI();
    doc["free_heap"] = ESP.getFreeHeap();
    doc["cpu_freq"] = ESP.getCpuFreqMHz();
    
    // Add system status if available
    if (g_computerController) {
        doc["pc_powered"] = g_computerController->isPCPoweredOn();
        doc["child_lock"] = g_computerController->isChildLockEnabled();
        doc["buzzer_enabled"] = g_computerController->isBuzzerEnabled();
    }
    
    String statusJson;
    serializeJson(doc, statusJson);
    
    // Only publish if status changed
    if (statusJson != lastStatusMessage) {
        if (mqttClient.publish(MQTT_TOPIC_STATUS, statusJson.c_str())) {
            ESP_LOGD(TAG, "Status published");
            lastStatusMessage = statusJson;
        } else {
            ESP_LOGE(TAG, "Failed to publish status");
        }
    }
}

void MQTTManager::publishEvent(const String& event, const String& data) {
    if (!isConnected) {
        return;
    }
    
    DynamicJsonDocument doc(512);
    doc["event"] = event;
    doc["timestamp"] = millis();
    if (data.length() > 0) {
        doc["data"] = data;
    }
    
    String eventJson;
    serializeJson(doc, eventJson);
    
    if (mqttClient.publish(MQTT_TOPIC_EVENTS, eventJson.c_str())) {
        ESP_LOGI(TAG, "Event published: %s", event.c_str());
    } else {
        ESP_LOGE(TAG, "Failed to publish event");
    }
}

void MQTTManager::publishSettings(const String& settings) {
    if (!isConnected) {
        return;
    }
    
    if (mqttClient.publish(MQTT_TOPIC_SETTINGS, settings.c_str())) {
        ESP_LOGI(TAG, "Settings published");
    } else {
        ESP_LOGE(TAG, "Failed to publish settings");
    }
}

String MQTTManager::getConnectionStatus() const {
    if (!WiFi.isConnected()) {
        return "WiFi disconnected";
    }
    
    if (!isConnected) {
        return "MQTT disconnected";
    }
    
    return "Connected";
}

void MQTTManager::handleControlMessage(const String& command, const String& data) {
    ESP_LOGI(TAG, "Received control command: %s%s", command.c_str(), 
             data.length() > 0 ? (" with data: " + data).c_str() : "");
    
    // Handle different commands
    if (command == "power_on") {
        if (g_computerController) {
            g_computerController->powerOnPC();
            publishEvent("pc_powered_on", "via_mqtt");
        }
    } else if (command == "power_off") {
        if (g_computerController) {
            g_computerController->powerOffPC();
            publishEvent("pc_powered_off", "via_mqtt");
        }
    } else if (command == "reset") {
        if (g_computerController) {
            g_computerController->resetPC();
            publishEvent("pc_reset", "via_mqtt");
        }
    } else if (command == "toggle_child_lock") {
        if (g_computerController) {
            g_computerController->toggleChildLock();
            publishEvent("child_lock_toggled", g_computerController->isChildLockEnabled() ? "enabled" : "disabled");
        }
    } else if (command == "toggle_buzzer") {
        if (g_computerController) {
            g_computerController->toggleBuzzer();
            publishEvent("buzzer_toggled", g_computerController->isBuzzerEnabled() ? "enabled" : "disabled");
        }
    } else {
        ESP_LOGW(TAG, "Unknown command: %s", command.c_str());
    }
}

// Static callback function
void MQTTManager::messageCallback(char* topic, byte* payload, unsigned int length) {
    if (!g_mqttManager) {
        return;
    }
    
    String topicStr = String(topic);
    String payloadStr = String((char*)payload, length);
    
    ESP_LOGI(TAG, "Message received on topic: %s with payload: %s", topicStr.c_str(), payloadStr.c_str());
    
    if (topicStr == MQTT_TOPIC_CONTROL) {
        // Parse JSON control message
        DynamicJsonDocument doc(512);
        DeserializationError error = deserializeJson(doc, payloadStr);
        
        if (error) {
            ESP_LOGE(TAG, "JSON parsing failed: %s", error.c_str());
            return;
        }
        
        String command = doc["command"] | "";
        String data = doc["data"] | "";
        
        if (command.length() > 0) {
            g_mqttManager->handleControlMessage(command, data);
        }
    }
} 