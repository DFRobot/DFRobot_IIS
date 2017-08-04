#ifndef SI2C_h
#define SI2C_h

#ifdef __cplusplus
extern "C" {
#endif

void twi_init(unsigned char sda, unsigned char scl);
void twi_stop(void);
void twi_setClock(unsigned int freq);
uint8_t twi_writeTo(unsigned char address, unsigned char * buf, unsigned int len, unsigned char sendStop);
uint8_t twi_readFrom(unsigned char address, unsigned char * buf, unsigned int len, unsigned char sendStop);

#ifdef __cplusplus
}
#endif

#endif