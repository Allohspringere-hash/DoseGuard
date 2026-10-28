DoseGuard — Test Report
1. Purpose
This document records the functional testing of the DoseGuard Arduino-based medicine reminder prototype. It distinguishes tests reported as passed from tests that still require further verification.
2. Test Environment
Microcontroller: Arduino Uno
Real-time clock: DS1302
Outputs: Active buzzer and red, green, and yellow LEDs
Inputs: Acknowledgement button, dose-reporting button, and 4×4 keypad
Storage: Arduino EEPROM
Development environment: Arduino IDE
Test date: To be recorded
3. Functional Test Results
The following results are based on the developer’s previously reported tests.
ID Test Expected behaviour Reported result
T01 Compile and upload Sketch compiles and uploads successfully PASS
T02 System status Yellow status LED illuminates PASS
T03 Reminder activation Buzzer and red LED activate at the configured reminder time PASS
T04 Reminder acknowledgement Acknowledgement button silences the buzzer and turns off the red LED PASS
T05 Dose reporting Separate button illuminates the green LED and records a dose-taken report PASS
T06 Alarm timeout Buzzer stops after three minutes without acknowledgement; red LED remains on PASS
T07 Restart and stored data Configured schedules and event history remain available after a restart PASS
Evidence note: These results reflect tests reported by the developer. Detailed test dates, repeated trials, measurements, and supporting photographs have not yet been recorded in this report.
4. Additional Reliability Tests Required
ID Test What must be verified Status
R01 RTC timekeeping Clock retains the correct time and advances correctly after USB power is removed Further verification required
R02 Long-duration operation Reminder timing remains correct during extended operation Not yet documented
R03 Power interruption during a pending alarm Behaviour after power is lost while a reminder is active Not yet documented
R04 Alarm recovery after restart No unexpected or duplicate alarm occurs after restarting near a scheduled reminder Not yet documented
R05 Event-history integrity Event types, dates, times, and record ordering are correct Not yet documented
R06 Repeated button presses Inputs do not cause unintended repeated events Not yet documented
R07 Schedule boundaries Reminders trigger correctly around minute changes and across midnight Not yet documented
5. Known Limitations
The device records a user’s report that a dose was taken; it does not verify swallowing.
The current design has not been clinically validated.
Pending-alarm state and timeout progress may not be fully preserved across power loss.
The event-history system requires further verification, including chronological ordering.
Initial functional tests do not establish long-term reliability.
6. Conclusion
The prototype has demonstrated its main reminder and user-input functions in initial testing. Further systematic testing is required to assess timing accuracy, recovery from power interruptions, event-history integrity, and longer-term reliability.
DoseGuard is an educational prototype and must not be relied upon as the sole method of medication management.
7. Future Test Record
For each new test, record:
Test ID and date
Hardware and software version
Exact procedure
Expected result
Actual result
PASS or FAIL
Relevant observations and evidence