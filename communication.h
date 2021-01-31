#include "init.h"

void esperar_paquete_Udp(void);
void I2C(void);
void read_UDP(int, int);
void enviar_en_spiA(byte*, int,uint16_t);
void enviar_en_spiC50011(byte*, int);
extern byte can;
