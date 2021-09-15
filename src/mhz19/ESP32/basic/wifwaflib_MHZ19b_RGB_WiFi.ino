/*
  hello_MHZ19b_RGB_WiFi.ino - MH-Z19b CO2 sensor basic program for ESP32 
                              This program uses the library from
                                  https://github.com/WifWaf/MH-Z19
                              Warning level signal RGB LED
                              Push Button for configuation control
                                (1) If the button is held down for less than 
                                one second, it sets the CO2 baseline, 
                                (2) if the duration is longer than one second, 
                                the device goes to calibration mode.
                              Basic WiFi Server support

  Copyright (c) 2021 Elías Todorovich

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

/*----------------------------------------------------------
  Basic WiFi Server in a NodeMCU ESP32s SoC 
  ----------------------------------------------------------*/
#include <WiFi.h>
#include "secrets.h"

char ssid[]     = SECRET_SSID;   // your network SSID (name) 
char password[] = SECRET_PASS;   // your network password

WiFiServer server(80);

/*----------------------------------------------------------
    Connect your device to the wireless network
  ----------------------------------------------------------*/
void connectWiFi(){

    while (WiFi.status() != WL_CONNECTED){
        WiFi.begin(ssid, password);
        Serial.print(".");
        updateRGB_LED (true);         // Blink while connecting
        delay(3000);
    }

    // Display a notification that the connection is successful. 
    Serial.println("Connected"); 
}

/*----------------------------------------------------------
    MH-Z19 CO2 sensor setup
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
  
  mhz19.autoCalibration(false);     // make sure auto calibration is off

  RGB_LEDSetup();

  setRGB_LEDColor (0, 0, 255);  // Blue means warming or Configuring:
                                //   baseline setting or calibrating

  pinMode(button1.PIN, INPUT_PULLUP);
  attachInterrupt(button1.PIN, isr_button, CHANGE);

  updateRGB_LED (true);         // Blink while connecting
  connectWiFi();
  updateRGB_LED (false);

  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
    
  server.begin();

  delay (180000); // Wait 3 minutes for warming purposes

  retrieveInfo_mhz19 ();
 
  Serial.println("co2[ppm],temp[°C]");
}

String prepareHtmlPage(int co2, int temp)
{
  String htmlPage;
  htmlPage.reserve(1024);               // prevent ram fragmentation
  htmlPage = F("HTTP/1.1 200 OK\r\n"
               "Content-Type: text/html\r\n"
               "Connection: close\r\n"  // the connection will be closed after completion of the response
               "Refresh: 6\r\n"         // refresh the page automatically every 6 sec
               "\r\n"
               "<!DOCTYPE HTML>"
               "<html> <body>"
               "<h1>Hello CO2</h1>"
               "<h2>CO2 baseline: ");
  htmlPage += CO2_base; // prints CO2 baseline
  htmlPage += F(" ppm </h2>");
               
  if (co2 > CO2_base+400) {
    // Red means high risk             
    htmlPage += F("<p style=\"color:red;\"> co2:  ");}
  else { htmlPage += F("<p> co2:  ");}
  htmlPage += co2; // prints CO2 measurement
  htmlPage += F(" ppm <br>"
                "temp:  ");
  htmlPage += temp; // temperature value
  htmlPage += F(" oC </p></body></html>"
                "\r\n");
  return htmlPage;
}

/*----------------------------------------------------------
    MH-Z19 CO2 sensor loop
  ----------------------------------------------------------*/
void loop() {
  int co2ppm = mhz19.getCO2();          // Request CO2 (as ppm)
  int temp = mhz19.getTemperature();    // Request Temperature (as Celsius)

  btnManager (co2ppm);

  // Measurements to computer for debugging purposes
  //
  //Serial.print("co2: "); 
  Serial.print(co2ppm);
  //Serial.print("temp: "); 
  Serial.print(","); 
  Serial.println(temp);

  // Measurements to WiFi Server 
  //
  WiFiClient client = server.available();
  // wait for a client (web browser) to connect
  if (client)
  {
//*    Serial.println("\n[Client connected]");
    while (client.connected())
    {
      // read line by line what the client (web browser) is requesting
      if (client.available())
      {
        String line = client.readStringUntil('\r');
//*        Serial.print(line);
        // wait for end of client's request, that is marked with an empty line
        if (line.length() == 1 && line[0] == '\n')
        {
          client.println(prepareHtmlPage(co2ppm, temp));
          break;
        }
      }
    }

    while (client.available()) {
      // but first, let client finish its request
      // that's diplomatic compliance to protocols
      // (and otherwise some clients may complain, like curl)
      // (that is an example, prefer using a proper webserver library)
      client.read();
    }

    // close the connection:
    client.stop();
//*    Serial.println("[Client disconnected]");
  }

  CO2RGB_LED(co2ppm);
  
  delay(samplingPeriod);
}
