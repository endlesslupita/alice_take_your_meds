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
const char* plotItems[] = {"Potion", "Cake", "Tea", "Key"};
const char* medNames[] = {"Vitamin A", "Vitamin B", "Vitamin C", "Calcium"};
const int pillCounts[] = {1, 2, 1, 1};
const int ledPins[] = {blueLED, greenLED, redLED, yellowLED};
const int ledColors[][3] = {
  {0, 0, 255},      // Blue
  {0, 255, 0},      // Green
  {100, 30, 30},    // Soft Red
  {255, 255, 0}     // Yellow
};

const int totalMeds = 4;
int currentMed = 0;
bool sequenceDone = false;
bool hasStarted = false;

unsigned long buttonPressStart = 0;
bool buttonPreviouslyPressed = false;
bool buttonHandled = false;

const unsigned long holdThreshold = 5000;
const unsigned long debounceDelay = 50;
unsigned long lastDebounceTime = 0;

bool waitingForMedConfirmation = false;

void setup() {
  for (int i = 0; i < totalMeds; i++) {
    pinMode(ledPins[i], OUTPUT);
    digitalWrite(ledPins[i], LOW);
  }

  pinMode(buttonPin, INPUT_PULLUP);
  Serial.begin(9600);
  lcd.begin(16, 2);
}

void loop() {
  if (!hasStarted) {
    waitForStartButton();
    return;
  }

  unsigned long currentTime = millis();
  bool buttonPressed = (digitalRead(buttonPin) == LOW);

  if (buttonPressed && !buttonPreviouslyPressed) {
    buttonPressStart = currentTime;
    lastDebounceTime = currentTime;
    buttonHandled = false;
  }

  // Handle long hold to reset
  if (buttonPressed && !buttonHandled && (currentTime - buttonPressStart >= holdThreshold)) {
    resetToFirstMedication();
    delay(1000);
    buttonHandled = true;
  }

  // Handle short press (on release)
  if (!buttonPressed && buttonPreviouslyPressed &&
      (currentTime - lastDebounceTime > debounceDelay) &&
      !buttonHandled) {

    if (waitingForMedConfirmation) {
      handleMedicationCompletion();
    }

    buttonHandled = true;
  }

  buttonPreviouslyPressed = buttonPressed;
}

void waitForStartButton() {
  const char* messages[][2] = {
    {"Alice fell down", "a hole."},
    {"She's trapped!", ""},
    {"Help her escape.", ""},
    {"Take your meds.", ""},
    {"Press button to", "start."}
  };

  for (int i = 0; i < 5; i++) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(messages[i][0]);
    lcd.setCursor(0, 1);
    lcd.print(messages[i][1]);

    unsigned long start = millis();
    while (millis() - start < 3500) {
      if (digitalRead(buttonPin) == LOW) {
        while (digitalRead(buttonPin) == LOW);
        hasStarted = true;
        showPlotIntro(currentMed);
        showCurrentMedication();
        return;
      }
    }
  }

  // Final wait screen
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Press button to");
  lcd.setCursor(0, 1);
  lcd.print("start.");

  while (digitalRead(buttonPin) == HIGH);
  while (digitalRead(buttonPin) == LOW);
  hasStarted = true;
  showPlotIntro(currentMed);
  showCurrentMedication();
}

void showPlotIntro(int medIndex) {
  setBacklightColor(
    ledColors[medIndex][0],
    ledColors[medIndex][1],
    ledColors[medIndex][2]
  );

  if (medIndex == 0) {
    showMessage("Alice found a", "potion! Drink me.");
  } else if (medIndex == 1) {
    showMessage("Eat the cake", "to grow taller.");
  } else if (medIndex == 2) {
    showMessage("Drink tea to be", "normal again.");
  } else if (medIndex == 3) {
    showMessage("Now unlock the", "garden door.");
  }

  delay(3500);
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

  delay(3500);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Press button");
  lcd.setCursor(0, 1);
  lcd.print("after taking.");

  waitingForMedConfirmation = true;
}

void handleMedicationCompletion() {
  rewardCurrentMedication();
  showStoryAfterMedication();

  waitingForMedConfirmation = false;

  if (currentMed == totalMeds - 1) {
    showCompletionMessage();
    sequenceDone = true;
  } else {
    currentMed++;
    showPlotIntro(currentMed);
    showCurrentMedication();
  }
}

void rewardCurrentMedication() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Good job!");
  lcd.setCursor(0, 1);
  lcd.print(medNames[currentMed]);
  lcd.print(" done");

  int pin = ledPins[currentMed];
  unsigned long start = millis();
  while (millis() - start < 5000) {
    digitalWrite(pin, HIGH);
    delay(250);
    digitalWrite(pin, LOW);
    delay(250);
  }
}

void showStoryAfterMedication() {
  if (currentMed == 0) {
    showMessage("Oh no! Now Alice", "is too short.");
    delay(3500);
    showMessage("She can't reach", "the key.");
    delay(3500);
  } else if (currentMed == 1) {
    showMessage("Uh oh.", "");
    delay(3500);
    showMessage("Now Alice is", "too tall!");
    delay(3500);
  } else if (currentMed == 2) {
    showMessage("You got the key!", "");
    delay(3500);
  }
}

void showCompletionMessage() {
  lcd.setRGB(0, 255, 255);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("You unlocked the");
  lcd.setCursor(0, 1);
  lcd.print("garden door!");
  delay(3500);
  showMessage("Done for now.", "Stay healthy!");
  delay(3500);

  for (int i = 0; i < totalMeds; i++) {
    digitalWrite(ledPins[i], LOW);
  }
}

void resetToFirstMedication() {
  currentMed = 0;
  sequenceDone = false;
  hasStarted = false;
  while (digitalRead(buttonPin) == LOW);
  waitForStartButton();
}

void setBacklightColor(int r, int g, int b) {
  lcd.setRGB(r, g, b);
}

void showMessage(const char* line1, const char* line2) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(line1);
  lcd.setCursor(0, 1);
  lcd.print(line2);
}
