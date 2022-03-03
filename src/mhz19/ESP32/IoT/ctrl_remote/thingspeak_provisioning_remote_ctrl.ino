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

//Local Device Credencials
const char* local_user_esp32 ="esp32";
const char* local_pass_esp32 ="4321";

//HTML CONTENT
String content;
String content_fixed_up;
String content_dynamic;
String content_fixed_down;
String netListHtml;

int totalNets;

bool AP_MODE = false; //Access Point State

/*----------------------------------------------------------
  ThingSpeak settings
  ----------------------------------------------------------*/
#include "ThingSpeak.h"

unsigned long myChannelNumber;
String myWriteAPIKey, myReadAPIKey;

/*----------------------------------------------------------
  Remote control settings
  ----------------------------------------------------------*/

//Calibration DATA
//Calibration Field, adjust your Thingspeak field for this purpose, remote_calibrate_state 1 (remote request), 2 (calibrate accepted by device), 0 (no calibrate, repose)
unsigned int remote_calibrate_field = 3; //Calibration Field, adjust your Thingspeak field for this purpose
int remote_calibrate_state = 0; //calibrate_state in device:  0 no calibrate, 1 calibrate request, 2 calibrate process 

//Example url to update a thingspeak field and request calibration (field3=1) https://api.thingspeak.com/update?api_key=THINGSPEAK-WRITE-API-KEY&field3=1 

//Restart DATA
unsigned int remote_restart_field = 4; //Restart Field, adjust your Thingspeak field for this purpose
int remote_restart_state = 0; //remote_restart_state in device:  0 no restart, 1 restart request, 2 restart process 

//Time Set DATA
unsigned int remote_timeset_field = 5; //Time Set Field, adjust your Thingspeak field for this purpose
int remote_timeset_state = 0; //remote_timeset_state in device:  0 no timeset, 1 timeset request, 2 timeset process 


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
    Connect to WiFi in STA mode
  ----------------------------------------------------------*/
void connectWiFiModeSTA(){
  int t = 0;
  WiFi.mode(WIFI_STA);
 
  //connection init
  Serial.print("Connecting to...");
  Serial.println(ssid);
  Serial.print("with pass...");
  Serial.println(password);

  while (WiFi.status() != WL_CONNECTED){
    WiFi.begin(ssid.c_str(), password.c_str());
    Serial.print(".");
    updateRGB_LED (true);         // Blink while connecting
    delay(10000);
    if (t++>30) return; // 60s*5=300s=5min
  }

  Serial.println("");
  Serial.println("WiFi connected");
  
}

