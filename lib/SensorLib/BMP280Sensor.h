#ifndef BMP280_SENSOR_H
#define BMP280_SENSOR_H

#include <Arduino.h>
#include <Wire.h> // For I2C communication
#include <Adafruit_BMP280.h>

// If you plan to use an existing library like Adafruit_BMP280, include it here:
// #include <Adafruit_BMP280.h> 

#define BMP280_SENSOR_READ_INTERVAL_MS 1000


class BMP280Sensor {
public:
    /**
     * @brief Construct a new BMP280Sensor object.
     *
     * @param wirePort The TwoWire instance to use for I2C communication with this sensor. Defaults to global Wire.
     * All member variables are initialised to safe defaults. Call begin() once, then call loop() regularly.
     */
    BMP280Sensor(TwoWire &wirePort = Wire);

    /**
     * @brief Initialises the BMP-280 sensor.
     *
     * This powers-up and configures the underlying Adafruit_BMP280 driver.  Most boards expose the
     * sensor at either 0x76 or 0x77; you may supply the address explicitly if it differs.
     *
     * @param i2c_addr  I²C address of the device (default 0x76).
     * @return true  Sensor initialised successfully and is ready for use.
     * @return false Sensor was not found or failed self-test.
     */
    bool begin(uint8_t i2c_addr = 0x77);

    /**
     * @brief Performs a blocking read of temperature, pressure and altitude.
     *
     * This method immediately talks to the hardware and updates the cached internal readings.
     * In most sketches you should not call this directly – instead call loop() regularly and
     * use the *getXXX()* accessors – but the method is provided for completeness.
     *
     * @param temperature  Reference that will receive the temperature in °C.
     * @param pressure     Reference that will receive the pressure in Pa.
     * @param altitude     Reference that will receive the altitude in metres (based on 1013.25 hPa sea-level).
     * @return true  Read successful, values have been updated.
     * @return false Read failed (e.g. sensor not initialised or I²C error).
     */
    bool readSensor(float &temperature, float &pressure, float &altitude);

    /**
     * @brief Returns the most recently measured temperature.
     * @return Temperature in °C, or NAN if no valid measurement is available.
     */
    float getTemperature() const;

    /**
     * @brief Returns the most recently measured pressure.
     * @return Pressure in Pascals, or NAN if no valid measurement is available.
     */
    float getPressure() const;

    /**
     * @brief Calculates/returns altitude based on the most recent pressure reading.
     *
     * The altitude calculation uses the barometric formula and therefore depends on the
     * assumed sea-level pressure.  If you supply *seaLevelhPa* it will be used; otherwise the
     * default 1013.25 hPa (ISA standard atmosphere) is applied.
     *
     * @param seaLevelhPa  Reference sea-level pressure in hecto-Pascals.
     * @return Altitude in metres, or NAN if no valid pressure reading is available.
     */
    float getAltitude(float seaLevelhPa = 1013.25F) const;

    /**
     * @brief Should be called frequently (e.g. from the main loop). Performs a sensor read at most once per second.
     */
    void loop();

private:
    // Example: Adafruit_BMP280 bmp; 
    bool _sensorExists;
    float _currentTemperature;
    float _currentPressure;
    float _currentAltitude;

    unsigned long _lastReadMs{0};          // Timestamp of the last successful sensor read
    const uint32_t _readIntervalMs{BMP280_SENSOR_READ_INTERVAL_MS};  // Minimum interval between two reads

    TwoWire* _i2cPort;                     // Stores the I2C bus instance used by this sensor
    Adafruit_BMP280 _bmp;                  // Driver instance, initialized with _i2cPort
};

#endif // BMP280_SENSOR_H