#include "BMP280Sensor.h"
#include <Adafruit_Sensor.h>
#include <math.h>

#ifndef BMP280_DEBUG
#define BMP280_DEBUG 1
#endif

#if BMP280_DEBUG
#define BMP280_DEBUG_PRINTLN(x) Serial.println(x)
#else
#define BMP280_DEBUG_PRINTLN(x)
#endif

BMP280Sensor::BMP280Sensor(TwoWire &wirePort) :
    _i2cPort(&wirePort),
    _bmp(_i2cPort), // Initialize _bmp with the specific TwoWire instance
    _sensorExists(false),
    _currentTemperature(NAN),
    _currentPressure(NAN),
    _currentAltitude(NAN) {
    // Empty
}
 
bool BMP280Sensor::begin(uint8_t i2c_addr) {
    // _bmp was constructed with _i2cPort.
    // Adafruit_BMP280::begin() will call _i2cPort->begin() if needed by the library.
    if (!_bmp.begin(i2c_addr)) {
        BMP280_DEBUG_PRINTLN("[BMP280] Failed to initialise sensor");
        _sensorExists = false;
        return false;
    }

    // Default settings from Adafruit example
    _bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,     /* Operating Mode. */
                     Adafruit_BMP280::SAMPLING_X2,     /* Temp. oversampling */
                     Adafruit_BMP280::SAMPLING_X16,    /* Pressure oversampling */
                     Adafruit_BMP280::FILTER_X16,      /* Filtering. */
                     Adafruit_BMP280::STANDBY_MS_500); /* Standby time. */

    BMP280_DEBUG_PRINTLN("[BMP280] Sensor initialised successfully");
    _sensorExists = true;
    _lastReadMs = 0;
    return true;
}

bool BMP280Sensor::readSensor(float &temperature, float &pressure, float &altitude) {
    if (!_sensorExists) {
        return false;
    }

    _currentTemperature = _bmp.readTemperature();
    _currentPressure = _bmp.readPressure();
    _currentAltitude = _bmp.readAltitude(1013.25F);

    temperature = _currentTemperature;
    pressure = _currentPressure;
    altitude = _currentAltitude;

    return true;
}

float BMP280Sensor::getTemperature() const {
    return _currentTemperature;
}

float BMP280Sensor::getPressure() const {
    return _currentPressure;
}

float BMP280Sensor::getAltitude(float seaLevelhPa) const {
    if (!_sensorExists) return NAN;
    if (isnan(_currentPressure)) return NAN;
    // Formula: altitude = 44330 * (1.0 - pow(_currentPressure / seaLevelhPa, 0.1903));
    return 44330.0f * (1.0f - pow(_currentPressure / 100.0f / seaLevelhPa, 0.1903f)); // Assuming _currentPressure is in Pa
}

void BMP280Sensor::loop() {
    if (!_sensorExists) return;

    unsigned long now = millis();
    if (now - _lastReadMs < _readIntervalMs) {
        return;
    }

    _lastReadMs = now;

    float t, p, a;
    if (!readSensor(t, p, a)) {
        BMP280_DEBUG_PRINTLN("[BMP280] Reading failed");
    }
}