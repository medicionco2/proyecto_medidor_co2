/*
  wifwaflib_MHZ19b_IoT_thingspeak.ino - Program for ESP32 and MH-Z19b 
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

int d=0; 
const int S=3; // Samples per telemetry recording

/*----------------------------------------------------------
  WiFi connection in a ESP32s 
  ----------------------------------------------------------*/
#include <WiFi.h>
#include "secrets.h"

char ssid[] = SECRET_SSID;       // your network SSID (name) 
char password[] = SECRET_PASS;   // your network password

WiFiClient myClient;

/*----------------------------------------------------------
  ThingSpeak settings
  ----------------------------------------------------------*/
#include "ThingSpeak.h"
  
char server[] = "api.thingspeak.com";
unsigned long myChannelNumber = SECRET_CH_ID;
const char* myWriteAPIKey = SECRET_WRITE_APIKEY;

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
  
  updateRGB_LED (true);         // Blink while connecting
  connectWiFi();
  updateRGB_LED (false); 

  ThingSpeak.begin(myClient);

  delay (180000); // Wait 3 minutes for warming purposes

  retrieveInfo_mhz19 ();

  pinMode(button1.PIN, INPUT_PULLUP);
  attachInterrupt(button1.PIN, isr_button, CHANGE);
 
  Serial.println("co2[ppm],temp[°C]");
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

  if (d%S==0){     // Telemetry is sent every S samplingPeriod seconds
  // Measurements to WiFi thingspeak IoT Platform 
  //
   if (WiFi.status() != WL_CONNECTED) connectWiFi();

   ThingSpeak.setField (1,co2ppm);
   ThingSpeak.setField (2,temp);

   ThingSpeak.writeFields(myChannelNumber,myWriteAPIKey);
   Serial.println("Datos enviados a ThingSpeak.");   
  }
  d++;

  CO2RGB_LED(co2ppm);
  
  delay(samplingPeriod);
}
