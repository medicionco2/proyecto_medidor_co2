/* 
INSTRUCCIONES
- Durante el boot ESP32 ejecuta el setup y lee los valores de SSID y contraseña si ya existiesen en la EEPROM e intenta la conexión WIFI con dichas credenciales
- Si este proceso falla o no existe una previa configuración de red WiFi, ESP32 se configura como un punto de acceso (AP - Access Point) y crea una red abierta Wifi sin proteccion y sin contraseña, para nuestro caso la red será "redmedidor"
- Esta red abierta WiFI permite al usuario conectarse a través de un navegador 
- Para establecer la conexion con el punto de acceso, desde el dispostivo que se desee conectar debe seleccionar la red en este caso con el nombre "redmedidor", luego debe ingresarse la dirección IP 192.168.4.1 en el navegador para acceder a
la página web que permite ver las redes disponibles e ingresar cual desea conectar y dejar establecida con nombre y contraseña
- Una vez ajustado el nombre de red y contraseña, ESP32 se reinicia e intenta conectarse a la red asignada
- Si se establece la conexion el proceso se ejecutó exitosamente.
- Si la conexión falla, ESP32 vuelve a configurarse como punto de acceso esperando una nueva configuración.
 - En caso que se desee modificar la red a la cual se conecta el ESP32 se debe pulsar el botón para que ingrese en el modo de punto de acceso. Recordar que una vez ingresada una red donde la conexión se establece con exito
 dicha red será a la que por defecto se conectará porque ya sus credenciales estan guardadas en EEPROM. El unico modo de evitar esa conexion automática es oprimiendo el boton. 
*/

#include <WiFi.h>
#include <HTTPClient.h>
#include <WebServer.h>
#include <EEPROM.h>

//Variables
int i = 0;
int statusCode;
const char* ssid = "redmedidor";
const char* passphrase = "";
String st;
String content;
String esid;
String epass = "";

// Const
const int btn_PIN = 21;


//Function Declarations
bool testWifi(void);
void launchWeb(void);
void setupAP(void);

//Establishing Local server at port 80
WebServer server(80);


#define DEBUG //comment this line to avoid all messages in serial

void setup()
{

  #ifdef DEBUG
  Serial.begin(115200); //Initialising if(DEBUG)Serial Monitor
  #endif
  
  Serial.println();
  Serial.println("Disconnecting current wifi connection");
  
  WiFi.disconnect();
  EEPROM.begin(512); //Initialasing EEPROM
  delay(10);

  pinMode(btn_PIN, INPUT_PULLUP);

  Serial.println();
  Serial.println();
  Serial.println("Startup");
  

  //---------------------------------------- Read eeprom for ssid and pass

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


  WiFi.begin(esid.c_str(), epass.c_str());
}
void loop() {

  if ((WiFi.status() == WL_CONNECTED))
  {

    for (int i = 0; i < 10; i++)
    {
      Serial.print("Connected to ");
      Serial.print(esid);
      Serial.println(" Successfully");
      delay(100);
    }

  }
  else
  {
  }

  
  if (testWifi() && (digitalRead(btn_PIN) != 0))
  {
    Serial.println(" connection status positive");
    return;
  }
  else
  {
   
    Serial.println("Connection Status Negative / D21 LOW - Button Pressed");
    Serial.println("Turning the HotSpot On");
    launchWeb();
    setupAP();// Setup HotSpot Access Point
  }

  Serial.println();
  Serial.println("Waiting.");

  while ((WiFi.status() != WL_CONNECTED))
  {
    Serial.print(".");
    delay(1000);
    Serial.print(".");
    delay(1000);
    Serial.print(".");
    delay(1000);
    Serial.print(" Access Point Activated, waiting for connections. Configure your Wifi Network connecting to redmedidor and typing 192.168.4.1 in your web browser \n"); //Guide to user
    delay(4000);
    server.handleClient();
  }
  delay(1000);
}


//----------------------------------------------- Fuctions used for WiFi credentials saving and connecting to it which you do not need to change
bool testWifi(void)
{
  int c = 0;
  
  while ( c < 20 ) {
    if (WiFi.status() == WL_CONNECTED)
    {
      return true;
    }
    delay(500);
    Serial.print("*");
    c++;
  }
  Serial.println("");
  Serial.println("Connect timed out, opening AP");
  return false;
}

void launchWeb()
{
  Serial.println("");
  if (WiFi.status() == WL_CONNECTED)
    Serial.println("WiFi connected");
  Serial.print("Local IP: ");
  Serial.println(WiFi.localIP());
  Serial.print("SoftAP IP: ");
  Serial.println(WiFi.softAPIP());
  createWebServer();
  // Start the server
  server.begin();
  Serial.println("Server started");
}

void setupAP(void)
{
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);
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
  Serial.println("");
  st = "<ol class=\"center\">";
  for (int i = 0; i < n; ++i)
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
  
  WiFi.softAP("redmedidor", "");
  Serial.println("Initializing_softap_for_wifi credentials_modification");
  launchWeb();
  Serial.println("over");
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
      content +="<title>Wi Fi Porvisioning UNCPBA - CO2 </title>";

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
      content +=" <h2>Redes Disponibles</h2>";
      content += "<p>";
      content += st;
      content += "</p>";

      content +="<form class=\"red\" method='get' action='setting'>";
      content +="<h3>Datos de red para el dispositivo</h3>";
      content +="<hr>";
      content +="<br>";
      content +="<label>SSID - Nombre de Red: </label>";
      content +="<br>";
      content +="<input class=\"input_text\" name='ssid' length=32>";
      content +="<br>";
      content +="<label>Pass: </label>";
      content +="<br>";
      content +="<input class=\"input_text\" name='pass' length=64  type=\"password\">";
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
      String qsid = server.arg("ssid");
      String qpass = server.arg("pass");
      if (qsid.length() > 0 && qpass.length() > 0) {
        Serial.println("clearing eeprom");
        for (int i = 0; i < 96; ++i) {
          EEPROM.write(i, 0);
        }
        Serial.println(qsid);
        Serial.println("");
        Serial.println(qpass);
        Serial.println("");

        Serial.println("writing eeprom ssid:");
        for (int i = 0; i < qsid.length(); ++i)
        {
          EEPROM.write(i, qsid[i]);
          Serial.print("Wrote: ");
          Serial.println(qsid[i]);
        }
        Serial.println("writing eeprom pass:");
        for (int i = 0; i < qpass.length(); ++i)
        {
          EEPROM.write(32 + i, qpass[i]);
          Serial.print("Wrote: ");
          Serial.println(qpass[i]);
        }
        EEPROM.commit();

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
