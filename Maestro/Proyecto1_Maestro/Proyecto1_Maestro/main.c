/*
 * Proyecto1_Maestro.c
 *
 * Created: 10/02/2026 14:57:37
 * Author : mario
 */ 

/************************************************************************/
/* LIBRERÍAS Y DEFINICIONES                                             */
/************************************************************************/
#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>
#include "I2C.h"
#include "TIMER1.h"

// Direcciones de esclavos
#define NANO_SLAVE1_ADDR	0x30		// Arduino Nano 1
#define NANO_SLAVE2_ADDR	0x31		// Arduino Nano 2
#define LIGHT_SENSOR_ADDR	0x32		// Sensor de luz I2C
#define HTEMP_SENSOR_ADDR	0x33		// Sensor de humedad I2C

// Comandos
#define I2C_CMD1 0x40
#define I2C_CMD2 0x40
#define I2C_CMD3 0x40

/************************************************************************/
/* SETUP				                                                */
/************************************************************************/
void setup(void){
	// Inicializar I2C a 100kHz con prescaler 1
	I2C_MasterInit(100000, 1);
	
	// Inicializar TIMER1 (A una frecuencia de )
	Timer1Init(TIMER1_MODE0_NORMAL,TIMER1_PRESCALER1,TIMER1_INTERRUPTS_DISABLED, TIMER1_COMP_OUT_MODE0);
	Timer1SetFrequency(100);
	
}

/************************************************************************/
/* MAINLOOP				                                                */
/************************************************************************/
int main(void) {	
	while(1) {
		// 1. Intentar iniciar comunicación
		if (I2C_MasterStart()) {
			
			// 2. Dirección + Write (0)
			// El estado 0x18 significa que el esclavo recibió su dirección y dio ACK
			if (I2C_Master_Write((NANO_SLAVE1_ADDR << 1) | I2C_WRITE) == 0x18) {
				
				// 3. Mandar el dato (un caracter 'A' para prender, 'B' para apagar)
				I2C_Master_Write('A');
				_delay_ms(500);
				
				// Mandar 'B' para que el esclavo apague el LED
				I2C_Master_Write('B');
			}
			
			// 4. Siempre cerrar con STOP
			I2C_MasterStop();
		}
		
		_delay_ms(500);
	}
}

/************************************************************************/
/* RUTINAS DE INTERRUPCIÓN                                              */
/************************************************************************/
