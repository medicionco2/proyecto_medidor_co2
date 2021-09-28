/*  
   ECO2 common functionality 
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

const int samplingPeriod = 15000; // 15 seconds

/*----------------------------------------------------------
    MH-Z19 CO2 sensor: UART2 in a ESP32s SoC
  ----------------------------------------------------------*/
#include <MHZ19.h>
MHZ19 mhz19;             // Constructor for library

const int rx2_pin = 16;  //Serial rx1 pin no
const int tx2_pin = 17; //Serial tx1 pin no
#define BAUDRATE 9600    // Device to MH-Z19 Serial baudrate (should not be changed)

#ifndef HWSERIAL
  #include <SoftwareSerial.h> 
  SoftwareSerial mySerial(rx2_pin, tx2_pin);
#endif

/*----------------------------------------------------------
  RGB LED in a ESP32
  ----------------------------------------------------------*/
const int led_R   = 12; 
const int PWMR_Ch = 0;
const int led_G   = 14;
const int PWMG_Ch = 1; 
const int led_B   = 27; 
const int PWMB_Ch = 2;

const int PWM_Freq = 1000;
const int PWM_Res = 8;

struct RGB_Color {
  byte R;
  byte G;
  byte B;
};
RGB_Color RGB_LED = {0, 0, 255}; // Blue by default

/*----------------------------------------------------------
  Push Button for configuation control 
  ----------------------------------------------------------*/
struct Button {
  const uint8_t PIN;
  uint32_t timePressed;
  bool down;
  bool event;
};

const int btn_PIN        = 21;
const int debounceThresh = 70  ; // milliseconds
const int maxPressT      = 6000; // milliseconds

/*----------------------------------------------------------
  The volatile modifier directs the compiler to load the variable
  from RAM, and not from a register. Specificaly, button1 can
  be changed by the ISR, beyond the control of the main code.
  ----------------------------------------------------------*/
volatile Button button1 = {btn_PIN, 0, false, false};

// For synchronization between the main code and the interrupt
portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;

/*----------------------------------------------------------
 The number of captured interrupts is sometimes odd. Falling
 edges should be followed by rising edges, but the software 
 does not capture everything that happens physically. To 
 solve by software both the debouncing problem, and eventual 
 electrical noise, the resulting code can look kind of weird.

 Capturing user interaction is priority and debounce is the 
 firt goal. Therefore, the first edge cannot be discarded, 
 even if it is followed by a possible short pulse.
 
 For efficiency,the Interrupt Service Routine is placed in 
 the RAM of the ESP32.
  ----------------------------------------------------------*/
void IRAM_ATTR isr_button() {
  static uint32_t lastMillis = 0;
  uint32_t nowMillis, elapsedT;

  //nowMillis = millis();
  nowMillis = xTaskGetTickCountFromISR(); // safe to be called from an ISR
  elapsedT = nowMillis - lastMillis;
  //Serial.printf("Elapsed %u millis\n", elapsedT);

  if (elapsedT > debounceThresh){ // debounce
    // button state updated inside a critical section to prevent 
    //   concurrent access problems
    portENTER_CRITICAL(&mux);
    if (!button1.down){
      button1.down = true;
      button1.timePressed = nowMillis; // to control time overruns due to noise
    } else { 
      button1.down = false;
      // Ignore too long keystrokes
      if (elapsedT < maxPressT) {
        button1.timePressed = elapsedT;
        button1.event = true;
      }
    }
    portEXIT_CRITICAL(&mux);  
  } 

  lastMillis = nowMillis;

  //Serial.printf("Exit %u last millis\n", lastMillis);
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
  RGB LED setup in a ESP32
  ----------------------------------------------------------*/
void RGB_LEDSetup() {
  ledcAttachPin(led_R, PWMR_Ch);
  ledcSetup(PWMR_Ch, PWM_Freq, PWM_Res);
  ledcAttachPin(led_G, PWMG_Ch);
  ledcSetup(PWMG_Ch, PWM_Freq, PWM_Res);
  ledcAttachPin(led_B, PWMB_Ch);
  ledcSetup(PWMB_Ch, PWM_Freq, PWM_Res);
}

/*----------------------------------------------------------
    Set the RGB LED color
  ----------------------------------------------------------*/
void setRGB_LEDColor(int red, int green, int blue)
{
  RGB_LED.R = red;
  RGB_LED.G = green;
  RGB_LED.B = blue;
    /*ledcWrite(PWMR_Ch, red  ); 
    ledcWrite(PWMG_Ch, green);
    ledcWrite(PWMB_Ch, blue );*/
}

void updateRGB_LED(bool blk) {
static bool LEDState = true;

  if (!blk or LEDState) {
    ledcWrite(PWMR_Ch, RGB_LED.R); 
    ledcWrite(PWMG_Ch, RGB_LED.G);
    ledcWrite(PWMB_Ch, RGB_LED.B);
  } else {
    ledcWrite(PWMR_Ch, 0); 
    ledcWrite(PWMG_Ch, 0);
    ledcWrite(PWMB_Ch, 0);
  }

  if (blk) LEDState = !LEDState; 
}

/*----------------------------------------------------------
    Feedback to RGB LED. CO2 measurement to RGB LED
  ----------------------------------------------------------*/
void CO2RGB_LED(int co2) {
  if (co2 < CO2_base+300) {
    // Green means low risk
    setRGB_LEDColor (0, 255, 0);}
  else if (co2 < CO2_base+400) {
    // Yellow means medium risk
    setRGB_LEDColor (255, 50, 0);}
  else if (co2 < CO2_base+600) {
    // Red means high risk
    setRGB_LEDColor (255, 0, 0);}
  else {
    // Purple means more than high risk
    setRGB_LEDColor (80, 0, 80); }
  
  /*if (button1.down) {
    setRGB_LEDColor (0, 0, 255); // Blue because the button is down
  }*/
  updateRGB_LED (false);
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
  updateRGB_LED (false);

  while (cnt_cal <= waitingSeconds) { // wait for waitingMinutes minutes
    ++cnt_cal;
    color_intensity = color_intensity-50; 
    
    setRGB_LEDColor (0, 0, color_intensity); updateRGB_LED (false);

    delay (1000);   // One second
  }
  // waitingMinutes minutes elapsed  
  mhz19.calibrate ();  // Take a reading which be used as the zero point for 400 ppm

  retrieveInfo_mhz19 ();
}

/*----------------------------------------------------------
    Manage the used interacion captured in the push button
  ----------------------------------------------------------*/
void btnManager (int co2) {
  if (button1.event) {
      Serial.printf("Button has been pressed for %u millis\n", button1.timePressed);
      if (button1.timePressed < 1000)
        CO2_base = co2;
      else calibrate_mhz19 ();
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
