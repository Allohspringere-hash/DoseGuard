# DoseGuard — Wiring Reference

This document records the current hardware pin assignments for the Arduino Uno prototype.

## Pin Mapping

| Component                | Arduino connection | Purpose                      |
| ------------------------ | ------------------ | ---------------------------- |
| DS1302 DAT               | D7                 | RTC data                     |
| DS1302 CLK               | D6                 | RTC clock                    |
| DS1302 RST               | D8                 | RTC reset/enable             |
| Active buzzer            | D3                 | Audible reminder             |
| Red LED                  | D4                 | Pending reminder alert       |
| Green LED                | D5                 | Dose reported taken          |
| Yellow LED               | D13                | System status                |
| Acknowledgement button   | D2 to GND          | Silence/acknowledge reminder |
| Dose confirmation button | A4 to GND          | Report dose taken            |
| Keypad rows              | A0–A3              | Keypad row signals           |
| Keypad columns           | D9–D12             | Keypad column signals        |

## Button Configuration

The two pushbuttons use the Arduino's internal pull-up configuration (`INPUT_PULLUP`). Each button connects its input pin to GND when pressed.

## Important Notes

* Confirm the pin mapping against the actual hardware before reconnecting components.
* LED wiring must use suitable current-limiting resistors.
* Check the buzzer's voltage and current requirements before powering it from an Arduino pin.
* This document describes the prototype wiring, not a certified medical-device design.
