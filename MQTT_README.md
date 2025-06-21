# MQTT Integration for Computer Controller

This document explains how to set up and use MQTT (Message Queuing Telemetry Transport) with your Computer Controller ESP32 project using HiveMQ Cloud.

## Overview

MQTT provides a lightweight, efficient way to communicate with your ESP32 from anywhere in the world. It's perfect for:
- Remote control of your computer
- Real-time status monitoring
- Event logging and notifications
- Integration with home automation systems

## Features

- **Real-time Status Updates**: Get live status of your ESP32 and connected PC
- **Remote Control**: Power on/off, reset PC, toggle settings from anywhere
- **Event Logging**: Track all actions and events
- **Secure Communication**: TLS encryption with HiveMQ Cloud
- **Global Access**: Access your device from anywhere with internet

## Setup Instructions

### 1. HiveMQ Cloud Account

1. Go to [HiveMQ Cloud](https://www.hivemq.com/cloud/)
2. Create a free account
3. Create a new cluster (free tier available)
4. Note down your cluster details:
   - Broker URL: Your cluster hostname
   - Port: `8883` (TLS) or `8884` (WebSocket)
   - Username and password (create in HiveMQ dashboard)

### 2. ESP32 Configuration

#### Update Credentials

Edit `include/Credentials.h` and update the MQTT credentials:

```cpp
// MQTT HiveMQ Cloud Configuration - KEEP THIS FILE OUT OF VERSION CONTROL
#define MQTT_BROKER_HOST "your-hivemq-broker-host"
#define MQTT_USERNAME "your-hivemq-username"
#define MQTT_PASSWORD "your-hivemq-password"
```

**Important**: The `Credentials.h` file should be kept out of version control to protect your sensitive data.

#### Build and Upload

1. Build the project:
   ```bash
   pio run
   ```

2. Upload to ESP32:
   ```bash
   pio run --target upload
   ```

3. Monitor serial output:
   ```bash
   pio device monitor
   ```

### 3. Python Client Setup

#### Install Dependencies

```bash
cd pc_app
pip install -r requirements.txt
```

#### Update Credentials

Edit `pc_app/mqtt_config.json` and update the credentials:

```json
{
    "broker": "your-hivemq-broker-host",
    "port": 8883,
    "username": "your-hivemq-username",
    "password": "your-hivemq-password"
}
```

## Usage

### ESP32 Features

The ESP32 will automatically:
- Connect to HiveMQ Cloud on startup
- Publish status updates every 5 seconds
- Listen for control commands
- Publish events for all actions

### Python Client

Run the Python client to control your ESP32:

```bash
cd pc_app
python mqtt_client.py
```

Available commands:
1. **Power On PC** - Activates power relay
2. **Power Off PC** - Activates power relay (same as power on)
3. **Reset PC** - Activates reset relay
4. **Toggle Child Lock** - Enable/disable child lock
5. **Toggle Buzzer** - Enable/disable buzzer sounds

### Phone Widget

Use the web-based phone widget:

1. Open `pc_app/phone_widget.html` in your phone's browser
2. Enter your MQTT credentials in the configuration form
3. Save and connect to start controlling your ESP32
4. Add to home screen for quick access

### MQTT Topics

The ESP32 uses these MQTT topics:

- `computer-controller/status` - Status updates (published by ESP32)
- `computer-controller/control` - Control commands (subscribed by ESP32)
- `computer-controller/events` - Event notifications (published by ESP32)
- `computer-controller/settings` - Settings updates (published by ESP32)

### Status Message Format

```json
{
  "timestamp": 1234567890,
  "uptime": 3600,
  "wifi_rssi": -45,
  "free_heap": 123456,
  "cpu_freq": 240,
  "pc_powered": true,
  "child_lock": false,
  "buzzer_enabled": true
}
```

### Control Message Format

```json
{
  "command": "power_on",
  "data": "",
  "timestamp": 1234567890
}
```

### Event Message Format

```json
{
  "event": "power_relay_activated",
  "data": "via_command",
  "timestamp": 1234567890
}
```

## Troubleshooting

### Connection Issues

1. **Check WiFi**: Ensure ESP32 is connected to WiFi
2. **Verify Credentials**: Double-check username/password in HiveMQ dashboard
3. **Check Serial Output**: Monitor ESP32 serial output for connection errors
4. **Firewall**: Ensure port 8883 is not blocked

### Common Error Codes

- `-2`: Connection refused (check credentials)
- `-3`: Server unavailable (check broker URL)
- `-4`: Bad username/password
- `-5`: Not authorized

### Debug Mode

Enable debug logging by setting the log level in `platformio.ini`:

```ini
-DCORE_DEBUG_LEVEL=4  ; Enable DEBUG level logging
```

## Advanced Usage

### Custom Commands

You can extend the MQTT functionality by adding new commands in `src/MQTTManager.cpp`:

```cpp
else if (command == "custom_command") {
    // Handle custom command
    publishEvent("custom_command_executed");
}
```

### Integration with Home Assistant

Add this to your `configuration.yaml`:

```yaml
mqtt:
  sensor:
    - name: "Computer Controller Status"
      state_topic: "computer-controller/status"
      value_template: "{{ value_json.pc_powered }}"
      
  switch:
    - name: "PC Power"
      command_topic: "computer-controller/control"
      payload_on: '{"command": "power_on"}'
      payload_off: '{"command": "power_off"}'
```

### Web Dashboard

You can create a web dashboard using MQTT.js:

```javascript
const client = mqtt.connect('wss://your-broker-host:8884/mqtt', {
  username: 'your-username',
  password: 'your-password'
});

client.subscribe('computer-controller/status');
client.on('message', (topic, message) => {
  const status = JSON.parse(message.toString());
  updateDashboard(status);
});
```

## Security Considerations

1. **Use Strong Passwords**: Create unique, strong passwords for your HiveMQ account
2. **TLS Encryption**: All communication is encrypted using TLS
3. **Client IDs**: Each ESP32 uses a unique client ID to prevent conflicts
4. **Access Control**: HiveMQ Cloud provides access control and monitoring
5. **Credential Protection**: Never commit credentials to version control

## Support

If you encounter issues:

1. Check the serial monitor output for error messages
2. Verify your HiveMQ Cloud configuration
3. Test with the Python client first
4. Check the ESP32's WiFi connection

## License

This MQTT integration is part of the Computer Controller project and follows the same license terms. 