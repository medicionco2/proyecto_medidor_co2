/*
  hello_MHZ19b_RGB_WiFi.ino - MH-Z19b CO2 sensor basic program for ESP32 
                              Warning level signal RGB LED
                              Push Button for configuation control
                                (1) If the button is held down for less than 
                                one second, it sets the CO2 baseline, 
                                (2) if the duration is longer than one second, 
                                the device goes to calibration mode.
                              Basic WiFi Server support
  version 0.1

  Copyright (c) 2021 Elías Todorovich

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU Affero General Public License as
  published by the Free Software Foundation, either version 3 of the
  License, or (at your option) any later version. 
*/

#ifndef ARDUINO_ARCH_ESP32
#define ARDUINO_ARCH_ESP32
#endif

int CO2_base = 400;

/*----------------------------------------------------------
    MH-Z19 CO2 sensor: UART2 in a NodeMCU ESP32s SoC
  ----------------------------------------------------------*/
#include <MHZ19_uart.h>
MHZ19_uart mhz19;

const int rx2_pin = 16;	//Serial rx1 pin no
const int tx2_pin = 17;	//Serial tx1 pin no

/*----------------------------------------------------------
  Basic WiFi Server in a NodeMCU ESP32s SoC 
  ----------------------------------------------------------*/
#include <WiFi.h>

const char* ssid     = "xxxxxxx";
const char* password = "xxxxxxx";

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
  bool pressed;
};

bool btn_down = false;

const int btn_PIN   = 21;
const int debounceThresh = 50;

Button button1 = {btn_PIN, 0, false};

// The Interrupt Service Routine is placed in the RAM of the ESP32
void IRAM_ATTR isr_button() {
  static uint32_t lastMillis = 0;

  if (millis() - lastMillis > debounceThresh){ // debounce
    if (!btn_down){
      btn_down = true;
      setRGB_LEDColor (0, 0, 255);  // Blue means warming or Configuring:
                                //   baseline setting or calibrating
    //Serial.println("Down...");
    } else { 
      btn_down = false;
    //Serial.println("... and Up");
      button1.timePressed = millis() - lastMillis;
      button1.pressed = true;
    }
  }
  lastMillis = millis();
}

/*----------------------------------------------------------
    MH-Z19 CO2 sensor  setup
    getStatus returns cero, so, I've bypassed this step 
  ----------------------------------------------------------*/
void setup() {
  Serial.begin(9600);
  
  mhz19.begin(rx2_pin, tx2_pin);
  mhz19.setAutoCalibration(false);
  
  /*while( mhz19.isWarming() ) {
    Serial.print("MH-Z19 now warming up...  status: ");
    Serial.println(mhz19.getStatus());
    delay(1000);
  } */

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

  delay (90000); // Wait 90 seconds for warming purposes
 
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
  const int waitingMinutes = 3;  //waiting 30 minutes
  const long waitingSeconds = waitingMinutes * 60L; 
  long cnt_cal = 0; 

  byte color_intensity = 255;
  
  setRGB_LEDColor (0, 0, color_intensity);  // Blue means warming or Configuring:
                                            //   baseline setting or calibrating

  while (cnt_cal <= waitingSeconds) { // espera media hora
    ++cnt_cal;
    color_intensity = color_intensity-50; 
    
    setRGB_LEDColor (0, 0, color_intensity);
    Serial.println(color_intensity);
    delay (1000);   // One second
  }
  // paso media hora  
  mhz19.calibrateZero (); // 1st zero calibration
  delay(60000); 
  mhz19.calibrateZero ();  // 2nd zero calibration
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
  int co2ppm = mhz19.getPPM();
  int temp = mhz19.getTemperature();

  if (button1.pressed) {
      Serial.printf("Button has been pressed for %u millis\n", button1.timePressed);
      if (button1.timePressed < 1000)
        CO2_base = co2ppm;
      else calibrate_mhz19 ();
      button1.pressed = false;
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

  if (!btn_down) 
  // CO2 measurement to RGB LED
  //
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
