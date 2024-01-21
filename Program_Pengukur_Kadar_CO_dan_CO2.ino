#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include <Wire.h>
#include <RTClib.h>
#include <MQ135.h>
#include <SPI.h>
#include <SD.h>


const int chipSelect = 10;

#define TFT_DC 4
#define TFT_CS 7
#define TFT_RST 3
#define TFT_MISO 6
#define TFT_MOSI 5
#define TFT_CLK 2

Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_MOSI, TFT_CLK, TFT_RST, TFT_MISO);

int mq135_pin = A0;
float maxCO = 0;
float minCO = 200;
float maxCO2 = 0;
float minCO2 = 200;
float volt;
char charMinCO[10];
char charMaxCO[10];
char charMinCO2[10];
char charMaxCO2[10];
char timeChar[100];
char dateChar[50];
char COChar[10];
char CO2Char[10];

float CO = 0;
float previousCO = 0;
float CO2 = 0;
float previousCO2 = 0;

DateTime previousTime;

String dateString;
int minuteNow = 0;
int minutePrevious = 0;

RTC_DS3231 rtc;
File dataFile;

void setup() {

  Serial.begin(9600);
  tft.begin();
  tft.setRotation(0);
  tft.fillScreen(ILI9341_BLACK);

  pinMode(chipSelect, OUTPUT);
  if (!SD.begin(chipSelect)) {
    Serial.println("Gagal membuka SD Card!");
    while (1)
      ;
  }
  Serial.println("SD Card berhasil diakses!");

  Wire.begin();
  if (!rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1)
      ;
  }
  if (rtc.lostPower()) {
    Serial.println("RTC lost power, setting time to default");
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }

  printText("GAS SENSOR", ILI9341_GREEN, 20, 20, 3);
  printText("KADAR CO DAN CO2", ILI9341_WHITE, 10, 60, 2);

  dataFile = SD.open("datalog.csv", FILE_WRITE);
  dataFile.println("Date,Time,CO2 (ppm),CO (ppm)");
  dataFile.close();
}

void loop() {


  int sensorValue = analogRead(mq135_pin);
  volt = sensorValue * (5.0 / 1023);
  CO = abs((87.08 * volt) - 120);
  CO2 = (224.89 * volt) + 90;
  
  if (CO != previousCO) {
    previousCO = CO;
    dtostrf(CO, 5, 1, COChar);
    tft.fillRect(60, 95, 240, 50, ILI9341_BLACK);
    printText("CO", ILI9341_WHITE, 0, 110, 2);
    printText(COChar, ILI9341_WHITE, 60, 95, 3);
    printText("ppm", ILI9341_WHITE, 210, 110, 1);
  }

  if (CO2 != previousCO2) {
    previousCO2 = CO2;
    dtostrf(CO2, 5, 1, CO2Char);
    tft.fillRect(60, 135, 240, 50, ILI9341_BLACK);
    printText("CO2", ILI9341_WHITE, 0, 150, 2);
    printText(CO2Char, ILI9341_WHITE, 60, 135, 3);
    printText("ppm", ILI9341_WHITE, 210, 150, 1);
  }

  DateTime now = rtc.now();
  int hour = now.hour();
  int minute = now.minute();
  int second = now.second();

  if (minute != minutePrevious) {
    minutePrevious = minute;
    previousTime = now;
    printDateTime(now);
  }

  dataFile = SD.open("datalog.csv", FILE_WRITE);
  dataFile.print(now.day(), DEC);
  dataFile.print("/");
  dataFile.print(now.month(), DEC);
  dataFile.print("/");
  dataFile.print(now.year(), DEC);
  dataFile.print(",");
  dataFile.print(now.hour(), DEC);
  dataFile.print(":");
  dataFile.print(now.minute(), DEC);
  dataFile.print(",");
  dataFile.print(CO2);
  dataFile.print(",");
  dataFile.println(CO);
  dataFile.close();

  Serial.print(now.day(), DEC);
  Serial.print("/");
  Serial.print(now.month(), DEC);
  Serial.print("/");
  Serial.print(now.year(), DEC);
  Serial.print(" ");
  Serial.print(now.hour(), DEC);
  Serial.print(":");
  Serial.print(now.minute(), DEC);
  Serial.print(", CO2 (ppm): ");
  Serial.print(CO2);
  Serial.print(", CO (ppm): ");
  Serial.println(CO);
  delay(100);
}


void printDateTime(DateTime now) {
  sprintf(timeChar, "%02d:%02d", now.hour(), now.minute());
  sprintf(dateChar, "%04d/%02d/%02d", now.year(), now.month(), now.day());

  tft.fillRect(0, 270, 240, 30, ILI9341_BLACK);
  printText(timeChar, ILI9341_YELLOW, 90, 270, 2);
  tft.fillRect(0, 300, 240, 30, ILI9341_BLACK);
  printText(dateChar, ILI9341_YELLOW, 90, 300, 2);
}

void printText(char *text, uint16_t color, uint16_t x, uint16_t y, uint8_t size) {
  tft.setCursor(x, y);
  tft.setTextColor(color);
  tft.setTextSize(size);
  tft.print(text);
}