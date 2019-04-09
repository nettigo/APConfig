// Creates an admin page on a webserver which allows the user to update the SSID and Password
// Performs basic checks to ensure that the input values are valid

#ifndef AdminPage_h
#define AdminPage_h

//Holds the admin webpage in the program memory
const char adminPage[] PROGMEM =
  "<html>"
  "<head>"
  "<style>input {font-size: 1.2em; width: 100%; max-width: 350px; display: block; margin: 5px auto; } </style>"
  "</head>"
  "<body>"
  "<form id='form' action='/admin' method='post'>"
  "<input name='newssid' type='text' minlength='2' maxlength='16' placeholder='SSID'>"
  "<input name='newpassword' id='password1' type='text' minlength='8' maxlength='16' placeholder='Haslo'>"
  "<input name='opensense' type='text' minlength='1' maxlength='25' placeholder='SenseBox ID'>"
  "<input name='sensorID1' type='text' minlength='1' maxlength='25' placeholder='Sensor 1 ID'>"
  "<input name='sensorID2' type='text' minlength='1' maxlength='25' placeholder='Sensor 2 ID'>"
  "<input name='sensorID3' type='text' minlength='1' maxlength='25' placeholder='Sensor 3 ID'>"
  "<input type='submit' value='Update'>"
  "</form>"
  "</body>"
  "</html>";


//Creates a webpage that allows the user to change the SSID and Password from the browser
void serveAdmin(ESP8266WebServer *webServer) {
  String message;

  // Check to see if we've been sent any arguments and instantly return if not
  if (webServer->args() == 0) {
    webServer->sendHeader("Content-Length", String(strlen(adminPage)));
    webServer->send(200, "text/html", adminPage);
  }
  else {
    // Create a string containing all the arguments, send them out to the serial port
    // Check to see if there are new values (also doubles to check the length of the new value is long enough)
    Serial.println(webServer->arg("newssid"));

    if ((webServer->arg("newssid").length() >= MIN_STR_LEN) &&
        (webServer->arg("newssid").length() < MAX_STR_LEN))
      webServer->arg("newssid").toCharArray(config.ssid, sizeof(config.ssid));

    Serial.println("sensorID1 ID");
    if ((webServer->arg("opensense").length() >= MIN_STR_LEN) &&
        (webServer->arg("opensense").length() < MAX_STR_LEN))
      webServer->arg("opensense").toCharArray(config.opensense, sizeof(config.opensense));

    if ((webServer->arg("sensorID1").length() >= MIN_STR_LEN) &&
        (webServer->arg("sensorID1").length() < MAX_STR_LEN))
      webServer->arg("sensorID1").toCharArray(config.sensorID1, sizeof(config.sensorID1));

    if ((webServer->arg("sensorID2").length() >= MIN_STR_LEN) &&
        (webServer->arg("sensorID2").length() < MAX_STR_LEN))
      webServer->arg("sensorID2").toCharArray(config.sensorID2, sizeof(config.sensorID2));

    if ((webServer->arg("sensorID3").length() >= MIN_STR_LEN) &&
        (webServer->arg("sensorID3").length() < MAX_STR_LEN))
      webServer->arg("sensorID3").toCharArray(config.sensorID3, sizeof(config.sensorID3));

    if (webServer->arg("newpassword").length() < MAX_STR_LEN)
      webServer->arg("newpassword").toCharArray(config.pass, sizeof(config.pass));
    webServer->arg("newpassword").toCharArray(config.pass, sizeof(config.pass));

    config.config_mode = false;
    // Store the new settings to EEPROM
    SaveConfig();
    PrintConfig();

    // Construct a message to tell the user that the change worked
    message = "New settings will take effect after restart";

    // Reply with a web page to indicate success or failure
    message = "<html><head><meta http-equiv='refresh' content='5;url=/' /></head><body>" + message;
    message += "<br/>Redirecting in 5 seconds...</body></html>";
    webServer->sendHeader("Content-Length", String(message.length()));
    webServer->send(200, "text/html", message);
  }
}
#endif


