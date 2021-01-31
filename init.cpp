#include <WiFi.h>
#include <WiFiClient.h>
#include <WiFiServer.h>
#include <WiFiUdp.h>
#include "init.h"

/*
  Declaracion variables para canales SPI y el reloj de ambos canales de SPI.
  VSPI inicializa los pines del SPI A en ESP32
  HSPI inicializa los pines del SPI C en ESP32
*/
const int spiClk = 5000000;
SPIClass * spiA = NULL;
SPIClass * spiC = NULL;
void SPI_configuracion() {
  spiA = new SPIClass(VSPI);
  spiA->begin();
  pinMode(5, OUTPUT);
  spiC = new SPIClass(HSPI);
  spiC->begin(26, 12, 13, 15);
  pinMode(15, OUTPUT);
}

/*
  Creación de servidor red en la ESP32 con nombre CP_WalkerAP y contraseña password. Se puede modificar en cualquier momento.
  En ella se inicializan aquellos puertos en los que vamos a recibir informacion desde la app.
*/
const char* ssid = "CPWalker";
const char* password = "cpwalker";

IPAddress local_IP(192, 168, 4, 2);
IPAddress gateway(192, 168, 4, 1);
IPAddress subnet(255, 255, 255, 0);



WiFiUDP udp50012,udp50015, udp50011, udp50016, udpCheckConn, udp1, udp2, udp3, udp4, udp5;
boolean conectado = false;
void init_LEDS(){
  ledcSetup(channel, freq, res);
  ledcSetup(channel1, freq, res);
  ledcSetup(channel2, freq, res);
  
  ledcAttachPin(r, channel);
  ledcAttachPin(g, channel1);
  ledcAttachPin(b, channel2);
}
void WIFI_configuracion() {
  
  Serial.begin(115200);
  // Configurar IP estática
  if (!WiFi.config(local_IP, gateway, subnet)) {
    Serial.println("STA Failed to configure");
  }
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED)
    {    delay(1000);
         Serial.print(".");
    }
  Serial.println("WiFi connected..!");
  Serial.print("Got IP: ");  
  Serial.println(WiFi.localIP());

  //WiFi.softAP(ssid, password);
  //IPAddress IP = WiFi.softAPIP();
  //Serial.print("AP IP address: ");
  //Serial.println(IP);
  //Serial.println(WiFi.softAPIP());
  ledcWrite(channel, pow(2,16) * porcentaje);
  //while (WiFi.softAPgetStationNum() == 0) {
  //}
  ledcWrite(channel, pow(2,16) * porcentaje);
  ledcWrite(channel1, pow(2,16) * porcentaje * 0.35);
  conectado = true;
  udp50015.begin(50015);
  udp50012.begin(50012);
  udp50011.begin(50011);
  udp50016.begin(50016);
  udpCheckConn.begin(9999);
}
