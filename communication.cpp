#include "communication.h"
#include <string.h>
#include <stdlib.h>


unsigned int count_sended_msgs = 0; 

/*
  Union con 2 tpios de datos declarados, 32 bytes y 8 floats
*/
union byte2single_traction {
  byte bytes[32];
  float floats[8];
};
typedef union byte2single_traction byte2single_traction;
byte2single_traction buf_trac1;

/*
  Funcion en la cual si se recibe informacion por alguno de los puertos de internet contemplados lee lo que hay en ese puerto. Para saber si hay
  informacion en un puerto se utiliza parsePacket().
*/
void esperar_paquete_Udp() {
  int packetSize50011 = udp50011.parsePacket();
  int packetSize50012 = udp50012.parsePacket();
  int packetSize50015 = udp50015.parsePacket();
  int packetSize50016 = udp50016.parsePacket();
  if (packetSize50011) {
    read_UDP(50011, packetSize50011);
    Serial.printf("\nReceived %d bytes from %s, port %d\n", packetSize50011, udp50011.remoteIP().toString().c_str(), udp50011.remotePort());
  }
  if (packetSize50012) {
    read_UDP(50012, packetSize50012);
    Serial.printf("\nReceived %d bytes from %s, port %d\n", packetSize50012, udp50012.remoteIP().toString().c_str(), udp50012.remotePort());
  }
  if (packetSize50015) {
    read_UDP(50015, packetSize50015);
    Serial.printf("\nReceived %d bytes from %s, port %d\n", packetSize50015, udp50015.remoteIP().toString().c_str(), udp50015.remotePort());
  }
  if (packetSize50016) {
    read_UDP(50016, packetSize50016);
    Serial.printf("\nReceived %d bytes from %s, port %d\n", packetSize50016, udp50016.remoteIP().toString().c_str(), udp50016.remotePort());
  }
}

/*
  Funcion en la cual leemos lo que se transmite desde la app y almacenamos los datos como bytes. Dependiendo del puerto que envie la información
  después transmitiremos dicha informacion por el canal spi correspondiente
*/
void read_UDP(int port, int packetSize)
{
  int i;
  int read_size = 70;
  switch (port) {
    case 50011:
      udp50011.read(&buf_trac1.bytes[0], read_size);
      enviar_en_spiC50011(buf_trac1.bytes, packetSize);
      break;
    case 50012:
      udp50012.read(&buf_trac1.bytes[0], read_size);
      enviar_en_spiA(buf_trac1.bytes, packetSize,uint16_t(50012));
      break;
    case 50015:
      udp50015.read(&buf_trac1.bytes[0], read_size);
      enviar_en_spiA(buf_trac1.bytes, packetSize,uint16_t(50015));
      break;
    case 50016:
      udp50016.read(&buf_trac1.bytes[0], read_size);
      enviar_en_spiA(buf_trac1.bytes, packetSize,uint16_t(50016));
    break;

  }
}

/*
  Funciones que transmiten la informacion de la app que llega al puerto correspondiente y la envian por el SPI correspondiente.
  Por el puerto 50011 llegan 8 datos utiles los cuales son transmitidos, por el 50012 llegan 4.
  Dado que el SPI de la texas recibe informacion en FALLING state antes de iniciar cualquier comunicacion hemos de poner el CS del SPI (ver en la documentacion
  que GPIOS son el CS de SPI A y SPI C)a LOW.
  Se transmiten primero 16 0 para eliminar toda la informacion que estuviese almacenada en las funciones de lectura de SPI de la texas. Después enviamos
  los datos de interés.
  Es muy importante establecer que la comunicacion por SPI tenga una frecuencia de 8MHz (spiCLK), MSBFIRST indica una transmisión de datos littleendian
  y SPI_MODE0 indica que el estado la polaridad del reloj y el CS se encuentren a 0
*/
void enviar_en_spiC50011(byte* data_wifi, int len) {
  uint16_t port_C = 50011;
  Serial.printf("\nspiC a mandar %c\n", data_wifi[0]);
  for (int i = 0; i < 8; i++) {
    Serial.printf("\n data wifiC[%i]=%x", i, data_wifi[i]);
  }
  spiC->beginTransaction(SPISettings(spiClk, MSBFIRST, SPI_MODE0));
  digitalWrite(15, LOW);
  for (int i = 0; i < 16; i++) {
    spiC->transfer16(0);
  }
  spiC->endTransaction();
  digitalWrite(15, HIGH);

  spiC->beginTransaction(SPISettings(spiClk, MSBFIRST, SPI_MODE0));
  digitalWrite(15, LOW);
  spiC->transfer16(port_C);
  for (int i = 0; i < 8; i++) {
    spiC->transfer16((uint16_t)data_wifi[i]);
  }
  spiC->endTransaction();
  digitalWrite(15, HIGH);
}

