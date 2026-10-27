
#include <ThreeWire.h>
#include <RtcDS1302.h>
#include <Keypad.h>
#include <EEPROM.h>

// ================ RTC ================

ThreeWire myWire(7, 6, 8);  // DAT, CLK, RST
RtcDS1302<ThreeWire> rtc(myWire);

// ============== COMPONENTS ==============

const int buzzerPin = 3;
const int redLedPin = 4;
const int greenLedPin = 5;
const int ackButtonPin = 2;
const int yellowLedPin = 13;
const int doseButtonPin = A4;

const unsigned long BUZZER_LIMIT = 180000UL; // 3 minutes

// ============== BUTTON STATE ==============

bool ackButtonLastState = HIGH;
bool doseButtonLastState = HIGH;

// ============== KEYPAD ==============

const byte ROWS = 4;
const byte COLS = 4;

char keys[ROWS][COLS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};

byte rowPins[ROWS] = {A0, A1, A2, A3};
byte colPins[COLS] = {9, 10, 11, 12};

Keypad keypad = Keypad(
  makeKeymap(keys), rowPins, colPins, ROWS, COLS
);

// ============== SCHEDULE EEPROM ==============

const byte EEPROM_MARKER_ADDRESS = 0;
const byte EEPROM_MARKER = 123;

const int MORNING_HOUR_ADDR = 1;
const int MORNING_MIN_ADDR = 2;
const int AFTERNOON_HOUR_ADDR = 3;
const int AFTERNOON_MIN_ADDR = 4;
const int EVENING_HOUR_ADDR = 5;
const int EVENING_MIN_ADDR = 6;

int morningHour = 8;
int morningMinute = 0;
int afternoonHour = 14;
int afternoonMinute = 0;
int eveningHour = 20;
int eveningMinute = 0;

// ============== HISTORY EEPROM ==============

// 100 records x 7 bytes, addresses 16 through 715.
const int HISTORY_BASE = 16;
const int HISTORY_COUNT = 100;
const int HISTORY_RECORD_SIZE = 7;

const int HISTORY_INDEX_ADDR = 10;
const int HISTORY_MARKER_ADDR = 11;
const byte HISTORY_MARKER = 77;

int historyIndex = 0;

// Event codes
const byte EVENT_ALARM = 1;
const byte EVENT_ACK = 2;
const byte EVENT_MISSED = 3;
const byte EVENT_TAKEN = 4;

// ============== ALARM STATE ==============

// An alarm remains pending until the patient confirms
// the dose. Acknowledgement alone does not confirm it.

bool alarmPending = false;
bool alarmAcknowledged = false;
bool buzzerActive = false;
bool doseConfirmed = false;
bool missedLogged = false;

unsigned long alarmStartMillis = 0;

// Prevent the same scheduled minute triggering repeatedly.
int lastAlarmDay = -1;
int lastAlarmMonth = -1;
int lastAlarmYear = -1;
int lastAlarmHour = -1;
int lastAlarmMinute = -1;

// ============== KEYPAD INPUT ==============

bool settingTime = false;
char selectedMedicine = ' ';
String enteredTime = "";

// ============== FUNCTION DECLARATIONS ==============

void loadSchedules();
void setDefaultTimes();
void saveSchedules();
void initializeHistory();
void saveEvent(byte eventCode, RtcDateTime eventTime);
void showHistory();
void showSchedules();
void printTime(int hour, int minute);
void handleKey(char key);
bool buttonPressed(int pin, bool &lastState);

// ============== SETUP ==============

void setup()
{
  Serial.begin(9600);

  pinMode(buzzerPin, OUTPUT);
  pinMode(redLedPin, OUTPUT);
  pinMode(greenLedPin, OUTPUT);
  pinMode(yellowLedPin, OUTPUT);

  pinMode(ackButtonPin, INPUT_PULLUP);
  pinMode(doseButtonPin, INPUT_PULLUP);

  // Read initial button states to avoid false presses at startup.
  ackButtonLastState = digitalRead(ackButtonPin);
  doseButtonLastState = digitalRead(doseButtonPin);

  digitalWrite(buzzerPin, LOW);
  digitalWrite(redLedPin, LOW);
  digitalWrite(greenLedPin, LOW);

  rtc.Begin();

  loadSchedules();
  initializeHistory();

  // All three schedules are always configured in this version.
  digitalWrite(yellowLedPin, HIGH);

  Serial.println();
  Serial.println("DOSEGUARD MEDICINE REMINDER");
  Serial.println("Schedules and history ready.");
  Serial.println("Yellow LED: reminder system active.");
  Serial.println("Red LED: reminder not acknowledged.");
  Serial.println("Green LED: dose reported taken.");

  showSchedules();

  Serial.println("A = Set morning time");
  Serial.println("B = Set afternoon time");
  Serial.println("C = Set evening time");
  Serial.println("D = View event history");
}

