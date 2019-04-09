#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

#define USESERIAL
#define USELED
//#define BME
#define DEBUG

/* Historia wersji
    2019-1 - pierwsza wersja
    2019-2 - sieć WiFi APConfig pojawia się tylko gdy urządzenie jest konfigurowane
*/

#ifdef BME
#define SW_VER  "BME2019-2"
#else
#define SW_VER  "BMP2019-2"
#endif

#include "Config.h"
#include "FirmwareReset.h"
#include "AdminPage.h"
#include "pogodelko.h"



ESP8266WebServer webServer(80);

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.printf("Start. Wersja oprogramowania: %s\n", SW_VER);

  Wire.begin();
  initLCD();
  lcd.clear();
  lcd.print("Start....");
  lcd.setCursor(0, 1);
  lcd.print("FW:");
  lcd.print(SW_VER);
  InitConfig();


  LoadConfig();
  PrintConfig();


  //Check to see if the flag is still set from the previous boot
  if (checkResetFlag()) {
    //Do the firmware reset here
    Serial.printf("Reset Firmware\n");
    config.config_mode = 1;

    //Start the wifi with the required username and password
    WiFi.mode(WIFI_AP);

    //Set the ssid to default value and turn off the password
    WiFi.softAP("APConfig", "", 1, false, 1);
  }
  else {
    if (config.config_mode) {
      //Start the wifi with the required username and password
      WiFi.mode(WIFI_AP);
      WiFi.softAP(config.ssid, config.pass, 1, false, 1);
    } else {
      //podlacz sie do sieci
      Serial.println("Łączenie z siecią");
      WiFi.mode(WIFI_STA);
      WiFi.begin(config.ssid, config.pass);
      while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
      }
      Serial.println("");
      Serial.println("WiFi connected");
      Serial.println("IP address: ");
      Serial.println(WiFi.localIP());
    }
  }

  if (config.config_mode) { //jeśli nie jest skonfigurowany to startuj admina
    //Serve a very basic page on the root url
    webServer.on("/", []() {
      webServer.send(200, "text/plain", "Hello World");
    });

    // Set up the admin page
    webServer.on("/admin", std::bind(serveAdmin, &webServer));
    webServer.begin();
    while (true) {
      webServer.handleClient();
      yield();
    }
  } else {
    setupPogo();
    displayIP();
  }
}


void loop() {
  // put your main code here, to run repeatedly:
  pogoRun();

}



