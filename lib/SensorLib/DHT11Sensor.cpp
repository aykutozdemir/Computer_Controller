#include "DHT11Sensor.h"

DHT11Sensor::DHT11Sensor(uint8_t pin)
    : _dht(pin, DHT11),
      _timer(DHT11_SENSOR_READ_INTERVAL_MS),
      _temperature(NAN),
      _humidity(NAN),
      _sensorOk(false),
      _retryCount(0) {}

bool DHT11Sensor::begin() {
    _dht.begin();
    // Give the sensor time to wake up after power-up (per datasheet ≈1 s)
    delay(1500);

    // Defer the first real measurement to loop(); assume OK for now.
    _sensorOk = false;
    _retryCount = 0;
    _timer.reset();
    return true;
}

void DHT11Sensor::loop() {
    if (!_timer.isReady()) {
        return; // Too soon for next read
    }
    
    _timer.reset();

    // Read DHT11 sensor with timeout protection
    unsigned long startTime = millis();
    const unsigned long timeout = 1000; // 1 second timeout
    
    float t = NAN;
    float h = NAN;
    
    // Try to read the sensor with timeout
    while (isnan(t) || isnan(h)) {
        t = _dht.readTemperature();
        h = _dht.readHumidity();
        
        // Check for timeout
        if (millis() - startTime > timeout) {
            break;
        }
        
        // Small delay to prevent tight loop
        delay(10);
    }

    // Check if readings are valid
    if (!isnan(t) && !isnan(h)) {
        _temperature = t;
        _humidity = h;
        _sensorOk = true;
        _retryCount = 0;  // Reset retry count on successful read
        _timer.setInterval(DHT11_SENSOR_READ_INTERVAL_MS);  // Reset to normal interval
    } else {
        // Increment retry count on failed read
        _retryCount++;
        _sensorOk = false;
        
        // If we've had too many consecutive failures, increase the interval
        if (_retryCount >= 3) {
            _timer.setInterval(1000);  // 1 second recovery interval
        }
    }
} 