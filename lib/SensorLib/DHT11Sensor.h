#pragma once

#include <Arduino.h>
#include <DHT.h>
#include <SimpleTimer.h>

// Default sample interval in milliseconds
#ifndef DHT11_SENSOR_READ_INTERVAL_MS
#define DHT11_SENSOR_READ_INTERVAL_MS 2000
#endif

/**
 * @brief Wrapper class around Adafruit DHT library for DHT11 sensor.
 *
 *  The class periodically reads the temperature and humidity from the sensor
 *  with a configurable minimum interval between consecutive reads, storing the
 *  latest values which can be queried via getters. If a reading fails, the
 *  previous valid reading is preserved and NAN is returned from the getter.
 */
class DHT11Sensor {
public:
    /**
     * @brief Construct a new DHT11Sensor object
     * @param pin  GPIO pin where the sensor data line is connected.
     */
    explicit DHT11Sensor(uint8_t pin);

    /**
     * @brief Initialize the sensor hardware.
     *
     *  Must be called in the Arduino setup phase. The function returns true if
     *  communication with the sensor was successful at least once.
     */
    bool begin();

    /**
     * @brief Should be called frequently from loop(). Will attempt to read the
     *  sensor not more often than the defined interval.
     */
    void loop();

    /** Temperature getter (°C). */
    float getTemperature() const { return _temperature; }
    /** Humidity getter (%RH). */
    float getHumidity() const { return _humidity; }
    
    /**
     * @brief Check if the sensor is available and working.
     * @return true if the sensor has successfully read at least once, false otherwise.
     */
    bool isAvailable() const { return _sensorOk; }

private:
    DHT _dht;                      // Driver instance
    SimpleTimer<> _timer;          // Timing timer 

    float _temperature;            // Cached temperature in °C
    float _humidity;               // Cached humidity %
    bool  _sensorOk;               // Flag set once a successful read happened
    uint32_t _retryCount;          // Retry counter
}; 