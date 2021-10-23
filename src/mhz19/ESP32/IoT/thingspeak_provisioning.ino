/*
  Thingspeak_params_provisioning.ino - Program for ESP32 and MH-Z19b 
     CO2 sensor. Connects to the IoT thingspeak platform.  
     This program uses the libraries from
       https://github.com/WifWaf/MH-Z19
       https://github.com/mathworks/thingspeak-arduino
     Both available in Arduino too.
       
     Warning level signal RGB LED
     Push Button for configuation control
       (1) If the button is held down for less than 
           one second, it sets the CO2 baseline, 
       (2) if the duration is longer than one second, 
           the device goes to calibration mode.
       (3) if the duration is longer than three seconds, 
           the device goes to Access Point mode for parameters provisioning.

     Params (WiFi, thinspeak) provisioning:
       (a) During the first device bootstrap, it is configured as an access 
           point (AP) and creates an open Wifi network without password with 
           the name "redmedidor". Then, to establish the connection to this 
           access point, in the device from which you want to connect to the 
           meter (phone, PC, etc.) you must select that network with the 
           name "redmedidor".
       (b) This open WiFI network allows the user to connect to the meter 
           through a browser. To do so, the address http://192.168.4.1 must 
           be entered in the browser to access the configuration web page.
       (c) After selecting the network name, password, and thingspeak 
           comunication parameters, the meter restarts and attempts to 
           connect to the assigned network. If the connection is established, 
           the provisioning process is successful.
       (d) In case of failure or if you want to change the network to which 
           the meter is connected, press the button for more than three seconds 
           to switch the device back to the access point mode.

  Copyright (c) 2021 Elías Todorovich, Matías Presso

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include "eco2.h"

int d = 0;
const int S = 3; // Samples per telemetry recording

/*----------------------------------------------------------
  WiFi connection in an ESP32s
  ----------------------------------------------------------*/
#include <WiFi.h>
#include <EEPROM.h>

String ssid;       // your network SSID (name)... stored in EEPROM
String password;   // your network password... stored in EEPROM

WiFiClient myClient;

/*----------------------------------------------------------
  Access Point mode in an ESP32s
  ----------------------------------------------------------*/
//#include <HTTPClient.h>
#include <WebServer.h>

WebServer server(80);  //Local server at port 80

int statusCode;
const char* AP_ssid = "redmedidor";
const char* AP_pass = "";
String st;
String content;
int nets;
bool AP_MODE = false;

/*----------------------------------------------------------
  ThingSpeak settings
  ----------------------------------------------------------*/
#include "ThingSpeak.h"

unsigned long myChannelNumber;
String myWriteAPIKey, myReadAPIKey;


/*----------------------------------------------------------
    Connect your device to the wireless network
  ----------------------------------------------------------*/
void connectWiFi() {
  while (WiFi.status() != WL_CONNECTED) {
    WiFi.begin(ssid.c_str(), password.c_str());
    Serial.print(".");
    updateRGB_LED (true);         // Blink while connecting
    delay(3000);
  }

  // Display a notification that the connection is successful.
  Serial.println("Connected");
}


/*----------------------------------------------------------
    Read meter parameters from EEPROM
  ----------------------------------------------------------*/
int readEEPROM(){
String myChannel;

  //  SSID
  Serial.println("Reading EEPROM ssid");

  ssid = "";
  for (int i = 0; i < 32; ++i)
  {
    ssid += char(EEPROM.read(i));
  }

  if (!(int)ssid.charAt(0)) return 0; // Parameters NOT found in EEPROM

  Serial.println();
  Serial.print("SSID: ");
  Serial.println(ssid); 
  
  //  password
  Serial.println("Reading EEPROM pass");

  password = "";
  for (int i = 32; i < 96; ++i)
  {
    password += char(EEPROM.read(i));
  }

  Serial.print("PASS: ");
  Serial.println(password);

  //  thingspeack - Channel ID
  Serial.println("Reading EEPROM Channel ID");

  myChannel = "";
  for (int i = 96; i < 104; ++i)
  {
    myChannel += char(EEPROM.read(i));
  }
  myChannelNumber = atol(myChannel.c_str());

  Serial.print("Channel ID: ");
  Serial.println(myChannel);

  //  thingspeack - Write API Key
  Serial.println("Reading EEPROM Write API Key");

  myWriteAPIKey = ""; 
  for (int i = 104; i < 136; ++i)
  {
    myWriteAPIKey += char(EEPROM.read(i));
  }

  Serial.print("Write APi Key: ");
  Serial.println(myWriteAPIKey);

  //  thingspeack - Read API Key
  Serial.println("Reading EEPROM Read API Key");

  myReadAPIKey = "";
  for (int i = 136; i < 168; ++i)
  {
    myReadAPIKey += char(EEPROM.read(i));
  }

  Serial.print("Read APi Key: ");
  Serial.println(myReadAPIKey);

  return 1; // Parameters found in EEPROM
}


