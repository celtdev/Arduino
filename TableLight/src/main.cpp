#define CLK 3
#define DT 4
#define SW 2
#define PIN_MAIN_LIGHT 9
#define PIN_BACK_LIGHT 10

#include <Arduino.h>
#include <GyverPWM.h>
#include <GyverEncoder.h>
#include <EEManager.h>

const int MAX_BRIGHT = 250;
const int DELTA = 10;
const int OVERLAP = 50;

struct BrightConfiguration
{
  uint8_t main;
  uint8_t back;
};

bool isMaxBright = false;
Encoder enc(CLK, DT, SW);
BrightConfiguration brightCfg;
EEManager memory(brightCfg);

void setup()
{
  Serial.begin(9600);

  pinMode(PIN_MAIN_LIGHT, OUTPUT);
  pinMode(PIN_BACK_LIGHT, OUTPUT);
  PWM_resolution(PIN_MAIN_LIGHT, 8, FAST_PWM);
  PWM_resolution(PIN_BACK_LIGHT, 8, FAST_PWM);

  memory.begin(0, 'b');
  enc.setType(TYPE2);

  PWM_set(PIN_MAIN_LIGHT, brightCfg.main);
  PWM_set(PIN_BACK_LIGHT, brightCfg.back);
}

void setNewBright(uint8_t port, uint16_t duty)
{
  if (duty == 0)
    PWM_detach(port);
  else
    PWM_set(port, duty);
}

void loop()
{
  enc.tick();
  if (memory.tick())
    Serial.println("Updated!");

  int main = brightCfg.main;
  int back = brightCfg.back;
  bool isChanged = false;

  // Calculate new values
  if (enc.isRight())
  {
    isMaxBright = false;
    back += DELTA;
    if (back > (MAX_BRIGHT - OVERLAP))
    {
      main += DELTA;
    }
  }
  else if (enc.isLeft())
  {
    isMaxBright = false;
    main -= DELTA;
    if (main < OVERLAP)
    {
      back -= DELTA;
    }
  }
  else if (enc.isClick())
  {
    isMaxBright = !isMaxBright;
    isChanged = true;
  }

  // Check new values
  main = constrain(main, 0, MAX_BRIGHT);
  back = constrain(back, 0, MAX_BRIGHT);
  isChanged |= main != brightCfg.main;
  isChanged |= back != brightCfg.back;

  // Set PWM
  if (isChanged)
  {
    brightCfg.main = main;
    setNewBright(PIN_MAIN_LIGHT, isMaxBright ? MAX_BRIGHT : main);
    memory.update();
  }

  if (isChanged)
  {
    brightCfg.back = back;
    setNewBright(PIN_BACK_LIGHT, isMaxBright ? MAX_BRIGHT : back);
    memory.update();
  }
}
