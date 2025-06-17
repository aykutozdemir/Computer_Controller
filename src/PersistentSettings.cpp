#include "PersistentSettings.h"

// Initialize static members
PersistentSettings* PersistentSettings::instance = nullptr;

PersistentSettings& PersistentSettings::getInstance() {
    if (instance == nullptr) {
        instance = new PersistentSettings();
    }
    return *instance;
}

void PersistentSettings::begin() {
    if (!preferences.begin(NVS_NAMESPACE, false)) {
        Serial.println("Failed to open preferences");
        return;
    }

    // Initialize child lock setting if not exists
    if (!preferences.isKey(NVS_KEY_CHILD_LOCK)) {
        preferences.putBool(NVS_KEY_CHILD_LOCK, false);
    }

    // Initialize buzzer setting if not exists
    if (!preferences.isKey(NVS_KEY_BUZZER_ENABLED)) {
        preferences.putBool(NVS_KEY_BUZZER_ENABLED, true);
    }

    // Initialize RF enabled setting if not exists
    if (!preferences.isKey(NVS_KEY_RF_ENABLED)) {
        preferences.putBool(NVS_KEY_RF_ENABLED, true); // Default to enabled
    }

    // Initialize RF button code setting if not exists
    if (!preferences.isKey(NVS_KEY_RF_BUTTON_CODE)) {
        preferences.putUInt(NVS_KEY_RF_BUTTON_CODE, 0); // Default to 0 (no code stored)
    }
}

bool PersistentSettings::isChildLockEnabled() {
    return preferences.getBool(NVS_KEY_CHILD_LOCK, false); // Default to false if not set
}

void PersistentSettings::setChildLockEnabled(bool enabled) {
    if (!preferences.putBool(NVS_KEY_CHILD_LOCK, enabled)) {
        Serial.println("Failed to save child lock setting");
    }
}

bool PersistentSettings::isBuzzerEnabled() {
    return preferences.getBool(NVS_KEY_BUZZER_ENABLED, true); // Default to true if not set
}

void PersistentSettings::setBuzzerEnabled(bool enabled) {
    if (!preferences.putBool(NVS_KEY_BUZZER_ENABLED, enabled)) {
        Serial.println("Failed to save buzzer setting");
    }
}

bool PersistentSettings::isRFEnabled() {
    return preferences.getBool(NVS_KEY_RF_ENABLED, true); // Default to true if not set
}

void PersistentSettings::setRFEnabled(bool enabled) {
    if (!preferences.putBool(NVS_KEY_RF_ENABLED, enabled)) {
        Serial.println("Failed to save RF enabled setting");
    }
}

uint32_t PersistentSettings::getRfButtonCode() {
    return preferences.getUInt(NVS_KEY_RF_BUTTON_CODE, 0); // Default to 0 if not set
}

void PersistentSettings::setRfButtonCode(uint32_t code) {
    if (!preferences.putUInt(NVS_KEY_RF_BUTTON_CODE, code)) {
        Serial.println("Failed to save RF button code");
    }
}

void PersistentSettings::clearAll() {
    ESP_LOGI("PersistentSettings", "Clearing all settings");
    
    // Clear all settings by setting them to their default values
    setChildLockEnabled(false);
    setBuzzerEnabled(true);
    setRFEnabled(false);
    setRfButtonCode(0);
    
    // Commit changes to NVS
    preferences.end();
    preferences.begin("settings", false);
    
    ESP_LOGI("PersistentSettings", "All settings cleared successfully");
} 