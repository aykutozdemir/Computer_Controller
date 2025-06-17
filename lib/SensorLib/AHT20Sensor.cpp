#include "AHT20Sensor.h"
#include <Adafruit_Sensor.h>

// Helper macro for error printing that doesn't flood serial if disabled
#ifndef AHT20_DEBUG
#define AHT20_DEBUG 1
#endif

#if AHT20_DEBUG
#define AHT20_DEBUG_PRINTLN(x) Serial.println(x)
#else
#define AHT20_DEBUG_PRINTLN(x)
#endif

AHT20Sensor::AHT20Sensor() :
    _sensorExists(false),
    _currentTemperature(NAN),
    _currentHumidity(NAN) {
    // Empty
}

bool AHT20Sensor::begin(TwoWire &wirePort) {
    // Attempt to initialise the sensor
    if (!_aht.begin(&wirePort)) {
        AHT20_DEBUG_PRINTLN("[AHT20] Failed to initialise sensor");
        _sensorExists = false;
        return false;
    }

    AHT20_DEBUG_PRINTLN("[AHT20] Sensor initialised successfully");
    _sensorExists = true;
    _lastReadMs = 0;
    return true;
}

bool AHT20Sensor::readSensor(float &temperature, float &humidity) {
    if (!_sensorExists) {
        return false;
    }

    sensors_event_t humEvent, tempEvent;
    if (!_aht.getEvent(&humEvent, &tempEvent)) {
        AHT20_DEBUG_PRINTLN("[AHT20] Failed to read sensor event");
        return false;
    }

    _currentTemperature = tempEvent.temperature;
    _currentHumidity = humEvent.relative_humidity;

    temperature = _currentTemperature;
    humidity = _currentHumidity;
    return true;
}

float AHT20Sensor::getTemperature() const {
    // Optionally, call readSensor() here if you want this to be a blocking read,
    // or ensure readSensor() is called periodically elsewhere.
    return _currentTemperature;
}

float AHT20Sensor::getHumidity() const {
    // Optionally, call readSensor() here.
    return _currentHumidity;
}

void AHT20Sensor::loop() {
    if (!_sensorExists) return;

    unsigned long now = millis();
    if (now - _lastReadMs < _readIntervalMs) {
        return; // not yet time for next read
    }

    _lastReadMs = now;

    float t, h;
    if (!readSensor(t, h)) {
        AHT20_DEBUG_PRINTLN("[AHT20] Reading failed");
    }
}