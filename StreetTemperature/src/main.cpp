#include <Arduino.h>

#define ENABLE_GxEPD2_GFX 0

#include <GxEPD2_BW.h>
#include <U8g2_for_Adafruit_GFX.h>
#include <GyverBME280.h>
#include <PrintString.cpp>
#include <RTClib.h>
#include <GyverPower.h>

#define MAX_DISPLAY_BUFFER_SIZE 800
#define MAX_HEIGHT(EPD) (EPD::HEIGHT <= MAX_DISPLAY_BUFFER_SIZE / (EPD::WIDTH / 8) ? EPD::HEIGHT : MAX_DISPLAY_BUFFER_SIZE / (EPD::WIDTH / 8))
GxEPD2_BW<GxEPD2_290, MAX_HEIGHT(GxEPD2_290)> display(GxEPD2_290(/*CS=*/SS, /*DC=*/8, /*RST=*/9, /*BUSY=*/7));
U8G2_FOR_ADAFRUIT_GFX u8g2Fonts;
GyverBME280 bme;
RTC_DS3231 rtc;

float curTempterature = -30.9;
float curHumidity = 99;
double curPressure = 777;
char digitalBuf[3];

void showString(String str, int16_t x, int16_t y, int16_t w, int16_t h, const uint8_t *font)
{
  u8g2Fonts.setFont(font);

  int16_t tw = u8g2Fonts.getUTF8Width(str.c_str());
  int16_t sx = (w - tw) / 2 + x;

  int16_t th = u8g2Fonts.getFontAscent();
  int16_t sy = (h - th) / 2 - y;

  u8g2Fonts.setCursor(sx, h - sy);
  display.drawRect(x, y, w, h, GxEPD_BLACK);
  u8g2Fonts.print(str);
}

void showValue(float value, int16_t x, int16_t y, int16_t w, int16_t h, const uint8_t *font, int precision = 1)
{
  PrintString strValue;
  strValue.print(value, precision);
  showString(strValue, x, y, w, h, font);
}

void showDateTime(int16_t x, int16_t y, int16_t w, int16_t h, const uint8_t *font)
{
  DateTime dt = rtc.now();
  PrintString str;
  
  sprintf(digitalBuf, "%02d", dt.day());
  str.print(digitalBuf);
  
  str.print('.');
  
  sprintf(digitalBuf, "%02d", dt.month());
  str.print(digitalBuf);
  
  str.print('.');
  
  str.print(dt.year());

  showString(str, x, y, w, h, font);
}

void updateDisplay()
{
  // maxWidth = 296
  // maxHeight = 128

  display.setPartialWindow(0, 0, display.width(), display.height());
  display.firstPage();

  do
  {
    display.fillScreen(GxEPD_WHITE);

    // Temperature
    showValue(curTempterature, 1, 1, 232, 96, u8g2_font_logisoso78_tn);

    // Pressure
    // showValue(curHumidity, 232, 0, 64, 64);
    showValue(curPressure, 232, 65, 64, 32, u8g2_font_logisoso20_tn, 0);

    // DateTime
    showDateTime(1, 96, 232, 32, u8g2_font_logisoso20_tn);

    // Humidity
    showValue(curHumidity, 232, 96, 64, 32, u8g2_font_logisoso20_tn);

    if (rtc.lostPower())
    {
      display.drawLine(8, 102, 16, 110, GxEPD_BLACK);
      display.drawLine(8, 110, 16, 102, GxEPD_BLACK);
    }
  } while (display.nextPage());
}

void getTemp()
{
  curTempterature = bme.readTemperature();
  curPressure = bme.readPressure() / 133.3;
  curHumidity = bme.readHumidity();
}

void showError()
{
  Serial.println("0x76 Error!");
}

void setup()
{
  Serial.begin(9600);
  Serial.println("Start");

  power.hardwareDisable(PWR_ADC | PWR_TIMER1);
  power.setSystemPrescaler(PRESCALER_2);
  power.setSleepMode(STANDBY_SLEEP);
  power.bodInSleep(false);

  // запуск датчика и проверка на работоспособность
  if (!bme.begin(0x76))
  {
    showError();
  }

  if (!rtc.begin())
  {
    Serial.println("DS3231 not found");
  }

  //rtc.adjust(DateTime(__DATE__, __TIME__));

  display.init();
  display.firstPage();
  display.setRotation(1);
  u8g2Fonts.begin(display);

  uint16_t bg = GxEPD_WHITE;
  uint16_t fg = GxEPD_BLACK;
  u8g2Fonts.setForegroundColor(fg);
  u8g2Fonts.setBackgroundColor(bg);

  do
  {
    display.fillScreen(GxEPD_WHITE);
    display.drawRect(0, 0, 296, 128, GxEPD_BLACK);
    display.drawLine(232, 0, 232, 128, GxEPD_BLACK);
    display.drawLine(0, 96, 296, 96, GxEPD_BLACK);
    display.drawLine(232, 64, 296, 64, GxEPD_BLACK);
  } while (display.nextPage());
}

void loop()
{
  getTemp();
  updateDisplay();
  
  power.sleepDelay(60000);
}