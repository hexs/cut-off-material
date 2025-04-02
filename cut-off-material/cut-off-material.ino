#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "Adafruit_Keypad.h"

// LCD configuration
LiquidCrystal_I2C lcd(0x27, 20, 4);

// Global state variables
bool editMode = true;
bool previousLimitState = HIGH;  // Using HIGH as default (INPUT_PULLUP)
int count = 0;
String inputPlanKey = "";

// Keypad configuration
const byte ROWS = 4;
const byte COLS = 4;
char keys[ROWS][COLS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};
byte rowPins[ROWS] = {11, 10, 9, 8};
byte colPins[COLS] = {7, 6, 5, 4};
Adafruit_Keypad customKeypad = Adafruit_Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS);

// Timing variables
unsigned long lastUpdateTime = 0;
const unsigned long updateInterval = 100;  // 100 ms update interval

// Function to reset and update the LCD display with default texts
void resetDisplay() {
  lcd.setCursor(0, 0);
  lcd.print("  Cut Off Material  ");
  lcd.setCursor(0, 1);
  lcd.print("Set Plan Cutoff:    ");
  lcd.setCursor(0, 2);
  lcd.print("Acl Plan       :    ");
  lcd.setCursor(0, 3);
  lcd.print("A=reset   B=start   ");
}

void setup() {
  Serial.begin(9600);

  // Configure pins
  pinMode(3, INPUT_PULLUP);   // Limit switch
  pinMode(A0, OUTPUT);        // Relay 1
  pinMode(A1, OUTPUT);        // Relay 2

  // Initialize LCD
  lcd.init();
  lcd.backlight();
  resetDisplay();

  // Initialize keypad
  customKeypad.begin();
}

void loop() {
  // Process keypad events
  customKeypad.tick();
  while (customKeypad.available()) {
    keypadEvent e = customKeypad.read();
    char keyChar = (char)e.bit.KEY;

    if (e.bit.EVENT == KEY_JUST_PRESSED) {// pressed
      // Handle key events
      if (keyChar == 'A') {  // Reset/edit mode
        editMode = true;
        count = 0;
        inputPlanKey = "";
        resetDisplay();
      }
      else if (keyChar == 'B') {  // Start mode
        editMode = false;
      }
      else if ((char)e.bit.KEY == 'C') {
      }
      else if ((char)e.bit.KEY == 'D') {
      }
      else if ((char)e.bit.KEY == '*') {
      }
      else if ((char)e.bit.KEY == '#') {
      }
      else {
        if (editMode && inputPlanKey.length() < 3) {
          inputPlanKey += keyChar;
          lcd.setCursor(16, 1);
          lcd.print(inputPlanKey);
        }
      }
    }

    else if (e.bit.EVENT == KEY_JUST_RELEASED) {// released
    }
  }

  // Periodic display update every updateInterval ms
  if (millis() - lastUpdateTime > updateInterval) {
    lastUpdateTime = millis();

    // Update the count on the LCD (row 2, column 16)
    lcd.setCursor(16, 2);
    lcd.print(count);

    // Show a cursor indicator if in edit mode
    lcd.setCursor(16 + inputPlanKey.length(), 1);
    lcd.print(editMode ? "_" : " ");

    // Convert the input string to an integer plan value
    int inputPlan = inputPlanKey.toInt();

    // If running and count reaches or exceeds the plan value, activate output
    if (!editMode && count >= inputPlan) {
      digitalWrite(A0, HIGH);
      digitalWrite(A1, HIGH);
    } else {
      digitalWrite(A0, LOW);
      digitalWrite(A1, LOW);
    }
  }

  // Monitor the limit switch: detect a falling edge (HIGH to LOW transition)
  bool currentLimitState = digitalRead(3);
  if (!editMode && previousLimitState == HIGH && currentLimitState == LOW) {
    count++;
    delay(100);  // Debounce delay
  }
  previousLimitState = currentLimitState;
}
