#ifndef DHT11_H_
#define DHT11_H_

#define F_CPU 16000000UL

#include <avr/io.h>
#include <util/delay.h>

// ==========================================
// CONFIGURACIÓN DE USUARIO (Modifica aquí)
// ==========================================
#define DHT_PORT    PORTD
#define DHT_DDR     DDRD
#define DHT_PIN_REG PIND
#define DHT_BIT     PD2  // El número del pin (0-7)
// ==========================================

// --- PROTOTIPOS ---
void DHT11_Init(void);
int8_t DHT11_Read(uint8_t *h, uint8_t *t);

#endif