#include "MQTTManager.h"
#include "Globals.h"
#include "ComputerController.h"

// External declaration for global ComputerController pointer
extern ComputerController* g_computerController;

// Global instance for callback access
static MQTTManager* g_mqttManager = nullptr;

MQTTManager::MQTTManager() : mqttClient(wifiClient), isConnected(false) {
    g_mqttManager = this;
}

void MQTTManager::begin() {
    Serial.println("MQTT: Initializing MQTT manager...");
    
    // Configure WiFi client for TLS
    wifiClient.setInsecure(); // Skip certificate verification for now
    
    // Configure MQTT client
    mqttClient.setServer(MQTT_BROKER, MQTT_PORT);
    mqttClient.setCallback(messageCallback);
    mqttClient.setKeepAlive(MQTT_KEEPALIVE_SECONDS);
    mqttClient.setSocketTimeout(30);
    
    // Initialize timer
    lastStatusTime = 0;
    
    Serial.println("MQTT: Manager initialized");
}

void MQTTManager::loop() {
    if (!WiFi.isConnected()) {
        if (isConnected) {
            Serial.println("MQTT: WiFi disconnected, disconnecting from broker");
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
    
    Serial.print("MQTT: Connecting to broker ");
    Serial.print(MQTT_BROKER);
    Serial.print(":");
    Serial.println(MQTT_PORT);
    
    // Generate unique client ID with timestamp
    String clientId = String(MQTT_CLIENT_ID) + "_" + String(millis());
    
    if (mqttClient.connect(clientId.c_str(), MQTT_USERNAME_CRED, MQTT_PASSWORD_CRED)) {
        Serial.println("MQTT: Connected to broker successfully");
        isConnected = true;
        
        // Subscribe to control topic
        if (mqttClient.subscribe(MQTT_TOPIC_CONTROL)) {
            Serial.println("MQTT: Subscribed to control topic");
        } else {
            Serial.println("MQTT: Failed to subscribe to control topic");
        }
        
        // Publish initial status
        publishStatusUpdate();
        
        return true;
    } else {
        Serial.print("MQTT: Connection failed, rc=");
        Serial.println(mqttClient.state());
        return false;
    }
}

void MQTTManager::disconnect() {
    if (isConnected) {
        mqttClient.disconnect();
        isConnected = false;
        Serial.println("MQTT: Disconnected from broker");
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
            Serial.println("MQTT: Status published");
            lastStatusMessage = statusJson;
        } else {
            Serial.println("MQTT: Failed to publish status");
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
        Serial.print("MQTT: Event published: ");
        Serial.println(event);
    } else {
        Serial.println("MQTT: Failed to publish event");
    }
}

void MQTTManager::publishSettings(const String& settings) {
    if (!isConnected) {
        return;
    }
    
    if (mqttClient.publish(MQTT_TOPIC_SETTINGS, settings.c_str())) {
        Serial.println("MQTT: Settings published");
    } else {
        Serial.println("MQTT: Failed to publish settings");
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
    Serial.print("MQTT: Received control command: ");
    Serial.print(command);
    if (data.length() > 0) {
        Serial.print(" with data: ");
        Serial.print(data);
    }
    Serial.println();
    
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
        Serial.print("MQTT: Unknown command: ");
        Serial.println(command);
    }
}

// Static callback function
void MQTTManager::messageCallback(char* topic, byte* payload, unsigned int length) {
    if (!g_mqttManager) {
        return;
    }
    
    String topicStr = String(topic);
    String payloadStr = String((char*)payload, length);
    
    Serial.print("MQTT: Message received on topic: ");
    Serial.print(topicStr);
    Serial.print(" with payload: ");
    Serial.println(payloadStr);
    
    if (topicStr == MQTT_TOPIC_CONTROL) {
        // Parse JSON control message
        DynamicJsonDocument doc(512);
        DeserializationError error = deserializeJson(doc, payloadStr);
        
        if (error) {
            Serial.print("MQTT: JSON parsing failed: ");
            Serial.println(error.c_str());
            return;
        }
        
        String command = doc["command"] | "";
        String data = doc["data"] | "";
        
        if (command.length() > 0) {
            g_mqttManager->handleControlMessage(command, data);
        }
    }
} 