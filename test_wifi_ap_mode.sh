#!/bin/bash

# Test script for ESP32 WiFi AP Mode improvements
# This script tests the enhanced AP mode setup and configuration

echo "ESP32 WiFi AP Mode Test Script"
echo "=============================="
echo ""

# Check if pio is available
if command -v pio &> /dev/null; then
    echo "✓ PlatformIO found"
else
    echo "✗ PlatformIO not found. Please install PlatformIO first."
    exit 1
fi

echo ""
echo "Building and uploading the improved WiFi AP mode code..."
echo ""

# Build and upload
pio run --target upload

if [ $? -eq 0 ]; then
    echo ""
    echo "✓ Upload successful!"
    echo ""
    echo "Testing WiFi AP Mode improvements..."
    echo ""
    echo "Expected improvements:"
    echo "  - Better AP mode setup with custom configuration"
    echo "  - Improved WiFi mode transition handling"
    echo "  - More reliable AP mode when WiFi connection fails"
    echo "  - Better timeout handling and error recovery"
    echo "  - Reduced authentication issues"
    echo ""
    echo "To test AP mode:"
    echo "1. Wait for the device to fail WiFi connection"
    echo "2. Look for 'AP mode setup successful' message"
    echo "3. Connect to 'ComputerController' WiFi network"
    echo "4. Navigate to 192.168.4.1 to configure WiFi"
    echo ""
    
    # Monitor serial output with focus on AP mode messages
    echo "Monitoring serial output (press Ctrl+C to stop)..."
    echo "Look for these AP mode improvements:"
    echo "  - 'Setting up WiFi AP mode...'"
    echo "  - 'WiFi AP started successfully'"
    echo "  - 'AP mode setup successful'"
    echo "  - 'WiFi mode transition' messages"
    echo ""
    
    # Monitor serial output
    pio device monitor --baud 115200 --filter "AP|WiFi|mode|setup|transition"
else
    echo ""
    echo "✗ Upload failed!"
    exit 1
fi 