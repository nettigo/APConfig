/***************************************************************************
  Pogodełko - mini stacja pogodowa
  https://nettigo.pl/products/pogodelko-twoja-pierwsza-stacja-pogodowa-z-wifi
  v 1.0
***************************************************************************/

#include <Wire.h>
#include <SPI.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h>
//#include <Adafruit_BME280.h>


#define BMP_SCK 13
#define BMP_MISO 12
#define BMP_MOSI 11
#define BMP_CS D4

//Adafruit_BMP280 bme; // I2C
Adafruit_BMP280 bme(BMP_CS); // hardware SPI
//Adafruit_BME280 bme(BMP_CS); // hardware SPI
//Adafruit_BMP280 bme(BMP_CS, BMP_MOSI, BMP_MISO,  BMP_SCK);
#include <LiquidCrystal_PCF8574.h>





LiquidCrystal_PCF8574 lcd1(0x27);  // set the LCD address to 0x27 for a 16 chars and 2 line display
LiquidCrystal_PCF8574 lcd2(0x3F);  // set the LCD address to 0x27 for a 16 chars and 2 line display
LiquidCrystal_PCF8574 &lcd = lcd1;  // set the LCD address to 0x27 for a 16 chars and 2 line display

void setupPogo() {
  byte error;
  Wire.begin();
  Wire.beginTransmission(0x27);
  error = Wire.endTransmission();
  if ( error == 0) {
    lcd = lcd1;
    Serial.println("Jest LCD na adresie 0x27");

  } else {
    lcd = lcd2;
    Serial.println("Jest LCD na adresie 0x3F?");
  }
  Serial.println(F("BMP280 test"));
  delay(100);
  lcd.begin(16, 2); // initialize the lcd
  lcd.setBacklight(1);

  if (!bme.begin()) {
    Serial.println("Could not find a valid BMP280 sensor, check wiring!");
    delay(1);
    lcd.print("Brak BMP280");
    while (1) {
      delay(1000);
      Serial.println("TICK");
    };
  }

  Serial.println("Init done");
}

void pogoRun() {
  Serial.print("Temperature = ");
  Serial.print(bme.readTemperature());
  Serial.println(" *C");
  lcd.setCursor(0, 0);
  lcd.print(bme.readTemperature());
  lcd.print(" C     ");
  Serial.print("Pressure = ");
  Serial.print(bme.readPressure());
  Serial.println(" Pa");
  lcd.setCursor(0, 1);
  lcd.print(bme.readPressure() / 100);
  lcd.print(" hPa     ");
  Serial.print("Approx altitude = ");
  Serial.print(bme.readAltitude(1013.25)); // this should be adjusted to your local forcase
  Serial.println(" m");

  Serial.println();
  delay(2000);
}
