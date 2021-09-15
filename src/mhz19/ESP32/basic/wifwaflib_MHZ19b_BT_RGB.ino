/*
  hello_MHZ19b_BT_RGB.ino - MH-Z19b CO2 sensor basic program for ESP32 
                            This program uses the library from
                                https://github.com/WifWaf/MH-Z19
                            Warning level signal RGB LED
                            Push Button for configuation control
                                (1) If the button is held down for less than 
                                one second, it sets the CO2 baseline, 
                                (2) if the duration is longer than one second, 
                                the device goes to calibration mode.
                            Bluetooth support

                            A posible bluetooth client is Serial Bluetooth Terminal:
                            https://play.google.com/store/apps/details?id=de.kai_morich.serial_bluetooth_terminal&hl=en&gl=US

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
  Bluetooth in a ESP32s SoC 
  ----------------------------------------------------------*/
#include "BluetoothSerial.h"

#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif
BluetoothSerial SerialBT;
   
  /*----------------------------------------------------------
    MH-Z19 CO2 sensor  setup
  ----------------------------------------------------------*/
void setup() {
  SerialBT.begin("NodeMCU ESP32s"); //Bluetooth device name
  Serial.begin(9600);

#ifdef HWSERIAL
  Serial2.begin(BAUDRATE, SERIAL_8N1, rx2_pin, tx2_pin);
  mhz19.begin(Serial2);
#else
  mySerial.begin(BAUDRATE); 
  mhz19.begin(mySerial);
#endif
  
  mhz19.autoCalibration(false);     // make sure auto calibration is off
  //mhz19.autoCalibration(true);     // make sure auto calibration is on

  RGB_LEDSetup();

  setRGB_LEDColor (0, 0, 255);  // Blue means warming or Configuring:
                                //   baseline setting or calibrating
  updateRGB_LED (false);

  pinMode(button1.PIN, INPUT_PULLUP);
  attachInterrupt(button1.PIN, isr_button, CHANGE);
  
  delay (180000); // Wait 3 minutes for warming purposes

  retrieveInfo_mhz19 ();
  
  Serial.println("co2[ppm],temp[°C]");
  
  SerialBT.println("=== Initialized ===");
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

  // Measurements to Bluetooth serial interface 
  //
  SerialBT.print("co2: ");
  SerialBT.print(co2ppm);
  SerialBT.print("; temp: ");
  SerialBT.println(temp);

  CO2RGB_LED(co2ppm);
  
  delay(samplingPeriod);
}
