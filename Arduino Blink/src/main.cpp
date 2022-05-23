#define CLK 3       // Экран
#define DIO 2       // Экран
#define RELEY_PIN 4 // Реле нагрева
#define SENS_PIN A0 // Датчик температуры
#define ENC_A 7
#define ENC_B 6
#define ENC_KEY 5

// Гистерезис
#define REL_HSTR 3
#define RELAY_ON 0
#define RELAY_OFF 1

// ========= ДАТЧИК ========
#define T_RESIST 92000 // сопротивление термистора при 25 градусах
#define R_RESIST 98000 // сопротивление резистора
#define B_COEF 4200    // Base коэффициент

#include <Arduino.h>
#include <GyverTM1637.h>
#include <GyverNTC.h>
#include <EncButton.h>

EncButton<EB_TICK, ENC_A, ENC_B, ENC_KEY> enc;
GyverTM1637 disp(CLK, DIO);
GyverNTC ntc(SENS_PIN, T_RESIST, B_COEF, 25, R_RESIST);

// Главные переменные
bool isSettingsMode;
uint8_t targetTemp = 50;
uint8_t currentTemp = 99;
uint8_t maxTemp = 85;
uint8_t minTemp = 0;

bool relayState = RELAY_ON;

uint8_t degreeSign = 0x63;
uint8_t digits[10] =
    {
        0x3f, // 0
        0x06, // 1
        0x5b, // 2
        0x4f, // 3
        0x66, // 4
        0x6d, // 5
        0x7d, // 6
        0x07, // 7
        0x7f, // 8
        0x6f, // 9
};

void updateDisplay()
{
  uint8_t toDisplay = currentTemp;
  uint8_t dSign = degreeSign;

  if (isSettingsMode)
  {
    toDisplay = targetTemp;
    dSign = 0;
  }
  disp.displayByte(3, dSign);

  uint8_t low = toDisplay % 10;
  uint8_t high = toDisplay / 10;
  disp.displayByte(0, digits[high]);
  disp.displayByte(1, digits[low]);
}

void controlTick()
{
  static uint32_t lastEncTick;

  if (isSettingsMode && (millis() - lastEncTick >= 5000))
  {
    isSettingsMode = false;
  }

  enc.tick(); // опрос энкодера
  if (enc.isTurn())
  { // если был поворот (любой)
    lastEncTick = millis();
    isSettingsMode = true;
    // при повороте меняем значения согласно режиму вывода
    if (enc.isRight())
    { // вправо
      targetTemp++;
      if (targetTemp > maxTemp)
      {
        targetTemp = maxTemp;
      }
    }
    if (enc.isLeft())
    { // влево
      if (targetTemp <= minTemp)
      {
        targetTemp = minTemp;
      }
      else
      {
        targetTemp--;
      }
    }
    updateDisplay(); // обновляем дисплей в любом случае
  }

  if (enc.isClick())
  { // клик по кнопке
    lastEncTick = millis();
    isSettingsMode = !isSettingsMode;
  }
}

void sensorTick()
{

  static uint32_t tmrSens;
  if (millis() - tmrSens >= 1000)
  {
    tmrSens = millis();
    currentTemp = round(ntc.getTempAverage());

    if (currentTemp < targetTemp - REL_HSTR)
    {
      relayState = LOW;
    }
    else if (currentTemp >= targetTemp)
    {
      relayState = HIGH;
    }

    digitalWrite(RELEY_PIN, relayState);
  }
}

void timerTick()
{
  static uint32_t dotTimer;
  if (millis() - dotTimer >= 500)
  {
    dotTimer = millis();
    static bool dotsFlag = 0;
    dotsFlag = !dotsFlag;

    if (isSettingsMode)
    {
      dotsFlag = 0;
    }

    disp.point(dotsFlag, false);
    updateDisplay();
  }
}

void setup()
{
  // Serial.begin(9600);

  pinMode(RELEY_PIN, OUTPUT);
  pinMode(SENS_PIN, INPUT);

  disp.clear();
  disp.brightness(2); // яркость, 0 - 7 (минимум - максимум)
  disp.clear();
  disp.displayByte(2, 0);
}

void loop()
{
  controlTick();
  sensorTick();
  timerTick();
}