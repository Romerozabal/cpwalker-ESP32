#include "communication.h"
#include <string.h>
#include <stdlib.h>

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
  int packetSize50017 = udp50017.parsePacket();
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
  if (packetSize50017) {
    read_UDP(50017, packetSize50017);
    Serial.printf("\nReceived %d bytes from %s, port %d\n", packetSize50017, udp50017.remoteIP().toString().c_str(), udp50017.remotePort());
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
    case 50017:
      udp50017.read(&buf_trac1.bytes[0], read_size);
      enviar_en_spiA(buf_trac1.bytes, packetSize,uint16_t(50017)); 
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
int msg_header_pos = 0;
int msg_size = 72; // Envían desde la Texas 36 datos in16 = 72 datos int8 recibidos por ESP32
byte income[72]; // Datos recibidos por el bus I2C
byte left_knee_pot[4], left_knee_ref[4], right_knee_pot[4], right_knee_ref[4], left_hip_pot[4], left_hip_ref[4], right_hip_pot[4], right_hip_ref[4],weightgauge[4],indexRight,indexLeft, left_knee_torque[4], right_knee_torque[4], left_hip_torque[4], right_hip_torque[4], elev_ref, traction_ref[4], left_wheel_vel[4], right_wheel_vel[4];

// Recibir datos de Texas
void I2C() {
  // Almacenar los datos procedentes del i2c mientras esten disponibles en variable "income"
  int index = 0;
  Wire.requestFrom(1, 72);
  while (Wire.available()) {
    byte datos = Wire.read();
    int8_t dato = (int8_t)datos;
    income[index] = datos;
    index++;
  }

  // Busca posición del último dato de la cabecera [255, >255< , ... data ... ] y guarda sus posiciones. 
  index = 0;
  for (int i = 0; i < msg_size; i++) {
    if (income[i] == 255 && income[(i + 1) % msg_size] == 255) {
      msg_header_pos = i + 1;
      break;
    }
  }
  
  // Valor angular real de rodilla izquierda
  left_knee_pot[0] = income[(msg_header_pos + 1) % msg_size]; // Entero
  left_knee_pot[1] = income[(msg_header_pos + 2) % msg_size]; // Entero
  left_knee_pot[2] = income[(msg_header_pos + 3) % msg_size]; // Decimal
  left_knee_pot[3] = income[(msg_header_pos + 4) % msg_size]; // Decimal

  // Posicion de referencia de rodilla izquierda
  left_knee_ref[0] = income[(msg_header_pos + 5) % msg_size];
  left_knee_ref[1] = income[(msg_header_pos + 6) % msg_size];
  left_knee_ref[2] = income[(msg_header_pos + 7) % msg_size];
  left_knee_ref[3] = income[(msg_header_pos + 8) % msg_size];

  // Valor angular real de rodilla derecha
  right_knee_pot[0] = income[(msg_header_pos + 9) % msg_size];
  right_knee_pot[1] = income[(msg_header_pos + 10) % msg_size];
  right_knee_pot[2] = income[(msg_header_pos + 11) % msg_size];
  right_knee_pot[3] = income[(msg_header_pos + 12) % msg_size];

  // Posicion de referencia de rodilla derecha
  right_knee_ref[0] = income[(msg_header_pos + 13) % msg_size];
  right_knee_ref[1] = income[(msg_header_pos + 14) % msg_size];
  right_knee_ref[2] = income[(msg_header_pos + 15) % msg_size];
  right_knee_ref[3] = income[(msg_header_pos + 16) % msg_size];
  
  // Valor angular real de cadera izquierda
  left_hip_pot[0] = income[(msg_header_pos + 17) % msg_size];
  left_hip_pot[1] = income[(msg_header_pos + 18) % msg_size];
  left_hip_pot[2] = income[(msg_header_pos + 19) % msg_size];
  left_hip_pot[3] = income[(msg_header_pos + 20) % msg_size];
  
  // Posicion de referencia de cadera izquierda
  left_hip_ref[0] = income[(msg_header_pos + 21) % msg_size];
  left_hip_ref[1] = income[(msg_header_pos + 22) % msg_size];
  left_hip_ref[2] = income[(msg_header_pos + 23) % msg_size];
  left_hip_ref[3] = income[(msg_header_pos + 24) % msg_size];

  // Valor angular real de cadera derecha
  right_hip_pot[0] = income[(msg_header_pos + 25) % msg_size];
  right_hip_pot[1] = income[(msg_header_pos + 26) % msg_size];
  right_hip_pot[2] = income[(msg_header_pos + 27) % msg_size];
  right_hip_pot[3] = income[(msg_header_pos + 28) % msg_size];

  // Posicion de referencia de cadera derecha
  right_hip_ref[0] = income[(msg_header_pos + 29) % msg_size];
  right_hip_ref[1] = income[(msg_header_pos + 30) % msg_size];
  right_hip_ref[2] = income[(msg_header_pos + 31) % msg_size];
  right_hip_ref[3] = income[(msg_header_pos + 32) % msg_size];

  // Peso galga
  weightgauge[0] = income[(msg_header_pos + 33) % msg_size];
  weightgauge[1] = income[(msg_header_pos + 34) % msg_size];
  weightgauge[2] = income[(msg_header_pos + 35) % msg_size];
  weightgauge[3] = income[(msg_header_pos + 36) % msg_size];

  // Index left knee
  indexLeft = income[(msg_header_pos + 38) % msg_size];

  // Index right knee
  indexRight = income[(msg_header_pos + 40) % msg_size];
  
  // Torque rodilla izquierda
  left_knee_torque[0] = income[(msg_header_pos + 41) % msg_size];
  left_knee_torque[1] = income[(msg_header_pos + 42) % msg_size];
  left_knee_torque[2] = income[(msg_header_pos + 43) % msg_size];
  left_knee_torque[3] = income[(msg_header_pos + 44) % msg_size];
  
  // Torque rodilla derecha
  right_knee_torque[0] = income[(msg_header_pos + 45) % msg_size];
  right_knee_torque[1] = income[(msg_header_pos + 46) % msg_size];
  right_knee_torque[2] = income[(msg_header_pos + 47) % msg_size];
  right_knee_torque[3] = income[(msg_header_pos + 48) % msg_size];
  
  // Torque cadera izquierda
  left_hip_torque[0] = income[(msg_header_pos + 49) % msg_size];
  left_hip_torque[1] = income[(msg_header_pos + 50) % msg_size];
  left_hip_torque[2] = income[(msg_header_pos + 51) % msg_size];
  left_hip_torque[3] = income[(msg_header_pos + 52) % msg_size];
  
  // Torque cadera derecha
  right_hip_torque[0] = income[(msg_header_pos + 53) % msg_size];
  right_hip_torque[1] = income[(msg_header_pos + 54) % msg_size];
  right_hip_torque[2] = income[(msg_header_pos + 55) % msg_size];
  right_hip_torque[3] = income[(msg_header_pos + 56) % msg_size];

  // Elevation reference
  elev_ref = income[(msg_header_pos + 58) % msg_size];

  // Traction reference
  traction_ref[0] = income[(msg_header_pos + 59) % msg_size];
  traction_ref[1] = income[(msg_header_pos + 60) % msg_size];
  traction_ref[2] = income[(msg_header_pos + 61) % msg_size];
  traction_ref[3] = income[(msg_header_pos + 62) % msg_size];

  // Left Wheel velocitu
  left_wheel_vel[0] = income[(msg_header_pos + 63) % msg_size];
  left_wheel_vel[1] = income[(msg_header_pos + 64) % msg_size];
  left_wheel_vel[2] = income[(msg_header_pos + 65) % msg_size];
  left_wheel_vel[3] = income[(msg_header_pos + 66) % msg_size];

  // Right Wheel velocitu
  right_wheel_vel[0] = income[(msg_header_pos + 67) % msg_size];
  right_wheel_vel[1] = income[(msg_header_pos + 68) % msg_size];
  right_wheel_vel[2] = income[(msg_header_pos + 69) % msg_size];
  right_wheel_vel[3] = income[(msg_header_pos + 70) % msg_size];

  // Send message
  udp.beginPacket(IPAddress(192, 168, 4, 3), 10000);
   // Rodilla izquierda
      // Posicion real
      udp.write(left_knee_pot, sizeof(left_knee_pot));
      // Posicion de referencia
      udp.write(left_knee_ref, sizeof(left_knee_ref));
   // Rodilla derecha
      // Posicion real
      udp.write(right_knee_pot, sizeof(right_knee_pot));
      // Posicion de referencia
      udp.write(right_knee_ref, sizeof(right_knee_ref));
   // Cadera izquierda
      // Posicion real
      udp.write(left_hip_pot, sizeof(left_hip_pot));
      // Posicion de referencia
      udp.write(left_hip_ref, sizeof(left_hip_ref));
   // Cadera derecha
      // Posicion real
      udp.write(right_hip_pot, sizeof(right_hip_pot));
      // Posicion de referencia
      udp.write(right_hip_ref, sizeof(right_hip_ref));
      // Index
      udp.write(indexRight);
      udp.write(indexLeft);
   // Torque
      // Rodilla Izquierda
      udp.write(left_knee_torque, sizeof(left_knee_torque));
      // Rodilla Derecha
      udp.write(right_knee_torque, sizeof(right_hip_ref));
      // Cadera Izquierda
      udp.write(left_hip_torque, sizeof(left_knee_torque));
      // Cadera Derecha
      udp.write(right_hip_torque, sizeof(right_hip_ref));
      // Peso de galga
      udp.write(weightgauge, sizeof(weightgauge));
   // Elevación
      udp.write(elev_ref);
   // Tracción
      // Referencia
      udp.write(traction_ref, sizeof(left_knee_torque));
      // Velocidad rueda izquierda
      udp.write(left_wheel_vel, sizeof(right_hip_ref));
      // Velocidad rueda derecha
      udp.write(left_wheel_vel, sizeof(right_hip_ref));
    udp.endPacket();
    Serial.printf("%d \n", millis());
}
