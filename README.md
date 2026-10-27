DoseGuard

An Arduino-based medicine reminder prototype

DoseGuard is a student-built embedded systems project designed to help users manage scheduled medication reminders and record their responses.

Current Features

• Three configurable daily reminder schedules
• Audible alarm and visual LED alerts
• Button to acknowledge a reminder
• Separate button to report that a dose has been taken
• Event history stored in Arduino EEPROM
• Real-time clock using the DS1302 module
• Keypad-based schedule configuration
• Alarm buzzer timeout after three minutes

Hardware

• Arduino Uno
• DS1302 real-time clock module
• 4×4 matrix keypad
• Active buzzer
• LEDs
• Pushbuttons
• Breadboard and jumper wires

How It Works

1. The real-time clock tracks the current date and time.
2. DoseGuard checks the time against configured reminder schedules.
3. When a reminder becomes due, the buzzer and red LED alert the user.
4. The user can acknowledge the alert using a button.
5. A separate button lets the user report that the dose was taken.
6. Events are recorded in the device’s EEPROM for later review.

Testing

The current prototype has passed the developer’s initial functional tests for reminder alerts, button responses, dose reporting, alarm timeout, and schedule/history retention across a restart.

Further testing and documentation are needed to establish reliability over longer periods and under different operating conditions.

Limitations and Safety

• DoseGuard is an educational prototype, not a clinically validated medical device.
• A button press records a user’s report; it cannot prove that medication was swallowed.
• The current prototype does not provide remote monitoring or automatic notifications to healthcare professionals.
• Its reliability and usability require further evaluation before any real-world medical deployment.

Do not rely on this prototype as your sole medication-management method.

Future Improvements

• LCD interface
• Improved power-loss and alarm-state recovery
• More comprehensive testing and event-history validation
• Enclosure and usability improvements
• Clearer setup and wiring documentation

Project Status

Educational prototype under active development.

Author

Biomedical Engineering student — University of Bamenda, Cameroon.