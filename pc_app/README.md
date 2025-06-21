# PC GPU Fan Controller

A lightweight Python utility that watches your NVIDIA RTX 5090 (or any NVML-compatible GPU) temperature and commands the ESP32-based *ComputerController* to control an auxiliary fan with linear PWM.

---

## Features

* **Automatic ESP32 discovery** – detects the serial port by VID/PID (CP210x, CH340, native USB).  
  Override with `--port /dev/ttyUSB0` if autodetection fails.
* **NVML integration** – reads GPU temperature every second using `pynvml`.
* **Linear PWM control** – fan speed scales smoothly from 0% to 100% between 50°C and 80°C.
* **Simple protocol** – sends `gpufan <speed>` commands understood by the firmware.

## Quick start

```bash
cd pc_app
python3 -m venv venv          # optional but recommended
source venv/bin/activate
pip install -r requirements.txt

python gpu_fan_controller.py  # run with defaults
```

### Arguments

| Flag | Default | Description |
|------|---------|-------------|
| `--high` | `80` | Maximum temperature (°C) for 100% fan speed |
| `--low`  | `50` | Minimum temperature (°C) for 0% fan speed |
| `--port` | *auto* | Serial device (e.g., `/dev/ttyUSB0`) |
| `--log`  | `info` | Log level: `debug`, `info`, `warning`, `error` |

## Systemd service (Linux)

Create `/etc/systemd/system/gpu-fan.service`:

```ini
[Unit]
Description=GPU auxiliary fan controller
After=multi-user.target

[Service]
Type=simple
ExecStart=/usr/bin/python3 /home/$USER/Developments/ComputerController/pc_app/gpu_fan_controller.py
Restart=on-failure

[Install]
WantedBy=multi-user.target
```

Then enable:

```bash
sudo systemctl daemon-reload
sudo systemctl enable --now gpu-fan.service
```

## Limitations / Todo

* Assumes a single GPU at index 0. Extendable via `pynvml`.
* Tested on Linux; Windows/macOS should work with the same libraries. 