void connectWiFi_1(){
    while (WiFi.status() != WL_CONNECTED){
        WiFi.begin(ssid.c_str(), password.c_str());
        Serial.print(".");
        updateRGB_LED (true);         // Blink while connecting
        delay(10000);
    }

    // Display a notification that the connection is successful. 
    Serial.println("WiFi Connected"); 
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


/*----------------------------------------------------------
    Connect to AP and WiFi in WIFI_AP_STA and launch web 
  ----------------------------------------------------------*/
void createAPserver(){

  WiFi.mode(WIFI_AP_STA);
 
  //access point 
  Serial.println("");
  Serial.println("Creating Accesspoint");
  WiFi.softAP(AP_ssid, "");
  Serial.print("Default IP address:\t");
  Serial.println(WiFi.softAPIP());

  //station part
  Serial.print("Connecting to...");
  Serial.println(ssid);
  Serial.print("with pass...");
  Serial.println(password);

  WiFi.begin(ssid.c_str(), password.c_str());

  delay(10000);

 
  if(WiFi.status() != WL_CONNECTED){
    Serial.println("");
    Serial.println("WiFi Connection Failed!!");
    Serial.println("Please verify your Network SSID name and password, and router connectivity");
  }
  else{
    Serial.println("");
    Serial.println("WiFi connected");
  }

  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.println("WiFi Status: ");
  Serial.println(WiFi.status());

  generateBasicWebHtml();

  server.on("/", handleConnectionRoot);
  server.on("/instrucciones", handleConnectionInstructions);
  server.on("/configuracion", handleConnectionConfiguration);
  server.on("/enviar_configuracion", handleConnectionSendConfiguration);
  server.on("/enviar_wifi", handleConnectionSendWiFiConfiguration);
  server.on("/verificacion_wifi", handleConnectionVerificationWiFi);
  server.begin();

};


/*----------------------------------------------------------
    Create Basic HTML Content Pattern
  ----------------------------------------------------------*/
void generateBasicWebHtml(){
  
      //HTML CONTENT
      content_fixed_up = "<!DOCTYPE html>";
      content_fixed_up += "<html>";

      // HEAD
      content_fixed_up +="<head>";
      content_fixed_up +="<meta charset=\"UTF-8\">";
      content_fixed_up +="<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">";
      content_fixed_up ="<title>Medidor C02  Configuraci&oacute;n</title>";
      content_fixed_up += "</head>";

      // STYLES
      content_fixed_up ="<style>"; 
      content_fixed_up +="html, body { height: 100%; margin: 0; } ";
      content_fixed_up +="body { background-color: black; color: white; font-family: 'Courier New', Courier, monospace; } ";
      content_fixed_up +="button { border: 1px white solid; color: white; background-color: black; padding: 15px; margin: 5px; font-family: 'Arial'; text-decoration: none; font-size: 13px; width: 135px; cursor: pointer;} ";
      content_fixed_up +="a button  { text-decoration: none; color: white; cursor: pointer; }";
      content_fixed_up +="div#mensajes{ padding: 15px; width: 100%; margin: 20px auto; text-align: center; } ";
      content_fixed_up +="div#mensajes ol { text-align: left; } ";
      content_fixed_up +=".fail { color: red; }";
      content_fixed_up +="a.link { color: orange; text-decoration: none;}";
      content_fixed_up +=".success{ color: green; } ";
      content_fixed_up +=" .yellow { color: rgba(255, 255, 0, 0.678); } ";
      content_fixed_up +="h1.unicolor, footer p { color: #1F7E97; text-align: center; } ";
      content_fixed_up +=".content { margin: 0 auto; width: 85%; min-height: 100%; text-align: center; } ";
      content_fixed_up +="footer { border-top: 1px solid #1F7E97; height: 50px; margin-top: -50px; padding: 0 20px; } ";
      content_fixed_up +="main { padding: 20px; padding-bottom: 50px; } ";
      content_fixed_up +="@media (min-width:1024px){ .content { width: 60%; } div#mensajes { width: 50%; } } ";
      content_fixed_up += "</style>";

      //BODY
      content_fixed_up +="<body>";
      content_fixed_up +="<div class=\"content\">";
      content_fixed_up +="<main>";
      content_fixed_up +="<h1 class=\"unicolor\">Medidor de CO2 UNICEN</h1>";
      content_fixed_up +="<a href=\"/instrucciones\"><button>Instrucciones</button></a>";
      content_fixed_up +="<a href=\"/configuracion\"><button >Configuraci&oacute;n</button></a>";
      content_fixed_up+="<div id=\"mensajes\">";
      content_fixed_up +="<br>";

      content_fixed_down ="</div> ";
      content_fixed_down +="</main>";
      content_fixed_down +="</div>";
      content_fixed_down +="<footer>";
      content_fixed_down +="<p>2021 - Proyecto Abierto Medici&oacute;n CO2 -  <b>UNICEN</b></p>";
      content_fixed_down +="</footer>";
      content_fixed_down +="</body>";
      content_fixed_down +="</html>";
  
}


/*----------------------------------------------------------
    192.168.4.1/ content & functionality
  ----------------------------------------------------------*/
void handleConnectionRoot(){
  Serial.println("Root access in Html Web Page");
  if(WiFi.status() != WL_CONNECTED){
  
      content_dynamic = "<p class=\"fail\">Su dispositivo no posee conexi&oacute;n a una red WiFi</p>";
      content_dynamic += "<p>Por favor lea las Instrucciones y luego configure su conexi&oacute;n dentro de las secci&oacute;n Configuraci&oacute;n</p>";

    Serial.println("No WiFi");
  }
  else{
      content_dynamic = "<p class=\"success\">Su dispositivo est&aacute; conectado a la Red WiFi : ";
      content_dynamic += ssid + "</p>";
      content_dynamic += "<p>Si desea cambiar de red o ingresar nuevas credenciales de Thingspeak, por favor lea las Instrucciones y luego configure su conexi&oacute;n dentro de Configuraci&oacute;n</p>";
    
  };
  content = content_fixed_up + content_dynamic + content_fixed_down;
  server.send(200, "text/html", content);
}


/*----------------------------------------------------------
    192.168.4.1/instrucciones content & functionality
  ----------------------------------------------------------*/
void handleConnectionInstructions(){
  Serial.println("Button Event \"Instrucciones\" in HtmlWeb Page");
    
    content_dynamic = "<h2>Instrucciones</h2>";
    content_dynamic += "<p>Siguiendo los siguientes pasos usted puede configurar su dispositivo para que se conecte a una nueva red WiFi o tambi&eacute;n puede cambiar de red WiFi.</p>";
    content_dynamic += "<ol><li>Ingrese en Configuraci&oacute;n</li>";
    content_dynamic += "<li>Introducza el usuario y contrase&ntilde;a del dispositivo (si no lo posee consulte al administrador o a qui&eacute;n lo suministr&oacute;).</li>";
    content_dynamic += "<li>Si el acceso a su dispostivo fue exitoso, debe seleccionar la red de WiFi que desea conectar el dispositivo y su contrase&ntilde;a. En ese mismo paso puede agregar opcionalemente el canal y credenciales para la plataforma Thingspeak que registra en la nube las mediciones.</li>";
    
    content_dynamic += "<li>Si su red y contrase&ntilde;a de WiFi es correcta su dispositivo queda configurado y conectado a la red seleccionada, luego de 10 segundos se reiniciar&ntilde;. Caso contrario vuelva a intentarlo.</li></ol>";
    content_dynamic += "<p>Para saber m&aacute;s de Thingspeak y su uso puede visitar la informaci&oacute;n de nuestro proyecto <a class=\"link\" href=\"https://github.com/medicionco2/proyecto_medidor_co2/blob/main/src/mhz19/ESP32/IoT/README.md\" target=\"_blank\">aqu&iacute;<a>";
    content_dynamic += " o tambi&eacute;n accediendo a <a class=\"link\" href=\"https://thingspeak.com/\" target=\"_blank\">Thingspeak<a>. Recuerde que si usted est&aacute; viendo esta informaci&oacute;n es porque se ha conectado al dispositivo medidor, para visitar en la web dicha informaci&oacute;n deber&aacute; conectarse nuevamente a su red WiFi con su computadora o m&oacute;vil.</p>";
    content = content_fixed_up + content_dynamic + content_fixed_down;

    if(WiFi.status() != WL_CONNECTED){
      Serial.println("No WiFi");
    }
    else{
      Serial.println("WiFi Connected");
    }

    server.send(200, "text/html", content);
}

/*----------------------------------------------------------
    192.168.4.1/configuracion content & functionality
  ----------------------------------------------------------*/
void handleConnectionConfiguration(){

  Serial.println("Button Event \"Configuracion\" in HtmlWeb Page");  
  
 
    content_dynamic = "<h2>Acceso al Dispositivo</h2>";
    content_dynamic += "<p>Ingrese credenciales del Dispositivo</p>";
    content_dynamic +="<form method='post' action='enviar_configuracion'>";
    content_dynamic += "<label for=\"local_board_user\">Usuario</label><br>";
    content_dynamic += "<input type=\"text\" name=\"local_board_user\"><br><br>";
    content_dynamic += "<label for=\"local_board_pass\">Contrase&ntilde;a</label><br>";
    content_dynamic += "<input type=\"password\" name=\"local_board_pass\"><br><br>";
    content_dynamic += "<button type=\"submit\">Acceder</button>";
    content_dynamic +="</form>";
    content = content_fixed_up + content_dynamic + content_fixed_down;
      
  server.send(200, "text/html", content);
}


/*----------------------------------------------------------
    192.168.4.1/enviar_configruracion functionality
  ----------------------------------------------------------*/
void handleConnectionSendConfiguration(){

   String local_board_user = server.arg("local_board_user");
   String local_board_pass = server.arg("local_board_pass");

   Serial.println("Usario Placa: " + local_board_user);
   Serial.println("Pass Placa: " + local_board_pass);

   if(local_board_pass == local_pass_esp32 && local_board_user == local_user_esp32){
    scanListNetworks();
    content_dynamic = "<h2>Configuraci&oacute;n WiFi Dispositivo</h2>";
    content_dynamic += "<p class='success'>Acceso Exitoso al dispositivo</p>";    
    content_dynamic += "<h3>Redes Disponibles</h3>";
    content_dynamic += netListHtml;

    content_dynamic +="<form method='post' action='enviar_wifi'>";
    content_dynamic +="<label for=\"wifi_name\">N&utilde;mero de su Red</label><br>";
    content_dynamic +="<input type=\"number\" min=\"1\" name=\"wifi_number\"><br><br>";
    content_dynamic +="<label for=\"wifi_pass\">Contrase&ntilde;a</label><br>";
    content_dynamic +="<input type=\"password\" name=\"wifi_pass\" length=64><br><br>";
    content_dynamic +="<label for=\"ts_ch_id\">ThingSpeak Canal ID <span class=\"yellow\">*</span></label><br>";
    content_dynamic +="<input type=\"text\" name=\"ts_ch_id\" length=8><br><br>";
    content_dynamic +="<label for=\"read_api_key\">ThingSpeak Read API Key <span class=\"yellow\">*</span></label><br>";
    content_dynamic +="<input type=\"text\" name=\"read_api_key\" length=32><br><br>";
    content_dynamic +="<label for=\"write_api_key\">ThingSpeak Write API Key <span class=\"yellow\">*</span></label><br>";
    content_dynamic +="<input type=\"text\" name=\"write_api_key\" length=32><br>";
    content_dynamic +="<p class=\"yellow\">* Opcionales</p><br>";
    content_dynamic +="<button type=\"submit\">Enviar</button>";
    content_dynamic +="</form>";
  
  }
  else {
    content_dynamic = "<p class='fail'>Acceso Denegado</p>";
    content_dynamic += "<p class='fail'>Verifique usuario y/o contrase&ntilde;a de su dispositivo</p>";
    content_dynamic += "<p>Para volver a intentar vuelva a ingresar en Configuraci&oacute;n</p>";
  }

  Serial.println("Button Event \"enviar_configuracion\" in HtmlWeb Page");  
  
  content = content_fixed_up + content_dynamic + content_fixed_down;
      
  server.send(200, "text/html", content);
}


/*----------------------------------------------------------
    192.168.4.1/enviar_configruracion content & functionality
  ----------------------------------------------------------*/
void handleConnectionSendWiFiConfiguration(){

   String sid_number = server.arg("wifi_number");
   int qsid_number = sid_number.toInt();
   String qsid = WiFi.SSID(qsid_number-1);
   String qpass = server.arg("wifi_pass");
   String qchannel_id = server.arg("ts_ch_id");
   String qwrite_api_key = server.arg("write_api_key");
   String qread_api_key = server.arg("read_api_key");
 
   
   Serial.println("Nombre de Red: " + qsid);
   Serial.println("Pass WiFi: " + qpass);
   Serial.println("Channel ID: " + qchannel_id);
   Serial.println("Read Api Key: " + qread_api_key);
   Serial.println("Write Api Key: " + qwrite_api_key);

   writeEEPROM(qsid, qpass, qchannel_id, qwrite_api_key, qread_api_key);

   Serial.println("Button Event \"enviar_configuracion_wifi\" in HtmlWeb Page");  

   WiFi.begin(qsid.c_str(), qpass.c_str());

   delay(5000);

   handleConnectionVerificationWiFi();

}

/*----------------------------------------------------------
    192.168.4.1/verificacion_wifi content & functionality
  ----------------------------------------------------------*/
void handleConnectionVerificationWiFi(){

  if(WiFi.status() != WL_CONNECTED){
    content_dynamic = "<p class='fail'>Su dispositivo medidor NO se ha podido conectar a su Red WiFi</p>";   
    content_dynamic += "<p class='fail'>Por favor verifque la contrase&ntilde;a y el estado de su router.</p>"; 
    content_dynamic += "<p class='fail'>Vuelva a intentarlo nuevamente ingresando a Configuraci&oacute;n</p>"; 
    content = content_fixed_up + content_dynamic + content_fixed_down;
    server.send(200, "text/html", content); 
  }
  else{
    content_dynamic = "<p class='success'>Su dispositivo medidor se ha conectado a su Red WiFi.</p>"; 
    content_dynamic += "<p>En 10 segundos se reiniciar&aacute; el dispostivo y se conectar&aacute; autom&aacute;ticamente a la red configurada.</p>"; 
    content = content_fixed_up + content_dynamic + content_fixed_down;
    server.send(200, "text/html", content);  
    delay(10000);
    WiFi.mode(WIFI_STA);
    Serial.println("Ingreso en modo STA");
    Serial.println("Se reiniciará ESP32");
    ESP.restart();
  };

}

/*----------------------------------------------------------
    Network Scan and network list generation
  ----------------------------------------------------------*/
void scanListNetworks(void){
  int n = WiFi.scanNetworks();
  Serial.println("scan done");
  if (n == 0)
    Serial.println("no networks found");
  else
  {
    Serial.print(n);
    Serial.println(" networks found");
    for (int i = 0; i < n; ++i)
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
   totalNets = n;
   
  netListHtml = "<ol class=\"center\">";
  for (int i = 0; i < n; ++i)
  {
    // Print SSID and RSSI for each network found
    netListHtml += "<li>";
    netListHtml += WiFi.SSID(i);
    netListHtml += " (";
    netListHtml += WiFi.RSSI(i);
    netListHtml += ")";
    netListHtml += "</li>";
  }
  netListHtml += "</ol>";
}


/*----------------------------------------------------------
    MH-Z19-based CO2 meter setup
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

  //WiFi.disconnect(); commented v3

  EEPROM.begin(512); //Initialasing EEPROM
  delay(10);
  updateRGB_LED (true);         // Blink while connecting
  if (readEEPROM()) // Read params from EEPROM, if present
    connectWiFi_1();
  else {
    Serial.println ("Changing to Access Point mode..."); 
    createAPserver();    
    AP_MODE = true;
  }
  updateRGB_LED (false);

  // Init and get the time
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  printLocalTime();

  ThingSpeak.begin(myClient);

  Serial.println ("Warming will start during 3 minutes..."); 
  delay (180000); // Wait 3 minutes for warming purposes
  //delay (20000); // Wait 20 secondos for debug purposes

  retrieveInfo_mhz19 ();

  pinMode(button1.PIN, INPUT_PULLUP);
  attachInterrupt(button1.PIN, isr_button, CHANGE);
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
        createAPserver();    
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
    Remote Calibration
  ----------------------------------------------------------*/

void remoteCalibration() {
  
  remote_calibrate_state = ThingSpeak.readFloatField( myChannelNumber, remote_calibrate_field); //Read remote State of Calibration

  Serial.println("Calibrate State in Device Readed in Remote");
  Serial.println(remote_calibrate_state);

  //Remote Calibrate Request
  if(remote_calibrate_state==1){
    Serial.println("Remote Calibrate Request Detected!!");
   
    remote_calibrate_state = 2;
    delay(20000); // Refresh rate in thingspeak for free registration is 15 seconds
    Serial.println("Calibrating State is");
    Serial.println(remote_calibrate_state);

    int writeApiResponse = ThingSpeak.writeField(myChannelNumber,remote_calibrate_field, remote_calibrate_state, myWriteAPIKey.c_str());
    
      if(writeApiResponse == 200){
        Serial.println("Channel update successfully with remote_calibrate_state = " + String(remote_calibrate_state));
        Serial.println("Starting calibration during 20 minutes aprox....");
        calibrate_mhz19();
        //delay(120000); // discomment for debugging puposes and comment calibrate_mhsz19()
        remote_calibrate_state=0;

        delay(20000); // Refresh rate in thingspeak for free registration is 15 seconds

        writeApiResponse = ThingSpeak.writeField(myChannelNumber,remote_calibrate_field, remote_calibrate_state, myWriteAPIKey.c_str());
    
          if(writeApiResponse == 200){
            Serial.println("Channel update successfully with remote_calibrate_state = " + String(remote_calibrate_state));
          }
          else{
            Serial.println("Problem updating channel with calibration ending confirmation. HTTP error code " + String(writeApiResponse));
          }
      }
      else {
      Serial.println("Problem updating channel with calibration confirmation. HTTP error code " + String(writeApiResponse));
    }

  }

}

/*----------------------------------------------------------
    Remote Restart
  ----------------------------------------------------------*/
void remoteRestart() {

  remote_restart_state = ThingSpeak.readFloatField( myChannelNumber, remote_restart_field); //Read remote Restart state

  Serial.println("Restart State in Device Readed in Remote");
  Serial.println(remote_restart_state);

  //Remote Restart Request
  if(remote_restart_state==1){
    Serial.println("Remote Restart Request Detected!!");
   
    remote_restart_state = 2;
    delay(20000); // Refresh rate in thingspeak for free registration is 15 seconds
    Serial.println("Restart State is");
    Serial.println(remote_restart_state);

    int writeApiResponse = ThingSpeak.writeField(myChannelNumber,remote_restart_field, remote_restart_state, myWriteAPIKey.c_str());
    
      if(writeApiResponse == 200){
        Serial.println("Channel update successfully with remote_restart_state = " + String(remote_restart_state));
        delay(40000); 
        remote_restart_state=0;

        delay(20000); // Refresh rate in thingspeak for free registration is 15 seconds

        writeApiResponse = ThingSpeak.writeField(myChannelNumber,remote_restart_field, remote_restart_state, myWriteAPIKey.c_str());
    
          if(writeApiResponse == 200){
            Serial.println("Channel update successfully with remote_restart_state = " + String(remote_restart_state));
            delay(20000); // Refresh rate in thingspeak for free registration is 15 seconds
            Serial.println("Restart begins...");
            delay(5000);
            ESP.restart();
          }
          else{
            Serial.println("Problem updating channel with restart ending confirmation. HTTP error code " + String(writeApiResponse));
          }
      }
      else {
      Serial.println("Problem updating channel with calibration confirmation. HTTP error code " + String(writeApiResponse));
    }

  }

}

/*----------------------------------------------------------
    Remote Time Set
  ----------------------------------------------------------*/

void remoteTimeSet() {
  
  remote_timeset_state = ThingSpeak.readFloatField( myChannelNumber, remote_timeset_field); //Read remote State of Time Set

  Serial.println("Time Set State in Device Readed in Remote");
  Serial.println(remote_timeset_state);

  //Remote Time Set Request
  if(remote_timeset_state==1){
    Serial.println("Remote Time Set Request Detected!!");
   
    remote_timeset_state = 2;
    delay(20000); // Refresh rate in thingspeak for free registration is 15 seconds
    Serial.println("Time Set State is");
    Serial.println(remote_timeset_state);

    int writeApiResponse = ThingSpeak.writeField(myChannelNumber,remote_timeset_field, remote_timeset_state, myWriteAPIKey.c_str());
    
    if(writeApiResponse == 200){
      Serial.println("Channel update successfully with remote_timeset_state = " + String(remote_timeset_state));
      Serial.println("Starting time set process....");
      delay(120000); 

      // Time adjusting function HERE

        
      remote_timeset_state=0;

      delay(20000); // Refresh rate in thingspeak for free registration is 15 seconds

      writeApiResponse = ThingSpeak.writeField(myChannelNumber,remote_timeset_field, remote_timeset_state, myWriteAPIKey.c_str());
    
      if(writeApiResponse == 200){
        Serial.println("Channel update successfully with remote_timeset_state = " + String(remote_timeset_state));
      }
      else{
            Serial.println("Problem updating channel with timeset ending confirmation. HTTP error code " + String(writeApiResponse));
          }
      }
   else {
      Serial.println("Problem updating channel with timeset confirmation. HTTP error code " + String(writeApiResponse));
  }

  }

}


/*----------------------------------------------------------
    MH-Z19 CO2 sensor loop
  ----------------------------------------------------------*/
void loop() {

  int co2ppm = mhz19.getCO2();          // Request CO2 (as ppm)
  int temp = mhz19.getTemperature();    // Request Temperature (as Celsius)
  struct tm timeinfo;  
  char buffer [25];

  btnManager_prov (co2ppm);

  remoteCalibration();

  remoteRestart();

  remoteTimeSet();


  if (!AP_MODE){
    // Measurements to computer for debugging purposes
    //
    //Serial.print("co2: ");
    Serial.print(co2ppm);
    //Serial.print("temp: ");
    Serial.print(",");
    Serial.println(temp);

    if (d%S==0){     // Telemetry is sent every S samplingPeriod seconds

   if (WiFi.status() != WL_CONNECTED)  connectWiFiModeSTA();
   
   if (WiFi.status() == WL_CONNECTED) {
      // Measurements to WiFi thingspeak IoT Platform 
      // 

      ThingSpeak.setField (1,co2ppm);
      ThingSpeak.setField (2,temp);
      ThingSpeak.setField (remote_calibrate_field,remote_calibrate_state); // remote calibrate field
      ThingSpeak.setField (remote_restart_field,remote_restart_state); // remote restart field
      ThingSpeak.setField (remote_timeset_field,remote_timeset_state); // remote timeset field

      ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey.c_str());
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

      ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey.c_str());
      Serial.print("Datos guardados anteriormente enviados a ThingSpeak: "); Serial.println(buffer);
      if (saved2send > 0) saved2send--;
     }
  }
    d++;

    CO2RGB_LED(co2ppm);

    delay(samplingPeriod);
  } 
  
  else { // Meter in Access Point mode
    server.handleClient();
  }


}