// ============== MAIN LOOP ==============

void loop()
{
  RtcDateTime now = rtc.GetDateTime();

  char key = keypad.getKey();

  if (key)
  {
    handleKey(key);
  }

  bool medicineTime =
    (now.Hour() == morningHour &&
     now.Minute() == morningMinute) ||

    (now.Hour() == afternoonHour &&
     now.Minute() == afternoonMinute) ||

    (now.Hour() == eveningHour &&
     now.Minute() == eveningMinute);

  bool alreadyTriggered =
    now.Day() == lastAlarmDay &&
    now.Month() == lastAlarmMonth &&
    now.Year() == lastAlarmYear &&
    now.Hour() == lastAlarmHour &&
    now.Minute() == lastAlarmMinute;

  // Start a new reminder once in the scheduled minute.
  if (medicineTime && !alreadyTriggered)
  {
    alarmPending = true;
    alarmAcknowledged = false;
    buzzerActive = true;
    doseConfirmed = false;
    missedLogged = false;

    alarmStartMillis = millis();

    lastAlarmDay = now.Day();
    lastAlarmMonth = now.Month();
    lastAlarmYear = now.Year();
    lastAlarmHour = now.Hour();
    lastAlarmMinute = now.Minute();

    digitalWrite(greenLedPin, LOW);

    Serial.println();
    Serial.println("ALARM: TIME TO TAKE MEDICINE!");
    Serial.println("Press the acknowledgement button to silence.");
    Serial.println("Press the second button to report dose taken.");

    saveEvent(EVENT_ALARM, now);
  }

  // Stop the buzzer after three minutes.
  // The red LED remains on if the alarm was not acknowledged.
  if (buzzerActive &&
      millis() - alarmStartMillis >= BUZZER_LIMIT)
  {
    buzzerActive = false;

    if (!alarmAcknowledged && !missedLogged)
    {
      missedLogged = true;

      RtcDateTime eventTime = rtc.GetDateTime();
      saveEvent(EVENT_MISSED, eventTime);

      Serial.println(
        "UNACKNOWLEDGED: Buzzer stopped after 3 minutes."
      );
      Serial.println("Red LED remains on.");
    }
    else
    {
      Serial.println("Buzzer stopped after 3 minutes.");
    }
  }

  // Button 1: acknowledge the reminder.
  // This silences the buzzer and turns off the red LED.
  // It does NOT report that the medicine was taken.
  if (buttonPressed(ackButtonPin, ackButtonLastState))
  {
    if (alarmPending && !alarmAcknowledged)
    {
      alarmAcknowledged = true;
      buzzerActive = false;

      RtcDateTime ackTime = rtc.GetDateTime();
      saveEvent(EVENT_ACK, ackTime);

      Serial.println("ACKNOWLEDGED: Reminder responded to.");
      Serial.println("Red LED OFF. Dose not yet confirmed.");
    }
  }

  // Button 2: patient reports that the dose was taken.
  if (buttonPressed(doseButtonPin, doseButtonLastState))
  {
    if (alarmPending && !doseConfirmed)
    {
      doseConfirmed = true;
      alarmPending = false;
      buzzerActive = false;

      digitalWrite(buzzerPin, LOW);
      digitalWrite(redLedPin, LOW);
      digitalWrite(greenLedPin, HIGH);

      RtcDateTime takenTime = rtc.GetDateTime();
      saveEvent(EVENT_TAKEN, takenTime);

      Serial.println(
        "DOSE CONFIRMED: Patient reports taking medicine."
      );
      Serial.println("Green LED ON until the next reminder.");
    }
    else
    {
      Serial.println(
        "No unconfirmed reminder is currently pending."
      );
    }
  }

  // Update outputs from the current state.
  digitalWrite(buzzerPin, buzzerActive ? HIGH : LOW);

  digitalWrite(
    redLedPin,
    (alarmPending && !alarmAcknowledged) ? HIGH : LOW
  );

  digitalWrite(greenLedPin, doseConfirmed ? HIGH : LOW);

  // Schedules are configured in this version, so yellow stays on.
  digitalWrite(yellowLedPin, HIGH);

  delay(50);
}

