#include <WiFi.h>
#include <WiFiUdp.h>
#include <SPI.h>
#include <Wire.h>
#define conexion 27
#define TI_RST 2
#define r 22
#define g 32
#define b 21
#define freq 5000
#define channel 0
#define channel1 1
#define channel2 2
#define res 16  
#define porcentaje 45 / 100

//variables
extern const char *ssid;
extern const char *password;
extern const int spiClk;
extern SPIClass * spiA;
extern SPIClass * spiC;
extern boolean conectado;
extern WiFiUDP udp50012,udp50015, udp50011,udp50016, udp50017, udpCheckConn, udp; // Objetos WifiUDP, envia y recive datos UDP. En placas basadas en AVR, los paquetes de salida estan limitados a 72 bytes

void SPI_configuracion(void);
void WIFI_configuracion(void);
void init_LEDS(void);
