/***************************************************************************
  Pogodełko - mini stacja pogodowa
  https://nettigo.pl/products/pogodelko-twoja-pierwsza-stacja-pogodowa-z-wifi
  v 1.0
***************************************************************************/

#include <Wire.h>
#include <SPI.h>
#include <ESP8266HTTPClient.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h>
#include <Adafruit_BME280.h>
#include <time.h>



#define BMP_SCK 13
#define BMP_MISO 12
#define BMP_MOSI 11
#define BMP_CS D4

//Adafruit_BMP280 bme; // I2C
#ifndef BME
Adafruit_BMP280 bme(BMP_CS); // hardware SPI
#else
Adafruit_BME280 bme(BMP_CS); // hardware SPI
#endif
//Adafruit_BMP280 bme(BMP_CS, BMP_MOSI, BMP_MISO,  BMP_SCK);
#include <LiquidCrystal_PCF8574.h>


char *server = "ingress.opensensemap.org";


LiquidCrystal_PCF8574 lcd1(0x27);  // set the LCD address to 0x27 for a 16 chars and 2 line display
LiquidCrystal_PCF8574 lcd2(0x3F);  // set the LCD address to 0x27 for a 16 chars and 2 line display
LiquidCrystal_PCF8574 &lcd = lcd1;  // set the LCD address to 0x27 for a 16 chars and 2 line display

// Set time via NTP, as required for x.509 validation
void setClock() {
  configTime(3 * 3600, 0, "pool.ntp.org", "time.nist.gov");

  Serial.print("Waiting for NTP time sync: ");
  time_t now = time(nullptr);
  while (now < 8 * 3600 * 2) {
    delay(500);
    Serial.print(".");
    now = time(nullptr);
  }
  Serial.println("");
  struct tm timeinfo;
  gmtime_r(&now, &timeinfo);
  Serial.print("Current time: ");
  Serial.print(asctime(&timeinfo));
}

void initLCD() {
  byte error;

  Wire.beginTransmission(0x27);
  error = Wire.endTransmission();
  if ( error == 0) {
    lcd = lcd1;
    Serial.println("Jest LCD na adresie 0x27");

  } else {
    lcd = lcd2;
    Serial.println("Jest LCD na adresie 0x3F?");
  }
  lcd.begin(16, 2); // initialize the lcd
  lcd.setBacklight(1);
  
}

void setupPogo() {
  setClock();
  Serial.println(F("BMP280 test"));
  delay(100);
  lcd.clear();
  if (!bme.begin()) {
    Serial.println(F("Could not find a valid BMP/E280 sensor, check wiring!"));
    delay(1);
    lcd.print("Brak BMP/E280");
    while (1) {
      delay(1000);
      Serial.println("TICK");
    };
  }

  Serial.println("Init done");
}

WiFiClientSecure client;

void waitForServerResponse () {
  //Ankommende Bytes ausgeben
  boolean repeat = true;
  do {
    yield();
    if (client.available()) {
      char c = client.read();
      Serial.print(c);
    }
    //Verbindung beenden
    if (!client.connected()) {
      Serial.println();
      Serial.println("--------------");
      Serial.println("Disconnecting.");
      Serial.println("--------------");
      client.stop();
      repeat = false;
    }
  } while (repeat);
}


void postFloatValue (float measurement, int digits, String sensorId) {
  //Float zu String konvertieren
  char obs[10];
  Serial.println("postFloatValue");
  Serial.println(sensorId);
  if (sensorId.length() < 10) {
    Serial.println("SensorID too short?");
    return;
  }
  dtostrf(measurement, 5, digits, obs);
  //Json erstellen
  String jsonValue = " {\"value\":";
  jsonValue += obs;
  jsonValue += "}";
  client.setInsecure();

  //Mit OSeM Server verbinden und POST Operation durchführen
  Serial.println("-------------------------------------");
  Serial.print("Connectingto OSeM Server...");
  String sendData("");
  if (client.connect(server, 443)) {
    Serial.println("connected!");
    Serial.println("-------------------------------------");
    //HTTP Header aufbauen
    sendData.concat("POST /boxes/"); sendData.concat(config.opensense); sendData.concat("/"); sendData.concat(sensorId); sendData.concat(" HTTP/1.1"); sendData.concat("\n");
    sendData.concat("Host:");
    sendData.concat(server); sendData.concat("\n");
    sendData.concat("Content-Type: application/json"); sendData.concat("\n");
    sendData.concat("Connection: close"); sendData.concat("\n");
    sendData.concat("Content-Length: "); sendData.concat(jsonValue.length()); sendData.concat("\n");
    sendData.concat("\n");
    //Daten senden
    sendData.concat(jsonValue);
    sendData.concat("\n");
#ifdef DEBUG
    Serial.print(sendData);
#endif
    client.print(sendData);
  } else {
    Serial.println("failed!");
    Serial.println("-------------------------------------");
  }
  //Antwort von Server im seriellen Monitor anzeigen
  waitForServerResponse();
}


void displayIP(void) {
  lcd.clear();
  lcd.print("SSID:");
  lcd.print(config.ssid);
  lcd.setCursor(0, 1);
  lcd.print(WiFi.localIP());
  delay(3000);
  lcd.clear();
}


unsigned long lastSent = 0;
unsigned long senseInterval = 120 * 1000;

void pogoRun() {
  Serial.print("Temperature = ");
  Serial.print(bme.readTemperature());
  Serial.println(" *C");
  lcd.setCursor(0, 0);
  lcd.print(bme.readTemperature());
  lcd.print(" C     ");
#ifdef BME
  lcd.setCursor(8, 0);
  lcd.print(bme.readHumidity());
  lcd.print(" %    ");
  Serial.printf("Humidity: %.2f %%\n", bme.readHumidity());

#endif
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
  if (strlen(config.opensense ) > 10 && millis() - lastSent > senseInterval ) {
    postFloatValue (bme.readTemperature(), 2, config.sensorID1) ;
    postFloatValue (bme.readPressure(), 2, config.sensorID2) ;
#ifdef BME
    postFloatValue (bme.readHumidity(), 2, config.sensorID3) ;
#endif
    lastSent = millis();
  }
  delay(2000);
}
