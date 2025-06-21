#ifndef MQTT_MANAGER_H
#define MQTT_MANAGER_H

#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

class MQTTManager {
private:
    WiFiClientSecure wifiClient;
    PubSubClient mqttClient;
    unsigned long lastStatusTime;
    bool isConnected;
    String lastStatusMessage;
    
    // Callback function for incoming MQTT messages
    static void messageCallback(char* topic, byte* payload, unsigned int length);
    
    // Helper methods
    void connect();
    void publishStatus();
    void handleIncomingMessage(const String& topic, const String& payload);
    
public:
    MQTTManager();
    
    // Initialization and connection
    void begin();
    void loop();
    bool connectToBroker();
    void disconnect();
    
    // Publishing methods
    void publishStatusUpdate();
    void publishEvent(const String& event, const String& data = "");
    void publishSettings(const String& settings);
    
    // Status methods
    bool isConnectedToBroker() const { return isConnected; }
    String getConnectionStatus() const;
    
    // Control methods
    void handleControlMessage(const String& command, const String& data);
};

#endif // MQTT_MANAGER_H 