/*----------------------------------------------------------
    Write meter parameters to EEPROM
  ----------------------------------------------------------*/
void writeEEPROM(String qsid, String qpass, String qchannel_id, String qwrite_api_key, String qread_api_key){
  //qsid = 32, qpass = 64, channel_id=8, write_api_key=32, read_api_key = 32

  for (int i = 0; i < 168; ++i) {
    EEPROM.write(i, 0);
  }
  Serial.println(qsid);
  Serial.println("");
  Serial.println(qpass);
  Serial.println("");
  Serial.println(qchannel_id);
  Serial.println("");
  Serial.println(qwrite_api_key);
  Serial.println("");
  Serial.println(qread_api_key);
  Serial.println("");

  Serial.println("writing eeprom ssid:");
  for (int i = 0; i < qsid.length(); ++i){
    EEPROM.write(i, qsid[i]);
    Serial.print("Wrote: ");
    Serial.println(qsid[i]);
  }
        
  Serial.println("writing eeprom pass:");
  for (int i = 0; i < qpass.length(); ++i){
    EEPROM.write(32 + i, qpass[i]);
    Serial.print("Wrote: ");
    Serial.println(qpass[i]);
  }
        
  Serial.println("writing eeprom Channel ID:");
  for (int i = 0; i < qchannel_id.length(); ++i){
    EEPROM.write(96 + i, qchannel_id[i]);
    Serial.print("Wrote: ");
    Serial.println(qchannel_id[i]);
  }

  Serial.println("writing eeprom Write API Key:");
  for (int i = 0; i < qwrite_api_key.length(); ++i){
    EEPROM.write(104 + i, qwrite_api_key[i]);
    Serial.print("Wrote: ");
    Serial.println(qwrite_api_key[i]);
  }

  Serial.println("writing eeprom Read API Key:");
  for (int i = 0; i < qread_api_key.length(); ++i){
    EEPROM.write(136 + i, qread_api_key[i]);
    Serial.print("Wrote: ");
    Serial.println(qread_api_key[i]);
  }
        
  EEPROM.commit();
}


void launchWeb()
{
  //Serial.println("");
  //if (WiFi.status() == WL_CONNECTED)
  //  Serial.println("WiFi connected");
  //Serial.print("Local IP: ");
  //Serial.println(WiFi.localIP());
  //Serial.print("SoftAP IP: ");
  //Serial.println(WiFi.softAPIP());
  createWebServer();
  // Start the server
  server.begin();
  Serial.println("Server started");
  Serial.print(" Access Point Activated, waiting for connections. Configure your Wifi Network connecting to ");
  Serial.print(AP_ssid);
  Serial.print(" and typing ");
  Serial.print(WiFi.softAPIP());
  Serial.print(" in your web browser \n");
}


