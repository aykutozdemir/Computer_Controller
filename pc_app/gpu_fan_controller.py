#!/usr/bin/env python3
"""
GPU Temperature Monitor & ESP32 Fan Controller
---------------------------------------------

This script runs on the PC side. It automatically locates the connected
ESP32 (ComputerController firmware) via the serial port, then checks the
RTX 5090 GPU temperature once per second. If the temperature exceeds a
configurable threshold, it sends a StaticSerialCommands directive to the
ESP32 to drive the external GPU fan from 0 → 100 % duty cycle.

Dependencies (see requirements.txt):
  • pyserial – serial port enumeration / IO
  • nvidia-ml-py3 (pynvml) – NVML bindings for GPU telemetry

Usage:
  python3 gpu_fan_controller.py [--high 80] [--low 70] [--port /dev/ttyUSB0]

  --high  High-temperature threshold in °C to enable the fan (default: 80)
  --low   Low-temperature threshold in °C to disable the fan (default: 70)
          (hysteresis prevents rapid toggling)
  --port  Explicit serial port. If omitted, the script auto-detects.

The script prints basic status information so it can be run as a systemd
service or from the terminal.
"""

from __future__ import annotations

import argparse
import logging
import sys
import time
from typing import Optional

import serial  # type: ignore
import serial.tools.list_ports  # type: ignore
from pynvml import (  # type: ignore
    nvmlDeviceGetHandleByIndex,
    nvmlDeviceGetTemperature,
    nvmlInit,
    nvmlShutdown,
    NVML_TEMPERATURE_GPU,
    nvmlDeviceGetName,
)
import textwrap  # after other imports

BAUD_RATE = 115200  # Must match SERIAL_BAUD_RATE on the ESP32
ESP32_VID_PID_CANDIDATES = {
    (0x10C4, 0xEA60),  # CP210x USB-to-UART (SiLabs) – common on ESP32 devkits
    (0x1A86, 0x7523),  # CH340/CH341 USB-Serial IC
    (0x303A, 0x1001),  # Espressif native USB (esp32-s3)
}

LOGGER = logging.getLogger("gpu_fan_controller")


def find_esp32_serial_port() -> Optional[str]:
    """Return the first serial port that looks like an ESP32.

    Detection strategy:
    1. Match VID/PID against known ESP32 USB-UART bridges.
    2. Fallback to the first /dev/ttyUSB* or /dev/ttyACM* device.
    """
    ports = list(serial.tools.list_ports.comports())

    # Primary: VID/PID match
    for port in ports:
        if port.vid is not None and port.pid is not None:
            if (port.vid, port.pid) in ESP32_VID_PID_CANDIDATES:
                return port.device

    # Secondary: name heuristic
    for port in ports:
        if "ttyUSB" in port.device or "ttyACM" in port.device:
            return port.device

    return None


def open_serial(port: str) -> serial.Serial:
    """Open a serial connection to the ESP32."""
    LOGGER.info("Opening serial port %s at %d baud", port, BAUD_RATE)
    ser = serial.Serial(port, BAUD_RATE, timeout=1)
    # Give the microcontroller a moment after DTR toggle reset (some boards reset on open)
    time.sleep(2)
    # Flush any boot messages
    ser.reset_input_buffer()
    ser.reset_output_buffer()
    return ser


def verify_identity(ser: serial.Serial, timeout: float = 2.0) -> bool:
    """Send the 'identity' command and confirm the expected response.

    Returns True if the device identifies as ComputerController, False otherwise.
    """
    ser.reset_input_buffer()
    ser.write(b"identity\n")
    ser.flush()

    deadline = time.time() + timeout
    response: list[str] = []

    while time.time() < deadline:
        line = ser.readline().decode(errors="ignore").strip()
        if not line:
            continue
        response.append(line)
        if line == "OK":
            break
    joined = "\n".join(response)
    LOGGER.debug("identity response:\n%s", joined)
    return "ComputerController" in joined


def send_fan_speed(ser: serial.Serial, speed: int) -> None:
    """Send a gpufan command to the ESP32.

    The StaticSerialCommands syntax expects a newline-terminated ASCII command.
    """
    speed = max(0, min(100, speed))
    cmd = f"gpufan {speed}\n".encode()
    LOGGER.debug("→ %s", cmd.strip().decode())
    ser.write(cmd)
    ser.flush()


def _calculate_speed(temp: int, low: int, high: int) -> int:
    """Return a fan duty percentage based on temperature.

    • ≤ low  → 0 %
    • ≥ high → 100 %
    • between → linear interpolation
    """
    if temp <= low:
        return 0
    if temp >= high:
        return 100
    # Linear interpolation
    ratio = (temp - low) / float(high - low)
    return int(round(ratio * 100))


class GpuMonitor:
    """Wrapper around NVML to fetch GPU temperature."""

    def __init__(self, index: int = 0):
        nvmlInit()
        self._handle = nvmlDeviceGetHandleByIndex(index)
        name = nvmlDeviceGetName(self._handle).decode()
        LOGGER.info("Monitoring GPU %02d: %s", index, name)

    def temperature(self) -> int:
        return int(nvmlDeviceGetTemperature(self._handle, NVML_TEMPERATURE_GPU))

    def shutdown(self):
        nvmlShutdown()


def run(port: Optional[str], high: int, low: int) -> None:
    if port is None:
        port = find_esp32_serial_port()
        if port is None:
            LOGGER.error("Could not locate ESP32 serial port. Use --port to specify explicitly.")
            sys.exit(1)

    ser = open_serial(port)

    if not verify_identity(ser):
        LOGGER.error("Connected device did not respond to identity command as expected. Aborting.")
        sys.exit(1)

    gpu = GpuMonitor(index=0)

    current_speed: int = 0  # Last speed sent to the ESP32 (percentage)
    send_fan_speed(ser, current_speed)  # Start with fan off

    try:
        while True:
            temp = gpu.temperature()
            LOGGER.info("GPU temperature: %d °C", temp)

            desired_speed = _calculate_speed(temp, low, high)

            # Only send command when the speed changes enough (≥2 % difference)
            if abs(desired_speed - current_speed) >= 2:
                LOGGER.debug("Updating fan speed → %d%%", desired_speed)
                current_speed = desired_speed
                send_fan_speed(ser, current_speed)

            # Sleep for 1 second (NVML polling interval)
            time.sleep(1)
    except KeyboardInterrupt:
        LOGGER.info("Interrupted by user – resetting fan and exiting …")
    finally:
        # Reset fan to 0 on exit for safety
        send_fan_speed(ser, 0)
        ser.close()
        gpu.shutdown()


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="GPU temperature watcher & ESP32 fan driver")
    parser.add_argument("--high", type=int, default=80, help="High temperature threshold (°C) to turn fan on")
    parser.add_argument("--low", type=int, default=70, help="Low temperature threshold (°C) to turn fan off")
    parser.add_argument("--port", type=str, help="Serial port of the ESP32 (auto-detect if omitted)")
    parser.add_argument("--log", type=str, default="info", choices=["debug", "info", "warning", "error"],
                        help="Logging level")
    args = parser.parse_args()

    logging.basicConfig(level=getattr(logging, args.log.upper()),
                        format="%(asctime)s | %(levelname)-8s | %(name)s: %(message)s",
                        datefmt="%H:%M:%S")

    if args.low >= args.high:
        LOGGER.error("--low must be lower than --high")
        sys.exit(1)

    run(args.port, args.high, args.low) 