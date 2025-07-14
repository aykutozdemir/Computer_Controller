/**
 * @file StringChangeDetector.h
 * @brief Utility class for detecting when string values change
 */

#pragma once

#include <Arduino.h>
#include <functional>

/**
 * @class StringChangeDetector
 * @brief Detects when a string value changes and executes a callback
 */
class StringChangeDetector {
private:
    String _lastValue;
    std::function<void(const String&)> _onChange;
    bool _initialized;

public:
    /**
     * @brief Constructor
     * @param onChange Callback function to execute when string changes
     */
    StringChangeDetector(std::function<void(const String&)> onChange = nullptr) 
        : _onChange(onChange), _initialized(false) {}

    /**
     * @brief Check if string has changed and execute callback if it has
     * @param currentValue The current string value to check
     * @return true if the string changed, false otherwise
     */
    bool checkAndUpdate(const String& currentValue) {
        if (!_initialized || currentValue != _lastValue) {
            _lastValue = currentValue;
            _initialized = true;
            
            if (_onChange) {
                _onChange(currentValue);
            }
            
            return true;
        }
        return false;
    }

    /**
     * @brief Set the callback function
     * @param onChange Callback function to execute when string changes
     */
    void setOnChange(std::function<void(const String&)> onChange) {
        _onChange = onChange;
    }

    /**
     * @brief Get the last recorded value
     * @return The last string value
     */
    const String& getLastValue() const {
        return _lastValue;
    }

    /**
     * @brief Reset the detector (next check will trigger onChange)
     */
    void reset() {
        _initialized = false;
        _lastValue = "";
    }

    /**
     * @brief Check if the detector has been initialized
     * @return true if initialized, false otherwise
     */
    bool isInitialized() const {
        return _initialized;
    }
}; 