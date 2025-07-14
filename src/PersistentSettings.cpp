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

    // Initialize defaults if missing
    if (!preferences.isKey(NVS_KEY_CHILD_LOCK)) {
        preferences.putBool(NVS_KEY_CHILD_LOCK, false);
    }
    if (!preferences.isKey(NVS_KEY_BUZZER_ENABLED)) {
        preferences.putBool(NVS_KEY_BUZZER_ENABLED, true);
    }
    if (!preferences.isKey(NVS_KEY_RF_ENABLED)) {
        preferences.putBool(NVS_KEY_RF_ENABLED, true);
    }
    if (!preferences.isKey(NVS_KEY_RF_BUTTON_CODE)) {
        preferences.putUInt(NVS_KEY_RF_BUTTON_CODE, 0);
    }

    // Load cached values
    cachedChildLock = preferences.getBool(NVS_KEY_CHILD_LOCK, false);
    cachedBuzzer    = preferences.getBool(NVS_KEY_BUZZER_ENABLED, true);
    cachedRFEnabled = preferences.getBool(NVS_KEY_RF_ENABLED, true);
    cachedRFButtonCode = preferences.getUInt(NVS_KEY_RF_BUTTON_CODE, 0);
}

bool PersistentSettings::isChildLockEnabled() {
    return cachedChildLock;
}

void PersistentSettings::setChildLockEnabled(bool enabled) {
    cachedChildLock = enabled;
    if (!preferences.putBool(NVS_KEY_CHILD_LOCK, enabled)) {
        Serial.println("Failed to save child lock setting");
    }
}

void PersistentSettings::toggleChildLock() {
    bool currentState = isChildLockEnabled();
    setChildLockEnabled(!currentState);
}

bool PersistentSettings::isBuzzerEnabled() {
    return cachedBuzzer;
}

void PersistentSettings::setBuzzerEnabled(bool enabled) {
    cachedBuzzer = enabled;
    if (!preferences.putBool(NVS_KEY_BUZZER_ENABLED, enabled)) {
        Serial.println("Failed to save buzzer setting");
    }
}

void PersistentSettings::toggleBuzzer() {
    bool currentState = isBuzzerEnabled();
    setBuzzerEnabled(!currentState);
}

bool PersistentSettings::isRFEnabled() {
    return cachedRFEnabled;
}

void PersistentSettings::setRFEnabled(bool enabled) {
    cachedRFEnabled = enabled;
    if (!preferences.putBool(NVS_KEY_RF_ENABLED, enabled)) {
        Serial.println("Failed to save RF enabled setting");
    }
}

uint32_t PersistentSettings::getRfButtonCode() {
    return cachedRFButtonCode;
}

void PersistentSettings::setRfButtonCode(uint32_t code) {
    cachedRFButtonCode = code;
    if (!preferences.putUInt(NVS_KEY_RF_BUTTON_CODE, code)) {
        Serial.println("Failed to save RF button code");
    }
}

void PersistentSettings::clearAll() {
    ESP_LOGI("PersistentSettings", "Clearing all settings");
    setChildLockEnabled(false);
    setBuzzerEnabled(true);
    setRFEnabled(false);
    setRfButtonCode(0);
} 