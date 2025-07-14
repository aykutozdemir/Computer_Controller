#!/bin/bash

# Test script for ESP32 WiFi watchdog fix
# This script helps monitor the ESP32 after applying the fix

echo "ESP32 WiFi Watchdog Fix Test Script"
echo "==================================="
echo ""

# Check if pio is available
if command -v pio &> /dev/null; then
    echo "✓ PlatformIO found"
else
    echo "✗ PlatformIO not found. Please install PlatformIO first."
    exit 1
fi

echo ""
echo "Building and uploading the fix..."
echo ""

# Build and upload
pio run --target upload

if [ $? -eq 0 ]; then
    echo ""
    echo "✓ Upload successful!"
    echo ""
    echo "Monitoring serial output (press Ctrl+C to stop)..."
    echo "Look for these improvements:"
    echo "  - More frequent watchdog feeding during WiFi connection"
    echo "  - Better timeout handling for WiFi connection attempts"
    echo "  - Reduced AUTH_EXPIRE errors"
    echo "  - No more watchdog timeout crashes"
    echo ""
    
    # Monitor serial output
    pio device monitor --baud 115200 --filter "WiFi|watchdog|AUTH_EXPIRE|task_wdt"
else
    echo ""
    echo "✗ Upload failed!"
    exit 1
fi 