void createWebServer()
{
  {
    server.on("/", []() {

      IPAddress ip = WiFi.softAPIP();
      String ipStr = String(ip[0]) + '.' + String(ip[1]) + '.' + String(ip[2]) + '.' + String(ip[3]);

      //HTML CONTENT
      content = "<!DOCTYPE html>";
      content += "<html>";

      // HEAD
      content += "<head>";
      content +="<meta charset=\"UTF-8\">";
      content +="<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">";
      content +="<title>Aprovisionamiento Wi Fi UNCPBA - CO2 </title>";

      // STYLES
      content +="<style>";
      content +="html{font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}";
      content +=".button { background-color: #187c1d; border: none; border-radius: 10px;color: white; padding: 12px 30px; text-decoration: none; font-size: 20px; margin: 10px; cursor: pointer;}";
      content +=".button-dark {background-color: #555555;}";
      content +=".red {background-color: #e7e4e4; padding: 10px;}";
      content +="label{font-weight: bold;}";
      content +=".center{text-align: center;list-style-position: inside;}",
      content +=".input_text {line-height: 20px; padding: 5px; border-radius: 4px; border: 2px #555555;}</style></head>";

      //BODY
      content +="<body>";
      content +="<h1>Bienvenido al acceso de credenciales WiFi del dispositivo</h1>";
      content +="<hr>";
      content +="<form action=\"/scan\" method=\"POST\"><input class=\"button button-dark\" type=\"submit\" value=\"Escanear\"></form>";

      //Ordered List of scanned list
      content += ipStr;
      content +="<h2>Redes Disponibles</h2>";
      content += "<p>Total Redes:";
      content += nets;
      content += "</p>";
      content += "<p>";
      content += st;
      content += "</p>";

      content +="<form class=\"red\" method='get' action='setting'>";
      content +="<h3>Datos de red para el dispositivo</h3>";
      content +="<hr>";
      content +="<br>";
      content +="<label>SSID - Número de Red: </label>";
      content +="<input class=\"input_text\" name='ssid_number' type='number' min='1' max='"; 
      content += nets; // Number of available networks
      content += "'length=32>";
      content +="<br>";
      content +="<br>";
      content +="<label>Pass: </label>";
      content +="<input class=\"input_text\" name='pass' length=64  type=\"password\">";
      content +="<br>";
      content +="<br>";
      content +="<label>Thingspeak Channel ID: </label>";
      content +="<input class=\"input_text\" name='channel_id' length=8  type=\"text\">";
      content +="<br>";
      content +="<br>";
      content +="<label>Thingspeak Write Api Key: </label>";
      content +="<input class=\"input_text\" name='write_api_key' length=32  type=\"text\">";
      content +="<br>";
      content +="<br>";
      content +="<label>Thingspeak Read Api Key: </label>";
      content +="<input class=\"input_text\" name='read_api_key' length=32  type=\"text\">";
      content +="<br>";
      content +="<br>";
      content +="<input class=\"button\" type='submit' value=\"Enviar\">";
      content +="</form>";
      content +="</body>";
      content +="</html>";
      
      server.send(200, "text/html", content);
    });
    
    server.on("/scan", []() {
      //setupAP();
      IPAddress ip = WiFi.softAPIP();
      String ipStr = String(ip[0]) + '.' + String(ip[1]) + '.' + String(ip[2]) + '.' + String(ip[3]);

      content = "<!DOCTYPE HTML>\r\n<html>go back / volver";
      server.send(200, "text/html", content);
    });

    server.on("/setting", []() {

      String sid_number = server.arg("ssid_number");
      int qsid_number = sid_number.toInt();
      String qsid = WiFi.SSID(qsid_number-1);
      String qpass = server.arg("pass");
      String qchannel_id = server.arg("channel_id");
      String qwrite_api_key = server.arg("write_api_key");
      String qread_api_key = server.arg("read_api_key");

      Serial.println("Selected Network: " + qsid + ", " + qpass);
      Serial.println("TS Channel ID: " + qchannel_id);
      Serial.println("TS Write Api Key: " +  qwrite_api_key);
      Serial.println("TS Read Api Key: " +  qread_api_key);
      
      if (qsid.length() > 0 && qpass.length() > 0) {
        Serial.println("clearing eeprom");

        writeEEPROM(qsid, qpass, qchannel_id, qwrite_api_key, qread_api_key);
 
        content = "{\"Success\":\"saved to eeprom... reset to boot into new wifi\"}";
        statusCode = 200;
        ESP.restart();
      } else {
        content = "{\"Error\":\"404 not found\"}";
        statusCode = 404;
        Serial.println("Sending 404");
      }
      server.sendHeader("Access-Control-Allow-Origin", "*");
      server.send(statusCode, "application/json", content);

    });
  }
}


