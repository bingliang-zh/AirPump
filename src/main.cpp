#include <U8g2lib.h>
#include <Wire.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

U8G2_SH1106_128X64_NONAME_F_HW_I2C display(U8G2_R0, /* reset=*/U8X8_PIN_NONE);

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

  display.begin();
  display.clearBuffer();
  display.sendBuffer();
  delay(2000);
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

  int LINE_HEIGHT = 14;
  int GAP = 6;
  display.clearBuffer();
  display.setFont(u8g2_font_12x6LED_tr); // Choose a suitable font
  display.setCursor(0, 1 * LINE_HEIGHT);
  display.print("Sen: ");
  display.print(sensorValue * 10, 1); // Bar
  display.print(" Bar/");

  display.print(sensorValue * 145.038, 0); // PSI
  display.println(" PSI");

  display.setCursor(0, 2 * LINE_HEIGHT);
  display.print("Tar: ");
  display.print(targetPressure * 10, 1); // Bar
  display.print(" Bar/");
  display.print(targetPressure * 145.038, 0); // PSI
  display.println(" PSI");

  display.setCursor(0, 3 * LINE_HEIGHT + GAP);
  display.print(persistMode ? "Persist Mode" : "");

  display.setCursor(0, 4 * LINE_HEIGHT + GAP);
  display.print(isPumping     ? "Pumping"
                : pumpingMode ? "Waiting"
                              : "Press to start");

  display.sendBuffer();
  handleButtonPress();
  controlPump();

  delay(100);
}
