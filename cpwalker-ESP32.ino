#include "init.h"
#include "communication.h"

/*
  WIFI_configuracion es una funcion declarada en init.cpp en la que se declara la sparkfun como router para establecer comunicación desde la aplicacion
  a la ESP32 sin necesidad de una red externa
  SPI_configuracion es una funcion declarada en  init.cpp en la que se inicializan los 2 canales disponibles de la ESP32 para comunicacion ESP32 y texas
  conexion es el gpio 0 de ls ESP32 declarado en init.cpp
  SPI_check es el gpio 2 de la ESP32 declarado en init.cpp
*/
int cont = 0, check = 0;

void setup() {
  setCpuFrequencyMhz(40);
  init_LEDS();
  WIFI_configuracion();
  SPI_configuracion();
  Wire.begin(16, 17);
  pinMode(TI_RST, OUTPUT);
  digitalWrite(TI_RST, LOW);
}
/*
  Mediante pruebas se detectó que reseteando TI la comunicación era más robusta.
  Comprobamos que exista algun cliente conectado al ESP32, si hay alguno el led estara encendido en otro caso apagado
  esperar_paquete_Udp es una funcion declarada en comunication.cpp la cual recibe la informacion desde la app
  I2C funcion declarada en communication.cpp que recibe datos por el canal I2C de la ESP32
  resetFunc es para resetear la ESP32 cada vez que salimos de la aplicacion de otra forma, tras salir de la app no funciona correctamente la comunicación.
*/
void (*resetFunc)(void) = 0;
void loop() {
  
  if (cont == 0) {
    digitalWrite(TI_RST, HIGH);
    delay(50);
    digitalWrite(TI_RST, LOW);
    delay(50);
    cont++;
  }
  /*Comprobar si hay algun paquete de udp con "parsePacket()"*/
  check = udpCheckConn.parsePacket();
  Serial.printf("Check: %d", check);
  if (check) {
    Serial.printf("\nReceive from port 9999\n");
    char app = udpCheckConn.read();
    int rec = app;
    Serial.printf("Received: %d", rec);
    if (rec == 255 ) {
        resetFunc();
      Serial.printf("\nresetFunction()\n");
    }
    else{
      ledcWrite(channel, 0);
      ledcWrite(channel1, pow(2,16) * porcentaje);
    }
  }
  if (conectado) {
    esperar_paquete_Udp();
    I2C();
  }
}
