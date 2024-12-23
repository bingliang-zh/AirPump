#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

const int sensorPin = A0;
const int potentiometerPin = A1;
// const int relayPin = 2;
const int relayPin = 13;
const int buttonPin = 3;

float sensorValue = 0;    // MPa
float targetPressure = 0; // MPa
bool pumpingMode = false;
bool persistMode = false;
bool isPumping = false;
unsigned long buttonPressTime = 0;
const unsigned long clickDuration = 1000;
const unsigned long longPressDuration = 1000;
const unsigned long actionTimeGap = 500;

void setup()
{
  pinMode(sensorPin, INPUT);
  pinMode(potentiometerPin, INPUT);
  pinMode(relayPin, OUTPUT);
  pinMode(buttonPin, INPUT_PULLUP);

  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.display();
  delay(2000);
  display.clearDisplay();
}

bool releaseLock = false;

void handleButtonPress()
{
  if (digitalRead(buttonPin) == LOW)
  {
    if (releaseLock)
    {
      return;
    }
    if (buttonPressTime == 0)
    {
      buttonPressTime = millis();
    }
    else if (millis() - buttonPressTime > longPressDuration)
    {
      persistMode = !persistMode;
      pumpingMode = false;
      releaseLock = true;
      buttonPressTime = 0;
    }
  }
  else
  {
    if (releaseLock)
    {
      releaseLock = false;
      // Delay to prevent multiple button press
      delay(200);
    }
    if (buttonPressTime != 0 && millis() - buttonPressTime < clickDuration)
    {
      pumpingMode = !pumpingMode;
    }
    buttonPressTime = 0;
  }
}

void controlPump()
{
  bool isPumpingFlag = false;
  if (pumpingMode)
  {
    if (!persistMode)
    {
      if (sensorValue >= targetPressure)
      {
        // Reached target pressure, exit pumping mode
        pumpingMode = false;
      }
      else
      {
        isPumpingFlag = true;
      }
    }
    else
    {
      if (isPumping)
      {
        if (sensorValue >= targetPressure)
        {
          isPumpingFlag = false;
        }
        else
        {
          isPumpingFlag = true;
        }
      }
      else
      {
        // if pressure drops 0.2 bars below target, start pumping
        if (sensorValue * 10 < targetPressure * 10 - 0.2)
        {
          isPumpingFlag = true;
        }
      }
    }
  }
  isPumping = isPumpingFlag;
  digitalWrite(relayPin, isPumpingFlag ? HIGH : LOW);
}

void loop()
{
  sensorValue = analogRead(sensorPin) * (1.6 / 1023.0);

  if (!pumpingMode)
  {
    targetPressure = analogRead(potentiometerPin) * (1.6 / 1023.0);
  }

  int TEXT_SIZE = 2;
  int GAP = 0;
  int CHAR_WIDTH = 6 * TEXT_SIZE;
  int LINE_HEIGHT = 8 * TEXT_SIZE;

  display.clearDisplay();
  display.setTextSize(TEXT_SIZE);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.print("S ");
  display.print(sensorValue * 10, 1); // Bar
  display.print(" Bar");

  display.setCursor(CHAR_WIDTH * 2, LINE_HEIGHT + GAP);
  display.print(sensorValue * 145.038, 0); // PSI
  display.println(" PSI");

  display.setCursor(0, 2 * (LINE_HEIGHT + GAP) + 2);
  display.print("T ");
  display.print(targetPressure * 10, 1); // Bar
  display.print(" Bar");
  display.setCursor(CHAR_WIDTH * 2, 3 * (LINE_HEIGHT + GAP) + 2);
  display.print(targetPressure * 145.038, 0); // PSI
  display.println(" PSI");

  if (persistMode)
  {
    display.setCursor(0, 3 * (LINE_HEIGHT + GAP) + 2);
    display.print("K");
  }
  if (isPumping)
  {
    display.setCursor(CHAR_WIDTH, 3 * (LINE_HEIGHT + GAP) + 2);
    display.print("P");
  }
  else if (pumpingMode)
  {
    display.setCursor(CHAR_WIDTH, 3 * (LINE_HEIGHT + GAP) + 2);
    display.print("p");
  }

  handleButtonPress();
  controlPump();

  display.display();
  delay(100);
}