void setupAP()
{
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);
  nets = WiFi.scanNetworks();
  Serial.println("scan done");
  if (nets == 0)
    Serial.println("no networks found");
  else
  {
    Serial.print(nets);
    Serial.println(" networks found");
    for (int i = 0; i < nets; ++i)
    {
      // Print SSID and RSSI for each network found
      Serial.print(i + 1);
      Serial.print(": ");
      Serial.print(WiFi.SSID(i));
      Serial.print(" (");
      Serial.print(WiFi.RSSI(i));
      Serial.print(")");
      
      delay(10);
    }
  }
  Serial.println("");
  st = "<ol class=\"center\">";
  for (int i = 0; i < nets; ++i)
  {
    // Print SSID and RSSI for each network found
    st += "<li>";
    st += WiFi.SSID(i);
    st += " (";
    st += WiFi.RSSI(i);

    st += ")";
    
    st += "</li>";
  }
  st += "</ol>";
  
  delay(100);
  
  WiFi.softAP(AP_ssid, AP_pass);
  //////////////////////////////launchWeb();
}


/*----------------------------------------------------------
    CO2 meter setup
  ----------------------------------------------------------*/
void setup() {
  Serial.begin(9600);

#ifdef HWSERIAL
  Serial2.begin(BAUDRATE, SERIAL_8N1, rx2_pin, tx2_pin);
  mhz19.begin(Serial2);
#else
  mySerial.begin(BAUDRATE);
  mhz19.begin(mySerial);
#endif

  mhz19.autoCalibration(false); // make sure auto calibration is off

  RGB_LEDSetup();

  setRGB_LEDColor (0, 0, 255);  // Blue means warming or Configuring:
                                //   baseline setting or calibrating

  WiFi.disconnect();

  EEPROM.begin(512); //Initialasing EEPROM
  delay(10);
  updateRGB_LED (true);         // Blink while connecting
  if (readEEPROM()) // Read params from EEPROM, if present
    connectWiFi();  
  else {
    Serial.println ("Changing to Access Point mode..."); 
    launchWeb();          
    setupAP();    
    AP_MODE = true;
  }
  updateRGB_LED (false);

  ThingSpeak.begin(myClient);

  delay (180000); // Wait 3 minutes for warming purposes

  retrieveInfo_mhz19 ();

  pinMode(button1.PIN, INPUT_PULLUP);
  attachInterrupt(button1.PIN, isr_button, CHANGE);

  Serial.println("co2[ppm],temp[°C]");
}


/*----------------------------------------------------------
    Manage the used interacion captured in the push button#include <HTTPClient.h>
  ----------------------------------------------------------*/
void btnManager_prov (int co2) {
  if (button1.event) {
      Serial.printf("Button has been pressed for %u millis\n", button1.timePressed);
      if (button1.timePressed < 1000)
        CO2_base = co2;
      else if (button1.timePressed < 3000) 
        calibrate_mhz19 ();
      else {
        Serial.println ("Changing to Access Point mode..."); 
        launchWeb();          
        setupAP();    
        AP_MODE = true;
      }
      // button state updated inside a critical section
      portENTER_CRITICAL(&mux);
      button1.event = false;
      portEXIT_CRITICAL(&mux);
  } else if (button1.down)
    if (millis() - button1.timePressed > maxPressT) { // dismiss for time overruns (due to noise in the button pin)
      Serial.printf("Button was down for %u millis\n", millis() - button1.timePressed);
      portENTER_CRITICAL(&mux);
      button1.down = false;
      portEXIT_CRITICAL(&mux);
    }  
}


/*----------------------------------------------------------
    MH-Z19 CO2 sensor loop
  ----------------------------------------------------------*/
void loop() {

  int co2ppm = mhz19.getCO2();          // Request CO2 (as ppm)
  int temp = mhz19.getTemperature();    // Request Temperature (as Celsius)

  btnManager_prov (co2ppm);
  
  if (!AP_MODE){
    // Measurements to computer for debugging purposes
    //
    //Serial.print("co2: ");
    Serial.print(co2ppm);
    //Serial.print("temp: ");
    Serial.print(",");
    Serial.println(temp);

    if (d % S == 0) { // Telemetry is sent every S samplingPeriod seconds
      // Measurements to WiFi thingspeak IoT Platform
      //
      if (WiFi.status() != WL_CONNECTED) connectWiFi();

      ThingSpeak.setField (1, co2ppm);
      ThingSpeak.setField (2, temp);

      ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey.c_str());
      Serial.println("Datos enviados a ThingSpeak.");
    }
    d++;

    CO2RGB_LED(co2ppm);

    delay(samplingPeriod);
  } 
  
  else { // Meter in Access Point mode
    while ((WiFi.status() != WL_CONNECTED)){
      delay(4000);
      server.handleClient();
    }
  }
}
