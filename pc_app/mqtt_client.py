#!/usr/bin/env python3
"""
MQTT Client for Computer Controller
Connects to HiveMQ Cloud and sends control commands to ESP32
"""

import paho.mqtt.client as mqtt
import json
import time
import sys
import os
from typing import Optional

# Load credentials from config file
def load_credentials():
    """Load MQTT credentials from config file"""
    config_file = os.path.join(os.path.dirname(__file__), 'mqtt_config.json')
    
    if os.path.exists(config_file):
        try:
            with open(config_file, 'r') as f:
                config = json.load(f)
                return config
        except Exception as e:
            print(f"Error loading config: {e}")
    
    # Fallback to environment variables or default values
    return {
        'broker': os.getenv('MQTT_BROKER', 'your-broker-host'),
        'port': int(os.getenv('MQTT_PORT', '8883')),
        'username': os.getenv('MQTT_USERNAME', 'your-username'),
        'password': os.getenv('MQTT_PASSWORD', 'your-password')
    }

class ComputerControllerMQTT:
    def __init__(self, broker: str, port: int = 8883, username: str = None, password: str = None):
        self.broker = broker
        self.port = port
        self.username = username
        self.password = password
        self.client = None
        self.connected = False
        
        # Topics
        self.status_topic = "computer-controller/status"
        self.control_topic = "computer-controller/control"
        self.events_topic = "computer-controller/events"
        self.settings_topic = "computer-controller/settings"
        
    def on_connect(self, client, userdata, flags, rc):
        """Callback when connected to MQTT broker"""
        if rc == 0:
            print(f"Connected to MQTT broker {self.broker}")
            self.connected = True
            
            # Subscribe to status and events topics
            client.subscribe(self.status_topic)
            client.subscribe(self.events_topic)
            print(f"Subscribed to {self.status_topic} and {self.events_topic}")
        else:
            print(f"Failed to connect to MQTT broker, return code: {rc}")
            
    def on_disconnect(self, client, userdata, rc):
        """Callback when disconnected from MQTT broker"""
        print(f"Disconnected from MQTT broker, return code: {rc}")
        self.connected = False
        
    def on_message(self, client, userdata, msg):
        """Callback when message received"""
        try:
            payload = msg.payload.decode('utf-8')
            print(f"Received message on {msg.topic}: {payload}")
            
            if msg.topic == self.status_topic:
                self.handle_status_message(payload)
            elif msg.topic == self.events_topic:
                self.handle_event_message(payload)
                
        except Exception as e:
            print(f"Error processing message: {e}")
            
    def handle_status_message(self, payload: str):
        """Handle status messages from ESP32"""
        try:
            data = json.loads(payload)
            print(f"Status Update:")
            print(f"  Uptime: {data.get('uptime', 'N/A')} seconds")
            print(f"  WiFi RSSI: {data.get('wifi_rssi', 'N/A')} dBm")
            print(f"  Free Heap: {data.get('free_heap', 'N/A')} bytes")
            print(f"  PC Powered: {data.get('pc_powered', 'N/A')}")
            print(f"  Child Lock: {data.get('child_lock', 'N/A')}")
            print(f"  Buzzer Enabled: {data.get('buzzer_enabled', 'N/A')}")
        except json.JSONDecodeError as e:
            print(f"Error parsing status JSON: {e}")
            
    def handle_event_message(self, payload: str):
        """Handle event messages from ESP32"""
        try:
            data = json.loads(payload)
            event = data.get('event', 'unknown')
            event_data = data.get('data', '')
            timestamp = data.get('timestamp', 'N/A')
            print(f"Event: {event} (data: {event_data}) at {timestamp}")
        except json.JSONDecodeError as e:
            print(f"Error parsing event JSON: {e}")
    
    def connect(self) -> bool:
        """Connect to MQTT broker"""
        try:
            self.client = mqtt.Client()
            self.client.on_connect = self.on_connect
            self.client.on_disconnect = self.on_disconnect
            self.client.on_message = self.on_message
            
            # Set username/password if provided
            if self.username and self.password:
                self.client.username_pw_set(self.username, self.password)
            
            # Connect to broker
            self.client.connect(self.broker, self.port, 60)
            self.client.loop_start()
            
            # Wait for connection
            timeout = 10
            while not self.connected and timeout > 0:
                time.sleep(1)
                timeout -= 1
                
            return self.connected
            
        except Exception as e:
            print(f"Error connecting to MQTT broker: {e}")
            return False
    
    def disconnect(self):
        """Disconnect from MQTT broker"""
        if self.client:
            self.client.loop_stop()
            self.client.disconnect()
            self.connected = False
            
    def send_command(self, command: str, data: str = "") -> bool:
        """Send control command to ESP32"""
        if not self.connected:
            print("Not connected to MQTT broker")
            return False
            
        try:
            message = {
                "command": command,
                "data": data,
                "timestamp": int(time.time() * 1000)
            }
            
            payload = json.dumps(message)
            result = self.client.publish(self.control_topic, payload)
            
            if result.rc == mqtt.MQTT_ERR_SUCCESS:
                print(f"Command '{command}' sent successfully")
                return True
            else:
                print(f"Failed to send command '{command}', return code: {result.rc}")
                return False
                
        except Exception as e:
            print(f"Error sending command: {e}")
            return False
    
    def power_on(self) -> bool:
        """Power on the PC"""
        return self.send_command("power_on")
    
    def power_off(self) -> bool:
        """Power off the PC"""
        return self.send_command("power_off")
    
    def reset(self) -> bool:
        """Reset the PC"""
        return self.send_command("reset")
    
    def toggle_child_lock(self) -> bool:
        """Toggle child lock"""
        return self.send_command("toggle_child_lock")
    
    def toggle_buzzer(self) -> bool:
        """Toggle buzzer"""
        return self.send_command("toggle_buzzer")

def main():
    """Main function with interactive menu"""
    # Load MQTT Configuration from config file
    config = load_credentials()
    
    # Create MQTT client
    mqtt_client = ComputerControllerMQTT(
        config['broker'], 
        config['port'], 
        config['username'], 
        config['password']
    )
    
    # Connect to broker
    if not mqtt_client.connect():
        print("Failed to connect to MQTT broker")
        sys.exit(1)
    
    print("\nComputer Controller MQTT Client")
    print("================================")
    print("Available commands:")
    print("  1. Power On PC")
    print("  2. Power Off PC")
    print("  3. Reset PC")
    print("  4. Toggle Child Lock")
    print("  5. Toggle Buzzer")
    print("  6. Exit")
    print()
    
    try:
        while True:
            choice = input("Enter command (1-6): ").strip()
            
            if choice == "1":
                mqtt_client.power_on()
            elif choice == "2":
                mqtt_client.power_off()
            elif choice == "3":
                mqtt_client.reset()
            elif choice == "4":
                mqtt_client.toggle_child_lock()
            elif choice == "5":
                mqtt_client.toggle_buzzer()
            elif choice == "6":
                break
            else:
                print("Invalid choice. Please enter 1-6.")
                
            print()
            
    except KeyboardInterrupt:
        print("\nExiting...")
    finally:
        mqtt_client.disconnect()

if __name__ == "__main__":
    main() 