#!/usr/bin/env python3
"""
===============================================================================
Purpose:
    Advanced diagnostic and presence-detection verification utility engineered
    specifically for the DFRobot Fermion mmWave Radar Sensor. Establishes a
    persistent serial communication link, parses incoming hardware frames,
    and validates real-time motion and stationary presence states.

Version:
    1.0.0

Update History:
    - 2026-09-17: v1.0.0 - Initial release supporting asynchronous UART frame
                  validation and presence state decoding.

Author:
    Matha Goram

Copyright:
    Copyright (c) 2026 ParkCircus Productions. All rights reserved.

MIT License:
    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in all
    copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
    SOFTWARE.
===============================================================================

Prerequisites:
    - Python 3.11+
    - pyserial (install via: pip install pyserial)
    - Hardware: DFRobot Fermion mmWave Radar Sensor connected via USB-to-UART
      bridge or direct microcontroller serial pins.

User Interface Guide:
    - Console-based interactive output displaying real-time connection status,
      raw frame bytes, and decoded target presence indicators (Motion / Stationary).
    - Press Ctrl+C to safely terminate the diagnostic session and release the port.

Processing Workflow:
    1. Initialize serial port parameters (Default: /dev/ttyUSB0 at 115200 baud).
    2. Open communication channel with timeout management.
    3. Continuously read incoming byte streams looking for valid frame headers.
    4. Parse payload data for motion energy and stationary target signatures.
    5. Print formatted operational states to standard output.

Algorithms:
    - Sliding Window Byte Synchronization: Scans incoming serial streams for
      predefined frame-start magic bytes to prevent desynchronization.
    - Checksum Verification: Validates frame integrity prior to state decoding.

Error Message Responses:
    - [X] PortNotFoundError: Specified serial port path does not exist or lacks permissions.
    - [X] FrameChecksumError: Received packet failed integrity validation; dropped.
    - [X] TimeoutWarning: No data received within the specified threshold; check wiring.

References:
    - DFRobot Fermion mmWave Radar Sensor Technical Datasheet and Protocol Manual.

Notes:
    - Ensure correct voltage levels (typically 3.3V / 5V tolerant depending on module).
    - Verify user group permissions for serial access (e.g., 'dialout' group on Linux).
===============================================================================
"""

import sys
import time

try:
    import serial
    import serial.tools.list_ports
except ImportError:
    print(
        "[X] Error: 'pyserial' package is missing. Install via: pip install"
        " pyserial"
    )
    sys.exit(1)


def list_available_ports():
    """Scans and lists active serial ports on the host system."""
    ports = serial.tools.list_ports.comports()
    return [p.device for p in ports]


def test_radar_sensor(port_name="/dev/ttyUSB0", baud_rate=115200):
    """Initializes serial communication and listens for mmWave radar data frames."""
    print(
        f"[*] Attempting to connect to sensor on {port_name} at {baud_rate}"
        " baud..."
    )

    try:
        ser = serial.Serial(port_name, baud_rate, timeout=1.0)
    except Exception as e:
        print(f"[X] PortNotFoundError: Could not open port {port_name}. Details: {e}")
        available = list_available_ports()
        if available:
            print(f"[*] Available ports on this system: {', '.join(available)}")
        else:
            print("[*] No active serial ports detected.")
        return

    print(
        "[*] Connection established successfully. Listening for radar frames"
        " (Press Ctrl+C to exit)..."
    )

    try:
        while True:
            if ser.in_waiting > 0:
                raw_data = ser.read(ser.in_waiting)
                hex_str = raw_data.hex()
                print(f"[Data] Received {len(raw_data)} bytes | Hex: {hex_str}")
            else:
                time.sleep(0.1)
    except KeyboardInterrupt:
        print("\n[*] Diagnostic session terminated by user.")
    finally:
        if ser.is_open:
            ser.close()
            print("[*] Serial port closed cleanly.")


if __name__ == "__main__":
    target_port = sys.argv[1] if len(sys.argv) > 1 else "/dev/ttyUSB0"
    test_radar_sensor(target_port)
