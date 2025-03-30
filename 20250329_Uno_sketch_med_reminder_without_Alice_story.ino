#include <Wire.h>
#include "rgb_lcd.h"

rgb_lcd lcd;

// Pins
const int buttonPin = 3;
const int blueLED = 4;
const int greenLED = 5;
const int redLED = 6;
const int yellowLED = 7;

// Medication info
const char* medNames[] = {"Vitamin A", "Vitamin B", "Vitamin C", "Calcium"};
const int pillCounts[] = {1, 2, 3, 1};
const int ledPins[] = {blueLED, greenLED, redLED, yellowLED};
const int ledColors[][3] = {
  {0, 0, 255},       // Blue
  {0, 255, 0},       // Green
  {255, 0, 0},       // Red
  {255, 255, 0}      // Yellow
};

const int totalMeds = 4;
int currentMed = 0;
bool sequenceDone = false;

unsigned long buttonPressStart = 0;
bool buttonPreviouslyPressed = false;
const unsigned long holdThreshold = 5000; // 5 seconds

void setup() {
  // Setup LED pins
  for (int i = 0; i < totalMeds; i++) {
    pinMode(ledPins[i], OUTPUT);
    digitalWrite(ledPins[i], LOW);
  }

  pinMode(buttonPin, INPUT_PULLUP);
  lcd.begin(16, 2);

  waitForStart();
  showCurrentMedication();
}

void loop() {
  bool buttonPressed = (digitalRead(buttonPin) == LOW);
  unsigned long currentTime = millis();

  if (buttonPressed) {
    if (!buttonPreviouslyPressed) {
      buttonPressStart = currentTime;
    } else if (currentTime - buttonPressStart >= holdThreshold) {
      resetToFirstMedication();
      delay(1000);
    }
  } else if (buttonPreviouslyPressed && !sequenceDone) {
    if (currentTime - buttonPressStart < holdThreshold) {
      rewardCurrentMedication();

      if (currentMed == totalMeds - 1) {
        showCompletionMessage();
        sequenceDone = true;
      } else {
        goToNextMedication();
      }
    }
  }

  buttonPreviouslyPressed = buttonPressed;
}

// -------------------- Display & Behavior Functions ------------------------

void waitForStart() {
  while (true) {
    // Show welcome message
    lcd.setRGB(255, 255, 255); // White
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Let's be healthy");
    lcd.setCursor(0, 1);
    lcd.print("and strong!");

    if (waitForButton(5000)) break;

    // Show instruction message
    lcd.setRGB(255, 0, 255); // Purple
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Take pills. Then");
    lcd.setCursor(0, 1);
    lcd.print("press button.");

    if (waitForButton(5000)) break;

       // Show instruction message
    lcd.setRGB(0, 255, 255); // aqua blue
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Press button to");
    lcd.setCursor(0, 1);
    lcd.print("start.");

    if (waitForButton(10000)) break; 
  }
}

bool waitForButton(unsigned long timeout) {
  unsigned long start = millis();
  while (millis() - start < timeout) {
    if (digitalRead(buttonPin) == LOW) {
      while (digitalRead(buttonPin) == LOW); // Wait for release
      return true;
    }
  }
  return false;
}

void showCurrentMedication() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Take ");
  lcd.print(pillCounts[currentMed]);
  lcd.print(pillCounts[currentMed] == 1 ? " pill" : " pills");

  lcd.setCursor(0, 1);
  lcd.print(medNames[currentMed]);

  setBacklightColor(
    ledColors[currentMed][0],
    ledColors[currentMed][1],
    ledColors[currentMed][2]
  );

  for (int i = 0; i < totalMeds; i++) {
    digitalWrite(ledPins[i], (i == currentMed) ? HIGH : LOW);
  }
}

void rewardCurrentMedication() {
  int pin = ledPins[currentMed];

  // Show the "Good job!" message FIRST
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Good job!");
  lcd.setCursor(0, 1);
  lcd.print(medNames[currentMed]);
  lcd.print(" done");

  // Then start blinking while message is showing
  unsigned long start = millis();
  while (millis() - start < 5000) {
    digitalWrite(pin, HIGH);
    delay(250);
    digitalWrite(pin, LOW);
    delay(250);
  }
}


void showCompletionMessage() {
  lcd.setRGB(0, 255, 255); // Cyan
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("All meds done!");
  lcd.setCursor(0, 1);
  lcd.print("Next dose: tmrw");

  for (int i = 0; i < totalMeds; i++) {
    digitalWrite(ledPins[i], LOW);
  }
}

void goToNextMedication() {
  currentMed++;
  showCurrentMedication();
}

void resetToFirstMedication() {
  currentMed = 0;
  sequenceDone = false;

  // Wait for user to release the button before continuing
  while (digitalRead(buttonPin) == LOW) {
    // Do nothing, just wait
  }

  waitForStart();
  showCurrentMedication();
}


void setBacklightColor(int r, int g, int b) {
  lcd.setRGB(r, g, b);
}