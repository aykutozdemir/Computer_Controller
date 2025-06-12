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
    if (!preferences.begin(NAMESPACE, false)) {
        ESP_LOGE("PersistentSettings", "Failed to open preferences");
        return;
    }
    // Ensure child_lock key exists to avoid NOT_FOUND spam
    if (!preferences.isKey(KEY_CHILD_LOCK)) {
        preferences.putBool(KEY_CHILD_LOCK, false);
        ESP_LOGI("PersistentSettings", "Initialized child_lock to default (false)");
    }
    ESP_LOGI("PersistentSettings", "Settings initialized");
}

bool PersistentSettings::isChildLockEnabled() {
    return preferences.getBool(KEY_CHILD_LOCK, false); // Default to false if not set
}

void PersistentSettings::setChildLockEnabled(bool enabled) {
    if (!preferences.putBool(KEY_CHILD_LOCK, enabled)) {
        ESP_LOGE("PersistentSettings", "Failed to save child lock setting");
        return;
    }
    ESP_LOGI("PersistentSettings", "Child lock %s", enabled ? "enabled" : "disabled");
} 