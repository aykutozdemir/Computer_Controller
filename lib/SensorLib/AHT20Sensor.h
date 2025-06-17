#ifndef AHT20_SENSOR_H
#define AHT20_SENSOR_H

#include <Arduino.h>
#include <Wire.h> // For I2C communication
#include <Adafruit_AHTX0.h>

#define AHT20_SENSOR_READ_INTERVAL_MS 1000

/**
 * @brief Class for interfacing with the AHT20 temperature and humidity sensor.
 */
class AHT20Sensor {
public:
    /**
     * @brief Construct a new AHT20Sensor object.
     */
    AHT20Sensor();

    /**
     * @brief Initializes the AHT20 sensor.
     * @param wirePort The TwoWire interface to use for I2C communication (default is Wire).
     * @return True if initialization was successful, false otherwise.
     */
    bool begin(TwoWire &wirePort = Wire);

    /**
     * @brief Reads the current temperature and humidity from the sensor.
     * @param temperature Reference to a float to store the temperature in degrees Celsius.
     * @param humidity Reference to a float to store the relative humidity in percent.
     * @return True if the read was successful, false otherwise.
     */
    bool readSensor(float &temperature, float &humidity);

    /**
     * @brief Gets the last read temperature.
     * @return The temperature in degrees Celsius, or NAN if no successful read has occurred.
     */
    float getTemperature() const;

    /**
     * @brief Gets the last read humidity.
     * @return The relative humidity in percent, or NAN if no successful read has occurred.
     */
    float getHumidity() const;

    /**
     * @brief Should be called frequently (e.g. from the main loop). Will perform a sensor read at most once per second.
     */
    void loop();

private:
    bool _sensorExists;
    float _currentTemperature;
    float _currentHumidity;
    unsigned long _lastReadMs{0};         // Timestamp of the last successful sensor read
    const uint32_t _readIntervalMs{AHT20_SENSOR_READ_INTERVAL_MS}; // Minimum interval between two reads
    Adafruit_AHTX0 _aht;                  // Driver instance for the sensor
};

#endif // AHT20_SENSOR_H