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

  Copyright (c) 2021-2022 Elías Todorovich, Matías Presso

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
  Getting Date and Time from NTP Server
  ----------------------------------------------------------*/
#include "time.h"
const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 3600*(-3); // (GMT-3)
const int   daylightOffset_sec = 0; // offset in seconds for daylight saving time. It is generally one hour=3600

struct Mment {
  int s_co2ppm, s_temp;
  struct tm  s_timeinfo;
};

// When WiFi is not available, measurements are saved every 5 minutes. Here we can store up tu 3 hours of measurements.
const int MAX_MM = 12*3;
struct Mment mm2send[MAX_MM];
int saved2send = 0; 

void printLocalTime(){
  struct tm timeinfo;
  char buffer [25];
  
  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
    return;
  }
  Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");

  Serial.print("For thingspeak, the timestamp string must be in the ISO 8601 format: ");
  // strftime doc: https://www.cplusplus.com/reference/ctime/strftime/
  strftime (buffer,25,"%F %T-03",&timeinfo); // Example "2017-01-12 13:22:54-05"
  Serial.println(buffer);
  
  Serial.println();
}

/*----------------------------------------------------------
    Connect your device to the wireless network
  ----------------------------------------------------------*/
void connectWiFi_1(){
    while (WiFi.status() != WL_CONNECTED){
        WiFi.begin(ssid, password);
        Serial.print(".");
        updateRGB_LED (true);         // Blink while connecting
        delay(3000);
    }

    // Display a notification that the connection is successful. 
    Serial.println("Connected"); 
}

void connectWiFi(){
  int t = 0;
  while (WiFi.status() != WL_CONNECTED){
    WiFi.begin(ssid, password);
    Serial.print(".");
    updateRGB_LED (true);         // Blink while connecting
    delay(3000);
    if (t++>100) return; // 60s*5=300s=5min
  }

  // Display a notification that the connection is successful. 
  Serial.println("Connected"); 
}

/*----------------------------------------------------------
    MH-Z19 CO2 sensor setup
  ----------------------------------------------------------*/
void setup() {
  //gt = millis();
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
  connectWiFi_1();
  updateRGB_LED (false); 

  // Init and get the time
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  printLocalTime();

  ThingSpeak.begin(myClient);

  delay (180000); // Wait 3 minutes for warming purposes

  retrieveInfo_mhz19 ();

  pinMode(button1.PIN, INPUT_PULLUP);
  attachInterrupt(button1.PIN, isr_button, CHANGE);
 
  //Serial.println("co2[ppm],temp[°C]");
}

/*----------------------------------------------------------
    MH-Z19 CO2 sensor loop
  ----------------------------------------------------------*/
void loop() {
  int co2ppm = mhz19.getCO2();          // Request CO2 (as ppm)
  int temp   = mhz19.getTemperature();    // Request Temperature (as Celsius)      
  struct tm timeinfo;  
  char buffer [25];                  

  btnManager (co2ppm);
  
  // Measurements to computer for debugging purposes
  //
  //Serial.print("co2: "); 
  Serial.print(co2ppm);
  //Serial.print("temp: "); 
  Serial.print(","); 
  Serial.println(temp);
 
  if (d%S==0){     // Telemetry is sent every S samplingPeriod seconds

   if (WiFi.status() != WL_CONNECTED)  connectWiFi();
   
   if (WiFi.status() == WL_CONNECTED) {
      // Measurements to WiFi thingspeak IoT Platform 
      // 
      ThingSpeak.setField (1,co2ppm);
      ThingSpeak.setField (2,temp);

      ThingSpeak.writeFields(myChannelNumber,myWriteAPIKey);
      Serial.println("Data sent to Thingspeak.");
   } else {
      // Measurements must be sent later when WiFi is available
      if (saved2send < MAX_MM) saved2send++;
      mm2send[saved2send-1].s_co2ppm = co2ppm;
      mm2send[saved2send-1].s_temp   = temp;
      if(!getLocalTime(&timeinfo)){
        Serial.println("Failed to obtain time");
        return;
      }
      mm2send[saved2send-1].s_timeinfo = timeinfo;
      Serial.println("Measurement saved for later registration in the IoT platform...");
   }
     
  } else { // Saved telemetry, if any, is sent
    if (WiFi.status() == WL_CONNECTED && saved2send > 0) {
      ThingSpeak.setField (1,mm2send[saved2send-1].s_co2ppm);
      ThingSpeak.setField (2,mm2send[saved2send-1].s_temp);
      // Example: "2017-01-12 13:22:54-05"
      strftime (buffer,25,"%F %T-03",&(mm2send[saved2send-1].s_timeinfo)); // Example "2017-01-12 13:22:54-05"
      ThingSpeak.setCreatedAt(buffer);

      ThingSpeak.writeFields(myChannelNumber,myWriteAPIKey);
      Serial.print("Datos guardados anteriormente enviados a ThingSpeak: "); Serial.println(buffer);
      if (saved2send > 0) saved2send--;
     }
  }
  d++;

  CO2RGB_LED(co2ppm);
  
  delay(samplingPeriod);
}