void enviar_en_spiA(byte* data_wifi, int len, uint16_t port_A) {
  Serial.printf("\nspiA a mandar %c a puerto %i\n", data_wifi[0],port_A);
  for (int i = 0; i < 4; i++) {
    Serial.printf("\n data wifiA[%i]=%x", i, data_wifi[i]);
  }
  spiA->beginTransaction(SPISettings(spiClk, MSBFIRST, SPI_MODE0));   // spiA_A
  digitalWrite(5, LOW);
  for (int i = 0; i < 16; i++) {
    spiA->transfer16(0);
  }
  spiA->endTransaction();
  digitalWrite(5, HIGH);

  spiA->beginTransaction(SPISettings(spiClk, MSBFIRST, SPI_MODE0));   // spiA_A
  digitalWrite(5, LOW);
  spiA->transfer16(port_A);
  for (int i = 0; i < 4; i++) {
      spiA->transfer16((uint16_t)data_wifi[i]);
  }
  spiA->endTransaction();
  digitalWrite(5, HIGH);
}

/*
  Funcion para recibir datos de TI a través de comunicación I2C, siguiendo el tutorial de la web de arduino.
  SDA pin 16, SCL pin 17, declarados en Wire.begin(16,17) en setup()
  Los datos recibidos son enviados a la aplicación a traves de paquetes UDP.
  Cada puerto contiene la información relativa a una articulación:10001-LK,10002-RK,10003-LH,10004-RH
*/
int cont1 = 0;
int i = 0;
int j[3];
int k = 0;
byte income[70]; // Datos de las articulaciones, obtenidos por la TI. [35bytes + 35bytes] 
byte enviar[4], enviar1[4], enviar2[4], enviar3[4], enviar4[4], enviar5[4], enviar6[4], enviar7[4],can,pos,enviar8[4];
void I2C() {

  // Almacenar los datos procedentes del i2c mientras esten disponibles en variable "income"
  // [cabecera, cabecera, entero1, decimal1, separador, entero2 , decimal2, separador, ... ]
  cont1 = 0;
  Wire.requestFrom(1, 70);
  while (Wire.available()) {
    byte datos = Wire.read();
    income[cont1] = datos;
    cont1++;
  }

  // Busca cabecera del mensaje [255, 255, 255, 255, ... data ... ] y guarda sus posiciones. 
  // Desde TI los datos se envian en formato de 16 bits, en ESP32 se reciben como dos datos de 8 bits.
  // El formato de mensaje que recibe de la TI es el siguiente:
  // [cabecera, cabecera, entero1, decimal1, separador, entero2 , decimal2, separador, ... ]
  cont1 = 0;
  for (i = 0; i < 70; i++) {
    if (income[i] == 255 && income[(i + 1)%70] == 255) {
      j[cont1] = i;
      cont1++;
    }
    if (cont1 == 3) {
      break;
    }
  }

  // Busca la posición del ultimo dato de cabecera para empezar a leer los datos y la almacena en la variable "i" definida previamente.
  for (i = 0; i < 3; i++) {
    if (j[(i + 1) % 3] != (j[i] + 1)) {
      break;
    }
  }

  // Ultimo mensaje de cabecera
  k = j[i];

  // Valor angular real de rodilla izquierda
  enviar[0] = income[(k + 2) % 70]; // Signo del digito. Si es 0 logico es positivo, si es 1 logico es negativo 
  enviar[1] = income[(k + 4) % 70]; // Valor real LSB ?
  enviar[2] = income[(k + 5) % 70]; // Valores decimal MSB ?
  enviar[3] = income[(k + 6) % 70]; // Valores decimales LSB ?

  // Salto del byte del delimitador (¿no deberían ser 2? 16bits de 0 = 2 8bits de 0)

  // Posicion de referencia de rodilla izquierda
  enviar1[0] = income[(k + 8) % 70];
  enviar1[1] = income[(k + 10) % 70];
  enviar1[2] = income[(k + 11) % 70];
  enviar1[3] = income[(k + 12) % 70];

  // Valor angular real de rodilla derecha
  enviar2[0] = income[(k + 14) % 70];
  enviar2[1] = income[(k + 16) % 70];
  enviar2[2] = income[(k + 17) % 70];
  enviar2[3] = income[(k + 18) % 70];

  // Posicion de referencia de rodilla derecha
  enviar3[0] = income[(k + 20) % 70];
  enviar3[1] = income[(k + 22) % 70];
  enviar3[2] = income[(k + 23) % 70];
  enviar3[3] = income[(k + 24) % 70];
  
  // Valor angular real de cadera izquierda
  enviar4[0] = income[(k + 26) % 70];
  enviar4[1] = income[(k + 28) % 70];
  enviar4[2] = income[(k + 29) % 70];
  enviar4[3] = income[(k + 30) % 70];
  
  // Posicion de referencia de cadera izquierda
  enviar5[0] = income[(k + 32) % 70];
  enviar5[1] = income[(k + 34) % 70];
  enviar5[2] = income[(k + 35) % 70];
  enviar5[3] = income[(k + 36) % 70];

  // Valor angular real de cadera derecha
  enviar6[0] = income[(k + 38) % 70];
  enviar6[1] = income[(k + 40) % 70];
  enviar6[2] = income[(k + 41) % 70];
  enviar6[3] = income[(k + 42) % 70];

  // Posicion de referencia de cadera derecha
  enviar7[0] = income[(k + 44) % 70];
  enviar7[1] = income[(k + 46) % 70];
  enviar7[2] = income[(k + 47) % 70];
  enviar7[3] = income[(k + 48) % 70];

  // Peso galga
  enviar8[0] = income[(k + 56) % 70];
  enviar8[1] = income[(k + 58) % 70];
  enviar8[2] = income[(k + 59) % 70];
  enviar8[3] = income[(k + 60) % 70];

  // Error de CAN
  can = income[(k + 52) % 70];

  // Error de posicion
  pos = income[(k + 64) % 70];
  
  // Rodilla izquierda
  udp1.beginPacket(IPAddress(192, 168, 4, 1), 10001);
    // Posicion real
    udp1.write(enviar, sizeof(enviar));
    // Posicion de referencia
    udp1.write(enviar1, sizeof(enviar1));
    udp1.endPacket();
    
  // Rodilla derecha
  udp2.beginPacket(IPAddress(192, 168, 4, 1), 10002);
    // Posicion real
    udp2.write(enviar2, sizeof(enviar2));
    // Posicion de referencia
    udp2.write(enviar3, sizeof(enviar3));
    udp2.endPacket();
    
  // Cadera izquierda
  udp3.beginPacket(IPAddress(192, 168, 4, 1), 10003);
    // Posicion real
    udp3.write(enviar4, sizeof(enviar4));
    // Posicion de referencia
    udp3.write(enviar5, sizeof(enviar5));
    udp3.endPacket();
    
  // Cadera derecha
  udp4.beginPacket(IPAddress(192, 168, 4, 1), 10004);
    // Posicion real
    udp4.write(enviar6, sizeof(enviar6));
    // Posicion de referencia
    udp4.write(enviar7, sizeof(enviar7));
    udp4.endPacket();
    
  // Peso de galga
  udp5.beginPacket(IPAddress(192, 168, 4, 1), 10005);
    udp5.write(enviar8, sizeof(enviar8));
    udp5.endPacket();
    
  // Error de CAN
  udp5.beginPacket(IPAddress(192, 168, 4, 1), 10006);
    udp5.write(can);
    udp5.endPacket();
    
  // Error de posicion
  udp5.beginPacket(IPAddress(192, 168, 4, 1), 10007);
    udp5.write(pos);
    udp5.endPacket();

  // Debug spi received, sended via udp
  count_sended_msgs = count_sended_msgs + 1; 
  if (count_sended_msgs == 1000) {
    count_sended_msgs = 0;
    Serial.printf("\nRodilla izquierda %d, %d", enviar, enviar1);
    Serial.printf("\nRodilla derecha %d, %d", enviar2, enviar3);
    Serial.printf("\nCadera izquierda %d, %d", enviar4, enviar5);
    Serial.printf("\nCadera derecha %d, %d", enviar6, enviar7);
  }
  
}
