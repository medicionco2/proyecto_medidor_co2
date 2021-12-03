/*
 * 
 * Program to test new Web Configuration for WiFi Provisioning
 * ESP32  wil try to connect to a WiFi net until control button is pressed
 * When a button event occurs ESP32 turns into AP and STA mode and allows the users to connect to esp32 Access Point network (Default is: http://192.168.4.1/) to configure networks or Thingspeak parameters
 * Local Credential of device will be required in Web Access, please verify values of local_user_esp32 and local_pass_esp32 
 * If local device credencial are verified, then you can configure WiFi Connection and ThingSpeak credentials..
 * If WiFi parameters are OK then the device will restart in 10 secondos, otherwise a correct WiFi connection parametres will be asked
 * ESP32 can be configured in two cases, when the button is pressed:
 *  a. When a WiFi connection is correct but you need to change WiFi Net or you may need to change Thingspeak access crendential.
 *  b. When a WiFi connection is not running and you need to change WiFi Net.
*/

#include <WiFi.h>
#include <WebServer.h>
#include <EEPROM.h>

const char* AP_ssid = "esp32";
const char* AP_pass = "";

const char* local_user_esp32 ="esp32";
const char* local_pass_esp32 ="4321";

//HTML CONTENT
String content;
String content_fixed_up;
String content_dynamic;
String content_fixed_down;
String netListHtml;

int totalNets;

//EEPROM read
String esid = "";
String epass = "";

String echannel_id;
String ewrite_api_key;
String eread_api_key;

// Button PIN
const int btn_PIN = 21;

bool serverFlag = false;


//Establishing Local server at port 80
WebServer server(80);

void setup(){
  pinMode(btn_PIN, INPUT_PULLUP);
  Serial.begin(9600);
  
  //Initialasing and read EEPROM
  readEEPROM();
  connectWiFiModeSTA();

}

/*----------------------------------------------------------
    Read meter parameters to EEPROM
  ----------------------------------------------------------*/

void readEEPROM() {

/* EEPROM READ */
  EEPROM.begin(512); //Initialasing EEPROM
  delay(10);

  Serial.println("Reading EEPROM ssid");
  
  for (int i = 0; i < 32; ++i)
  {
    esid += char(EEPROM.read(i));
  }

  Serial.println();
  Serial.print("SSID: ");
  Serial.println(esid);
  
  Serial.println("Reading EEPROM pass");
  
  for (int i = 32; i < 96; ++i)
  {
    epass += char(EEPROM.read(i));
  }

  Serial.print("PASS: ");
  Serial.println(epass);

  Serial.println("Reading EEPROM Channel ID");
  
  for (int i = 96; i < 104; ++i)
  {
    echannel_id += char(EEPROM.read(i));
  }

  Serial.print("Channel ID: ");
  Serial.println(echannel_id);

  Serial.println("Reading EEPROM Write API Key");
  
  for (int i = 104; i < 136; ++i)
  {
    ewrite_api_key += char(EEPROM.read(i));
  }

  Serial.print("Write APi Key: ");
  Serial.println(ewrite_api_key);

  Serial.println("Reading EEPROM Read API Key");
  
  for (int i = 136; i < 168; ++i)
  {
    eread_api_key += char(EEPROM.read(i));
  }

  Serial.print("Read APi Key: ");
  Serial.println(eread_api_key);

  
  }


/*----------------------------------------------------------
    Write configuration to EEPROM
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
    Connect to WiFi in STA mode
  ----------------------------------------------------------*/
void connectWiFiModeSTA(){

   WiFi.mode(WIFI_STA);
 
  //connection init
  Serial.print("Connecting to...");
  Serial.println(esid);
  Serial.print("with pass...");
  Serial.println(epass);

  WiFi.begin(esid.c_str(), epass.c_str());

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
  Serial.println(esid);
  Serial.print("with pass...");
  Serial.println(epass);

  WiFi.begin(esid.c_str(), epass.c_str());

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
       content_dynamic += esid + "</p>";
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

    Serial.println("No WiFi");
 
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


void loop(){
  
  while(WiFi.status() != WL_CONNECTED && digitalRead(btn_PIN) != 0){
    Serial.println("WiFi Connection Failed!!");
    Serial.println("...Trying to connect");
    Serial.println("Press Button during some secons to pass to access point mode");
    
    connectWiFiModeSTA();
  }
  
  if (digitalRead(btn_PIN) == 0)
   {
    Serial.println("Button Pressed");
    createAPserver();
    serverFlag = true;
    Serial.println("Turning the device into AP");
  }
 

   while (serverFlag == true )
   {
    Serial.println("Access Point Activated. Please configure your Wifi Network and Thingspeak credentials connecting to esp32 WiFi network and typing 192.168.4.1 in your web browser"); 
    server.handleClient();
   }

   Serial.println("Connected to WiFi Net : " + esid);

}
