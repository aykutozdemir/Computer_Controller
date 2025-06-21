# Computer Controller Web Server

This document describes the web server functionality added to the Computer Controller project.

## Overview

The web server provides a modern, responsive web interface for monitoring system status and controlling settings. It runs on port 80 and includes real-time status updates using Server-Sent Events (SSE).

## Features

### Real-time Status Monitoring
- **WiFi Status**: Shows connection status
- **PC Power Status**: Shows if the connected computer is powered on
- **Environmental Sensors**: Temperature and humidity readings
- **GPU Fan Control**: Current speed and RPM
- **System Information**: Free heap memory and uptime
- **Feature Status**: Buzzer and RF functionality status

### Settings Management
- **Buzzer Control**: Enable/disable buzzer sounds
- **RF Control**: Enable/disable RF receiver functionality
- **Child Lock**: Enable/disable child lock feature
- **GPU Fan Speed**: Set fan speed percentage (0-100%)

### Power Control
- **Power On/Off**: Activate power relay
- **Reset PC**: Activate reset relay
- **Test Buzzer**: Play a test beep pattern

## Accessing the Web Interface

1. **Connect to WiFi**: Ensure the ESP32 is connected to your WiFi network
2. **Find IP Address**: Check the serial monitor for the ESP32's IP address
3. **Open Browser**: Navigate to `http://[ESP32_IP_ADDRESS]`
4. **Use Interface**: The web interface will load with real-time status updates

## API Endpoints

### GET `/`
- **Description**: Main web interface
- **Response**: HTML page with status and settings tabs

### GET `/status`
- **Description**: Get current system stat1us as JSON
- **Response**: JSON object with all status information

### GET `/settings`
- **Description**: Get current settings as JSON
- **Response**: JSON object with current settings

### POST `/settings`
- **Description**: Update system settings
- **Body**: JSON object with settings to update
- **Response**: Success/error status

### POST `/control`
- **Description**: Execute control actions
- **Body**: JSON object with action parameter
- **Actions**: "power", "reset", "beep"

### GET `/sse`
- **Description**: Server-Sent Events endpoint for real-time updates
- **Response**: Continuous stream of status updates every second

## Example API Usage

### Get Status
```bash
curl http://[ESP32_IP]/status
```

### Update Settings
```bash
curl -X POST http://[ESP32_IP]/settings \
  -H "Content-Type: application/json" \
  -d '{"buzzerEnabled": true, "gpuFanSpeed": 50}'
```

### Control Actions
```bash
curl -X POST http://[ESP32_IP]/control \
  -H "Content-Type: application/json" \
  -d '{"action": "power"}'
```

## Technical Details

### Status Update Frequency
- Status updates are sent every 1000ms (1 second) via SSE
- The web interface automatically reconnects if the connection is lost

### JSON Response Format

#### Status Response
```json
{
  "wifiConnected": true,
  "pcPoweredOn": true,
  "temperature": 23.5,
  "humidity": 45.2,
  "gpuFanSpeed": 50,
  "gpuFanRpm": 1200,
  "gpuFanEnabled": true,
  "buzzerEnabled": true,
  "rfEnabled": true,
  "childLock": false,
  "uptime": 3600000,
  "freeHeap": 204800,
  "timestamp": 1640995200000
}
```

#### Settings Response
```json
{
  "buzzerEnabled": true,
  "rfEnabled": true,
  "childLock": false,
  "gpuFanSpeed": 50
}
```

## Browser Compatibility

The web interface is designed to work with modern browsers that support:
- ES6 JavaScript features
- Fetch API
- Server-Sent Events
- CSS Grid and Flexbox

## Security Considerations

- The web server runs on HTTP (not HTTPS) for simplicity
- No authentication is implemented - anyone on the network can access the interface
- Consider implementing authentication for production use
- The interface is intended for local network use only

## Troubleshooting

### Web Interface Not Loading
1. Check that the ESP32 is connected to WiFi
2. Verify the IP address is correct
3. Ensure no firewall is blocking port 80
4. Check serial monitor for any error messages

### Status Not Updating
1. Check browser console for JavaScript errors
2. Verify SSE connection is established
3. Check network connectivity
4. Restart the ESP32 if necessary

### Settings Not Saving
1. Check that the JSON format is correct
2. Verify all required fields are present
3. Check serial monitor for error messages
4. Ensure the ESP32 has enough free memory

## Development

The web server is implemented in the `WebServerManager` class:
- **Header**: `include/WebServerManager.h`
- **Implementation**: `src/WebServerManager.cpp`

To modify the web interface:
1. Edit the HTML/CSS/JavaScript in the `handleRoot()` method
2. Add new API endpoints as needed
3. Update the status JSON format if adding new fields
4. Test thoroughly on different browsers 