// ============== BUTTON DEBOUNCING ==============

// Returns true once for each valid press.
// INPUT_PULLUP means LOW = pressed.
bool buttonPressed(int pin, bool &lastState)
{
  bool currentState = digitalRead(pin);

  if (lastState == HIGH && currentState == LOW)
  {
    delay(25);

    currentState = digitalRead(pin);

    if (currentState == LOW)
    {
      lastState = LOW;
      return true;
    }
  }

  lastState = currentState;
  return false;
}

// ============== SCHEDULE STORAGE ==============

void loadSchedules()
{
  if (EEPROM.read(EEPROM_MARKER_ADDRESS) == EEPROM_MARKER)
  {
    morningHour = EEPROM.read(MORNING_HOUR_ADDR);
    morningMinute = EEPROM.read(MORNING_MIN_ADDR);

    afternoonHour = EEPROM.read(AFTERNOON_HOUR_ADDR);
    afternoonMinute = EEPROM.read(AFTERNOON_MIN_ADDR);

    eveningHour = EEPROM.read(EVENING_HOUR_ADDR);
    eveningMinute = EEPROM.read(EVENING_MIN_ADDR);

    if (morningHour > 23 || morningMinute > 59 ||
        afternoonHour > 23 || afternoonMinute > 59 ||
        eveningHour > 23 || eveningMinute > 59)
    {
      setDefaultTimes();
    }
  }
  else
  {
    setDefaultTimes();
  }
}

void setDefaultTimes()
{
  morningHour = 8;
  morningMinute = 0;
  afternoonHour = 14;
  afternoonMinute = 0;
  eveningHour = 20;
  eveningMinute = 0;

  saveSchedules();
}

void saveSchedules()
{
  EEPROM.update(EEPROM_MARKER_ADDRESS, EEPROM_MARKER);

  EEPROM.update(MORNING_HOUR_ADDR, morningHour);
  EEPROM.update(MORNING_MIN_ADDR, morningMinute);

  EEPROM.update(AFTERNOON_HOUR_ADDR, afternoonHour);
  EEPROM.update(AFTERNOON_MIN_ADDR, afternoonMinute);

  EEPROM.update(EVENING_HOUR_ADDR, eveningHour);
  EEPROM.update(EVENING_MIN_ADDR, eveningMinute);
}

// ============== HISTORY STORAGE ==============

void initializeHistory()
{
  if (EEPROM.read(HISTORY_MARKER_ADDR) == HISTORY_MARKER)
  {
    historyIndex = EEPROM.read(HISTORY_INDEX_ADDR);

    if (historyIndex >= HISTORY_COUNT)
    {
      historyIndex = 0;
    }
  }
  else
  {
    historyIndex = 0;

    EEPROM.update(HISTORY_INDEX_ADDR, historyIndex);
    EEPROM.update(HISTORY_MARKER_ADDR, HISTORY_MARKER);
  }
}

void saveEvent(byte eventCode, RtcDateTime eventTime)
{
  int address =
    HISTORY_BASE + historyIndex * HISTORY_RECORD_SIZE;

  EEPROM.update(address, eventCode);
  EEPROM.update(address + 1, eventTime.Day());
  EEPROM.update(address + 2, eventTime.Month());
  EEPROM.update(address + 3, eventTime.Hour());
  EEPROM.update(address + 4, eventTime.Minute());
  EEPROM.update(address + 5, eventTime.Year() & 0xFF);
  EEPROM.update(address + 6, eventTime.Year() >> 8);

  historyIndex = (historyIndex + 1) % HISTORY_COUNT;

  EEPROM.update(HISTORY_INDEX_ADDR, historyIndex);
  EEPROM.update(HISTORY_MARKER_ADDR, HISTORY_MARKER);
}

