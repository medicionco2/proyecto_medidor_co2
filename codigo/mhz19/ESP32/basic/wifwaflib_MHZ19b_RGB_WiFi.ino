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

#ifndef ESP32
#define ESP32
#endif

// Comment the following line if you use SoftwareSerial
#define HWSERIAL

/*----------------------------------------------------------
!!Experimental!! The baseline from which CO2 increments are 
considered is 400ppm by default.

In environments with CO2 sources other than people, a new 
baseline can be set using the push button. It is recommended 
not to use this functionality because it is experimental. 
  ----------------------------------------------------------*/
int CO2_base = 400;

/*----------------------------------------------------------
    MH-Z19 CO2 sensor: UART2 in a ESP32s SoC
  ----------------------------------------------------------*/
#include <MHZ19.h>
MHZ19 mhz19;             // Constructor for library

const int rx2_pin = 16;	//Serial rx1 pin no
const int tx2_pin = 17;	//Serial tx1 pin no
#define BAUDRATE 9600    // Device to MH-Z19 Serial baudrate (should not be changed)

#ifndef HWSERIAL
  #include <SoftwareSerial.h> 
  SoftwareSerial mySerial(rx2_pin, tx2_pin);
#endif

/*----------------------------------------------------------
  Basic WiFi Server in a NodeMCU ESP32s SoC 
  ----------------------------------------------------------*/
#include <WiFi.h>

const char* ssid     = "Fibertel WiFi668 2.4GHz";
const char* password = "01424988126";

WiFiServer server(80);

/*----------------------------------------------------------
  RGB LED in a NodeMCU ESP32s SoC 
  ----------------------------------------------------------*/
const int led_R   = 12; 
const int PWMR_Ch = 0;
const int led_G   = 14;
const int PWMG_Ch = 1; 
const int led_B   = 27; 
const int PWMB_Ch = 2;

const int PWM_Freq = 1000;
const int PWM_Res = 8;

/*----------------------------------------------------------
  Push Button for configuation control 
  ----------------------------------------------------------*/
struct Button {
  const uint8_t PIN;
  uint32_t timePressed;
  bool down;
  bool event;
};

const int btn_PIN   = 21;
const int debounceThresh = 70; // milliseconds

Button button1 = {btn_PIN, 0, false, false};

// The Interrupt Service Routine is placed in the RAM of the ESP32
void IRAM_ATTR isr_button() {
  static uint32_t lastMillis = 0;

  if (millis() - lastMillis > debounceThresh){ // debounce
    if (!button1.down){
      button1.down = true;
      setRGB_LEDColor (0, 0, 255);  // Blue means warming or Configuring:
                                //   baseline setting or calibrating
    //Serial.println("Down...");
    } else { 
      button1.down = false;
    //Serial.println("... and Up");
      button1.timePressed = millis() - lastMillis;
      button1.event = true;
    }
  }
  lastMillis = millis();
}

/*----------------------------------------------------------
    getVersion(char array[]) returns version number to the 
    argument. The first 2 char are the major version, and 
    second 2 bytes the minor version. e.g 02.11
  ----------------------------------------------------------*/
void retrieveInfo_mhz19 () {
  char myVersion[4];          
  mhz19.getVersion(myVersion);

  Serial.print("\nFirmware Version: ");
  for(byte i = 0; i < 4; i++)
  {
    Serial.print(myVersion[i]);
    if(i == 1)
      Serial.print(".");    
  }
   Serial.println("");

   Serial.print("Range: ");
   Serial.println(mhz19.getRange());   
   Serial.print("Background CO2: ");
   Serial.println(mhz19.getBackgroundCO2());
   Serial.print("Temperature Cal: ");
   Serial.println(mhz19.getTempAdjustment());
   Serial.print("ABC Status: "); mhz19.getABC() ? Serial.println("ON") :  Serial.println("OFF");
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

  ledcAttachPin(led_R, PWMR_Ch);
  ledcSetup(PWMR_Ch, PWM_Freq, PWM_Res);
  ledcAttachPin(led_G, PWMG_Ch);
  ledcSetup(PWMG_Ch, PWM_Freq, PWM_Res);
  ledcAttachPin(led_B, PWMB_Ch);
  ledcSetup(PWMB_Ch, PWM_Freq, PWM_Res);

  setRGB_LEDColor (0, 0, 255);  // Blue means warming or Configuring:
                                //   baseline setting or calibrating

  pinMode(button1.PIN, INPUT_PULLUP);
  attachInterrupt(button1.PIN, isr_button, CHANGE);

  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
    
  server.begin();

  delay (180000); // Wait 3 minutes for warming purposes

  retrieveInfo_mhz19 ();
 
  Serial.println("co2[ppm],temp[°C]");
}

/*----------------------------------------------------------
    Set the RGB LED color
  ----------------------------------------------------------*/
void setRGB_LEDColor(int red, int green, int blue)
{
    ledcWrite(PWMR_Ch, red  ); 
    ledcWrite(PWMG_Ch, green);
    ledcWrite(PWMB_Ch, blue );
}

/*----------------------------------------------------------
    For calibration, the instrument must be placed in the 
    open air.
  ----------------------------------------------------------*/
void calibrate_mhz19()
{
  // the device must remain stable, outdoors for waitingMinutes minutes
  const int waitingMinutes = 21;  
  const long waitingSeconds = waitingMinutes * 60L; 
  long cnt_cal = 0; 

  byte color_intensity = 255;
  
  setRGB_LEDColor (0, 0, color_intensity);  // Blue means warming or Configuring:
                                            //   baseline setting or calibrating

  while (cnt_cal <= waitingSeconds) { // wait for waitingMinutes minutes
    ++cnt_cal;
    color_intensity = color_intensity-50; 
    
    setRGB_LEDColor (0, 0, color_intensity);

    delay (1000);   // One second
  }
  // waitingMinutes minutes elapsed  
  mhz19.calibrate ();  // Take a reading which be used as the zero point for 400 ppm

  retrieveInfo_mhz19 ();
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

  if (button1.event) {
      Serial.printf("Button has been pressed for %u millis\n", button1.timePressed);
      if (button1.timePressed < 1000)
        CO2_base = co2ppm;
      else calibrate_mhz19 ();
      button1.event = false;
  }

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

  if (!button1.down) 
  // CO2 measurement to RGB LED
  if (co2ppm < CO2_base+300) {
    // Green means low risk
    setRGB_LEDColor (0, 255, 0);}
  else if (co2ppm < CO2_base+400) {
    // Yellow means medium risk
    setRGB_LEDColor (255, 50, 0);}
  else if (co2ppm < CO2_base+600) {
    // Red means high risk
    setRGB_LEDColor (255, 0, 0);}
  else {
    // Purple means more than high risk
    setRGB_LEDColor (80, 0, 80);
  }
  
  delay(6000);
}
