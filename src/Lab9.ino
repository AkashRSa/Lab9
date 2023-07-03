#include "env.h"
#include "oled-wing-adafruit.h"
#include "blynk.h"
#include "SparkFun_VCNL4040_Arduino_Library.h"

#define VCNL 0x60
#define BLUE_LED D7
#define YELLOW_LED D5
#define GREEN_LED D6
#define Potentiometer A0
#define BUTTON D4
#define TMP36 A1

SYSTEM_THREAD(ENABLED);

OledWingAdafruit display;
VCNL4040 proximitySensor;

bool buttonPressed = false;
bool aPushSent = false;
bool bPushSent = false;
bool cPushSent = false;
int lightValue = 0;
int lightLevel = 0;
bool firstSetPointDone = false;
bool secondSetPointDone = false;
int firstSetPointNum;
int secondSetPointNum;
int lightNum = 1;

void setup()
{
  Blynk.begin(BLYNK_AUTH_TOKEN);
  display.setup();
  Wire.begin();
  Serial.begin(9600);
  if (!Serial.isConnected())
  {
  }
  Serial.println(":(");
  display.clearDisplay();
  betterDisplaySetup();
  display.println("Set Point Mode");
  display.display();
  proximitySensor.begin();
  proximitySensor.powerOnAmbient();
  pinMode(BLUE_LED, OUTPUT);
  pinMode(YELLOW_LED, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);
  pinMode(TMP36, INPUT);

  digitalWrite(BLUE_LED, LOW);
  digitalWrite(YELLOW_LED, LOW);
  digitalWrite(GREEN_LED, LOW);
}

void loop()
{
  Blynk.run();
  display.loop();
  uint16_t lightValue = proximitySensor.getAmbient();
  Serial.println(lightValue);
  int potentiometerNumber = analogRead(Potentiometer);
  bool buttonRead = digitalRead(BUTTON);
  uint64_t reading = analogRead(TMP36);
  double voltage = (reading * 3.3) / 4095.0;
  double temperatureC = (voltage - 0.5) * 100;
  double temperatureF = (temperatureC * 9 / 5) + 32;
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  if (!firstSetPointDone)
  {
    display.println("First input: " + String(map(potentiometerNumber, 0, 4096, 0, 65535)));
    if (buttonRead)
    {
      firstSetPointNum = map(potentiometerNumber, 0, 4096, 0, 65535);
      firstSetPointDone = true;
    }
    betterDelay(200);
  }
  else if (!secondSetPointDone)
  {
    display.println("Second input: " + String(map(potentiometerNumber, 0, 4096, 0, 65535)));
    if (buttonRead)
    {
      secondSetPointNum = map(potentiometerNumber, 0, 4096, 0, 65535);
      secondSetPointDone = true;
    }
    betterDelay(200);
  }
  else
  {
    if (lightValue < firstSetPointNum)
    {
      digitalWrite(GREEN_LED, LOW);
      digitalWrite(YELLOW_LED, LOW);
      digitalWrite(BLUE_LED, HIGH);
      if (lightNum != 1) {
        Blynk.logEvent("notifyphone", "Light level is under " + String(firstSetPointNum));
        lightNum = 1;
      }
    }
    if (lightValue >= firstSetPointNum && lightValue < secondSetPointNum)
    {
      digitalWrite(GREEN_LED, HIGH);
      digitalWrite(YELLOW_LED, LOW);
      digitalWrite(BLUE_LED, LOW);
      if (lightNum != 2) {
        Blynk.logEvent("notifyphone", "Light level is between " + String(firstSetPointNum) + " and " + String(secondSetPointNum));
        lightNum = 2;
      }
    }
    if (lightValue >= secondSetPointNum)
    {
      digitalWrite(GREEN_LED, LOW);
      digitalWrite(YELLOW_LED, HIGH);
      digitalWrite(BLUE_LED, LOW);
      if (lightNum != 3) {
        Blynk.logEvent("notifyphone", "Light level is above " + String(secondSetPointNum));
        lightNum = 3;
      }
    }
    if (display.pressedA())
    {
      aPushSent = !aPushSent;
    }
    if (aPushSent)
    {
      display.clearDisplay();
      display.setCursor(0, 0);
      display.println("Light is: " + String(lightValue));
    }
    if (display.pressedB())
    {
      bPushSent = !bPushSent;
    }
    if (bPushSent)
    {
      display.clearDisplay();
      display.setCursor(0, 0);
      display.println("Temp in C: " + String(temperatureC, 2));
      display.println("Temp in F: " + String(temperatureF, 2));
    }
  }
  Blynk.virtualWrite(V2, lightValue);
  Blynk.virtualWrite(V3, temperatureC);
  Blynk.virtualWrite(V4, temperatureF);
  display.display();
}

BLYNK_WRITE(V5)
{
  display.println("Button Pressed!");;
}

void lightChangeLEDs()
{
  if (lightValue < firstSetPointNum)
  {
    digitalWrite(GREEN_LED, LOW);
    digitalWrite(YELLOW_LED, LOW);
    digitalWrite(BLUE_LED, HIGH);
  }
  else if (lightValue >= firstSetPointNum && lightValue < secondSetPointNum)
  {
    digitalWrite(GREEN_LED, HIGH);
    digitalWrite(YELLOW_LED, LOW);
    digitalWrite(BLUE_LED, LOW);
  }
  else if (lightValue >= secondSetPointNum)
  {
    digitalWrite(GREEN_LED, LOW);
    digitalWrite(YELLOW_LED, HIGH); // supposed to be red but I didnt have a red LED
    digitalWrite(BLUE_LED, LOW);
  }
}

void betterDisplaySetup()
{
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.display();
}

void betterDelay(int time)
{
  int timePerSegment = 100;
  for (int passedTime = 0; passedTime < time + 1; passedTime += timePerSegment)
  {
    display.loop();
    Blynk.run();
    delay(timePerSegment);
  }
}