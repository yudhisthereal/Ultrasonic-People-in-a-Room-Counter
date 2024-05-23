#line 1 "/home/yudhis/Documents/Kuliah/Embed/proyek/ultrasonic_person_counter/README.md"
# Ultrasonic Person-in-a-Room Counter

## Overview

This Arduino-based project utilizes ultrasonic sensors to count the number of people entering and exiting a room. It provides real-time monitoring of the room occupancy and controls a relay to manage the room's lighting based on occupancy status.

## Features

- Counts the number of people entering and exiting a room.
- Displays real-time occupancy count on an LCD screen.
- Automatically controls room lighting using a relay.
- Provides feedback messages on the LCD for user interaction.
- Utilizes ultrasonic sensors for accurate distance measurement.

## Hardware Requirements

- Arduino board (Tested on Arduino Uno)
- Ultrasonic sensors (2x)
- Relay module
- LCD display 16x2 with I2C module
- Push button for reset (optional)
- Resistors and jumper wires

## Installation and Usage

1. Connect the hardware components according to the specified pin configurations.
2. Upload the provided Arduino sketch (`ultrasonic_person_counter.ino`) to your Arduino board.
3. Open the serial monitor to view debugging messages (baud rate: 9600).
4. Monitor the LCD display for real-time occupancy count and feedback messages.
5. Press the reset button (if available) to reset the LCD display.

## Dependencies

- Arduino IDE / Visual Studio Code with Arduino Extension
- Wire.h (Arduino library)
- LiquidCrystal_I2C.h (Arduino library)

## Contributors

- Yudhistira (https://github.com/yudhisthereal) - Developer
