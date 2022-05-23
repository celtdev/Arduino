#include <Arduino.h>

#define ENABLE_GxEPD2_GFX 0

#include <GxEPD2_BW.h>
#include <U8g2_for_Adafruit_GFX.h>
#include <GyverWDT.h>
#include <avr/sleep.h>
#include <microDS18B20.h>

#define MAX_DISPLAY_BUFFER_SIZE 800
#define MAX_HEIGHT(EPD) (EPD::HEIGHT <= MAX_DISPLAY_BUFFER_SIZE / (EPD::WIDTH / 8) ? EPD::HEIGHT : MAX_DISPLAY_BUFFER_SIZE / (EPD::WIDTH / 8))
GxEPD2_BW<GxEPD2_290, MAX_HEIGHT(GxEPD2_290)> display(GxEPD2_290(/*CS=*/ SS, /*DC=*/ 8, /*RST=*/ 9, /*BUSY=*/ 7));
U8G2_FOR_ADAFRUIT_GFX u8g2Fonts;
MicroDS18B20<2> sensor;
double temperatureValue = 99.9;

class PrintString : public Print, public String
{
  public:
    size_t write(uint8_t data) override
    {
      return concat(char(data));
    };
};

const uint16_t margin = 10;

void updateDisplay()
{
  PrintString valueString;
  valueString.print(temperatureValue, 1);
  
  int16_t tw = u8g2Fonts.getUTF8Width(valueString.c_str());
  int16_t x = (display.width() - tw) / 2;
  int16_t y = 110;

  display.setPartialWindow(margin, margin, display.width() - 2*margin, display.height() - 2*margin);
  display.firstPage();

  do{
    display.fillScreen(GxEPD_WHITE);
    u8g2Fonts.setCursor(x, y);
    u8g2Fonts.print(valueString);
  }while(display.nextPage());
}

void getTemp()
{
  if (sensor.readTemp())
  {
    temperatureValue = sensor.getTemp();
  }
  else
  {
    temperatureValue = -99.9;
  }
}

void setup()
{
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);

  Serial.begin(9600);
  display.init();
  display.setTextColor(GxEPD_BLACK);
  display.firstPage();
  display.setRotation(1);

  u8g2Fonts.begin(display);
  delay(1000);

  uint16_t bg = GxEPD_WHITE;
  uint16_t fg = GxEPD_BLACK;
  u8g2Fonts.setForegroundColor(fg);
  u8g2Fonts.setBackgroundColor(bg);

  do{
    display.fillScreen(GxEPD_WHITE);
    u8g2Fonts.setFont(u8g2_font_logisoso92_tn);
  }while (display.nextPage());
}

void loop()
{
  updateDisplay();
  sensor.requestTemp();

  Watchdog.enable(ISR_MODE, WDT_TIMEOUT_8S);
  display.powerOff();
  sleep_enable();
  sleep_cpu();
}

ISR(WATCHDOG) {
  sleep_disable();
  Watchdog.disable();
  getTemp();
}