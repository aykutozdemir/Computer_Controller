#include "RFStudyManager.h"

// Define log tag
static const char* TAG = "RFStudyManager";

RFStudyManager::RFStudyManager(RCSwitchReceiver& rfReceiver, PersistentSettings& settings)
    : rfReceiver(rfReceiver), settings(settings) {
}

bool RFStudyManager::startListening(uint32_t timeoutMs, RFCodeCallback callback) {
    if (listening) {
        ESP_LOGW(TAG, "Already listening for RF codes");
        return false;
    }

    listening = true;
    startTime = millis();
    timeout = timeoutMs;
    this->callback = callback;
    ESP_LOGI(TAG, "Started listening for RF codes%s", timeoutMs ? " with timeout" : "");
    return true;
}

void RFStudyManager::stopListening() {
    if (!listening) {
        return;
    }

    listening = false;
    callback = nullptr;
    ESP_LOGI(TAG, "Stopped listening for RF codes");
}

uint32_t RFStudyManager::getStoredCode() const {
    return settings.getRfButtonCode();
}

void RFStudyManager::clearStoredCode() {
    settings.setRfButtonCode(0);
    ESP_LOGI(TAG, "Cleared stored RF code");
}

void RFStudyManager::process() {
    if (!listening) {
        return;
    }

    // Check timeout if set
    if (timeout > 0 && (millis() - startTime) >= timeout) {
        ESP_LOGI(TAG, "RF study timeout reached");
        stopListening();
        return;
    }

    // Check for new RF code
    if (rfReceiver.read()) {
        uint32_t code = rfReceiver.getButtonCode();
        if (code != 0) {
            ESP_LOGI(TAG, "Detected RF code: 0x%lX (%lu)", code, code);
            
            // Store the code
            settings.setRfButtonCode(code);
            
            // Call callback if set
            if (callback) {
                callback(code);
            }
            
            // Stop listening after successful detection
            stopListening();
        }
    }
} 