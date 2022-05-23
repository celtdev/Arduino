#include <Arduino.h>

#define ENABLE_GxEPD2_GFX 0

#include <GxEPD2_BW.h>
#include <U8g2_for_Adafruit_GFX.h>
#include <GyverBME280.h>
#include <microDS3231.h>

//#include <GyverWDT.h>
//#include <avr/sleep.h>

#define MAX_DISPLAY_BUFFER_SIZE 800
#define MAX_HEIGHT(EPD) (EPD::HEIGHT <= MAX_DISPLAY_BUFFER_SIZE / (EPD::WIDTH / 8) ? EPD::HEIGHT : MAX_DISPLAY_BUFFER_SIZE / (EPD::WIDTH / 8))
GxEPD2_BW<GxEPD2_290, MAX_HEIGHT(GxEPD2_290)> display(GxEPD2_290(/*CS=*/SS, /*DC=*/8, /*RST=*/9, /*BUSY=*/7));
U8G2_FOR_ADAFRUIT_GFX u8g2Fonts;
GyverBME280 bme;
MicroDS3231 rtc;

double curTempterature = 15.9;
double curHumidity = 99;
double curPressure = 777;

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
  PrintString temperatureStringValue;
  temperatureStringValue.print(curTempterature, 1);

  PrintString humidityStringValue;
  humidityStringValue.print(curHumidity, 1);

  PrintString pressureStringValue;
  pressureStringValue.print(curPressure, 1);

  // int16_t tw = u8g2Fonts.getUTF8Width(valueString.c_str());
  int16_t x = 0; //(display.width() - tw) / 2;
  int16_t y1 = 30;
  int16_t y2 = 60;
  int16_t y3 = 90;
  int16_t y4 = 120;

  // display.setPartialWindow(margin, margin, display.width() - 2*margin, display.height() - 2*margin);
  display.setPartialWindow(0, 0, 500, 500);
  display.firstPage();

  do
  {
    display.fillScreen(GxEPD_WHITE);
    u8g2Fonts.setCursor(x, y1);
    u8g2Fonts.print(temperatureStringValue);
    u8g2Fonts.setCursor(x, y2);
    u8g2Fonts.print(humidityStringValue);
    u8g2Fonts.setCursor(x, y3);
    u8g2Fonts.print(pressureStringValue);
    u8g2Fonts.setCursor(x, y4);
    u8g2Fonts.print(rtc.getDateString());
    u8g2Fonts.setCursor(x+150, y4);
    u8g2Fonts.print(rtc.getTimeString());
  } while (display.nextPage());
}

void getTemp()
{
  curTempterature = bme.readTemperature();
  curPressure = bme.readPressure();
  curHumidity = bme.readHumidity();
  Serial.println("Values updated");
}

void showError()
{
  Serial.println("0x76 Error!");
}

void setup()
{
  Serial.begin(9600);
  Serial.println("Start");

  // запуск датчика и проверка на работоспособность
  if (!bme.begin(0x76))
  {
    showError();  
  }

  if (!rtc.begin()) {
    Serial.println("DS3231 not found");
  }
  rtc.setTime(COMPILE_TIME);

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

  do
  {
    display.fillScreen(GxEPD_WHITE);
    // u8g2Fonts.setFont(u8g2_font_logisoso92_tn);
    u8g2Fonts.setFont(u8g2_font_logisoso20_tn);
  } while (display.nextPage());
}

void loop()
{
  getTemp();
  updateDisplay();
  delay(1000);

  // Watchdog.enable(ISR_MODE, WDT_TIMEOUT_1S);
  // display.powerOff();
  // sleep_enable();
  // sleep_cpu();
}

// ISR(WATCHDOG) {
//   sleep_disable();
//   Watchdog.disable();
//   getTemp();
// }