void showHistory()
{
  Serial.println();
  Serial.println("===== SAVED EVENT HISTORY =====");

  int recordsFound = 0;

  for (int i = 0; i < HISTORY_COUNT; i++)
  {
    int slot = (historyIndex + i) % HISTORY_COUNT;
    int address = HISTORY_BASE + slot * HISTORY_RECORD_SIZE;

    byte eventCode = EEPROM.read(address);

    if (eventCode < EVENT_ALARM || eventCode > EVENT_TAKEN)
    {
      continue;
    }

    byte day = EEPROM.read(address + 1);
    byte month = EEPROM.read(address + 2);
    byte hour = EEPROM.read(address + 3);
    byte minute = EEPROM.read(address + 4);

    unsigned int year =
      EEPROM.read(address + 5) |
      ((unsigned int)EEPROM.read(address + 6) << 8);

    // Basic validation prevents many stale entries being displayed.
    if (month < 1 || month > 12 ||
        day < 1 || day > 31 ||
        hour > 23 || minute > 59 ||
        year < 2020 || year > 2099)
    {
      continue;
    }

    Serial.print(year);
    Serial.print("/");
    Serial.print(month);
    Serial.print("/");
    Serial.print(day);
    Serial.print(" ");

    if (hour < 10) Serial.print("0");
    Serial.print(hour);
    Serial.print(":");

    if (minute < 10) Serial.print("0");
    Serial.print(minute);
    Serial.print(" - ");

    if (eventCode == EVENT_ALARM)
      Serial.println("ALARM TRIGGERED");
    else if (eventCode == EVENT_ACK)
      Serial.println("ACKNOWLEDGED");
    else if (eventCode == EVENT_MISSED)
      Serial.println("NOT ACKNOWLEDGED IN 3 MINUTES");
    else if (eventCode == EVENT_TAKEN)
      Serial.println("DOSE REPORTED TAKEN");

    recordsFound++;
  }

  if (recordsFound == 0)
  {
    Serial.println("No valid events recorded yet.");
  }

  Serial.println("===============================");
}

// ============== DISPLAY SCHEDULES ==============

void showSchedules()
{
  Serial.print("Morning: ");
  printTime(morningHour, morningMinute);

  Serial.print("Afternoon: ");
  printTime(afternoonHour, afternoonMinute);

  Serial.print("Evening: ");
  printTime(eveningHour, eveningMinute);

  Serial.println();
}

void printTime(int hour, int minute)
{
  if (hour < 10) Serial.print("0");
  Serial.print(hour);
  Serial.print(":");

  if (minute < 10) Serial.print("0");
  Serial.println(minute);
}

// ============== KEYPAD HANDLER ==============

void handleKey(char key)
{
  if (!settingTime)
  {
    if (key == 'D')
    {
      showHistory();
      return;
    }

    if (key == 'A' || key == 'B' || key == 'C')
    {
      selectedMedicine = key;
      settingTime = true;
      enteredTime = "";

      if (key == 'A')
        Serial.println("Set MORNING time (HHMM):");
      else if (key == 'B')
        Serial.println("Set AFTERNOON time (HHMM):");
      else
        Serial.println("Set EVENING time (HHMM):");

      Serial.println("# = Save, * = Cancel");
    }

    return;
  }

  if (key == '*')
  {
    settingTime = false;
    enteredTime = "";

    Serial.println("Setting cancelled.");
    return;
  }

  if (key >= '0' && key <= '9')
  {
    if (enteredTime.length() < 4)
    {
      enteredTime += key;
      Serial.print("Entered: ");
      Serial.println(enteredTime);
    }

    return;
  }

  if (key == '#')
  {
    if (enteredTime.length() != 4)
    {
      Serial.println("Enter four digits, e.g. 0830.");
      return;
    }

    int hour = enteredTime.substring(0, 2).toInt();
    int minute = enteredTime.substring(2, 4).toInt();

    if (hour > 23 || minute > 59)
    {
      Serial.println("Invalid time. Use 0000 to 2359.");
      enteredTime = "";
      return;
    }

    if (selectedMedicine == 'A')
    {
      morningHour = hour;
      morningMinute = minute;
    }
    else if (selectedMedicine == 'B')
    {
      afternoonHour = hour;
      afternoonMinute = minute;
    }
    else if (selectedMedicine == 'C')
    {
      eveningHour = hour;
      eveningMinute = minute;
    }

    saveSchedules();

    Serial.print("Schedule saved: ");
    printTime(hour, minute);

    showSchedules();

    settingTime = false;
    enteredTime = "";
  }
}