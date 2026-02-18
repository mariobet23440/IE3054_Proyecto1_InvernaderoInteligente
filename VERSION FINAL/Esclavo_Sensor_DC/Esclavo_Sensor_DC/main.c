/*
 * Esclavo_Sensor_DHT11_Final.c
 * Proyecto: Invernadero Inteligente
 */ 

#define F_CPU 16000000UL

#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>

// Librerías del proyecto
#include "ADC.h"
#include "I2C_Slave.h"
#include "dc_motor.h"
#include "UART.h"
#include "DHT11.h"

/************************************************************************/
/* CONFIGURACIÓN Y COMANDOS                                             */
/************************************************************************/

#define SLAVE_ADDR      0x31
#define CMD_READ_SOIL   0x40
#define CMD_FAN_ON      0x41
#define CMD_FAN_OFF     0x42
#define CMD_FAN_PWM     0x43
#define CMD_READ_HUM    0x44
#define CMD_READ_TEMP   0x45

#define SOIL_POWER_PIN  PB0  // D8 en Arduino

// Variables globales para caché (Almacenan la última lectura exitosa)
volatile uint8_t ultima_humedad = 0;
volatile uint8_t ultima_temperatura = 0;

extern volatile uint8_t tx_data; // Definido en I2C_Slave.c

/************************************************************************/
/* MAIN                                                                 */
/************************************************************************/

int main(void)
{
    uint8_t command; 
    uint16_t dht_timer = 0; // Contador para leer el DHT11 sin bloquear
    
    // 1. Inicialización de periféricos
    UART_Init(UART_BAUD_9600_16MHZ, UART_INTERRUPTS_DISABLED);
    UART_SendString("Invernadero: Esclavo Ambiental Iniciado\r\n");
    
    I2C_Slave_Init(SLAVE_ADDR);
    ADC_Init();
    DC_Init();
    DHT11_Init();
    
    // Configuración de pin de alimentación del sensor de suelo (ahorro de energía)
    DDRB |= (1 << SOIL_POWER_PIN);
    PORTB &= ~(1 << SOIL_POWER_PIN); 
    
    while(1)
    {
        // --- TAREA 1: LECTURA PERIÓDICA DEL DHT11 (Cada ~2 segundos) ---
        // El DHT11 no se debe leer más rápido de 1 vez por segundo.
        if (dht_timer >= 2000) 
        {
            uint8_t h, t;
            if (DHT11_Read(&h, &t) != -1) 
            {
                ultima_humedad = h;
                ultima_temperatura = t;
            }
            dht_timer = 0;
        }

        // --- TAREA 2: ATENCIÓN DE COMANDOS I2C ---
        command = I2C_Slave_CheckCommand();
        
        if (command != 0xFF)
        {
            switch(command)
            {
                case CMD_READ_SOIL:
                {
                    PORTB |= (1 << SOIL_POWER_PIN); // Encender sensor de suelo
                    _delay_ms(5);                   // Estabilización corta
                    uint16_t soil_val = ADC_Read(0);
                    PORTB &= ~(1 << SOIL_POWER_PIN); // Apagar sensor
                    tx_data = (uint8_t)(soil_val >> 2); // Enviar 8 bits
                }
                break;
                
                case CMD_READ_HUM:
                    tx_data = ultima_humedad; // Respuesta instantánea
                    break;

                case CMD_READ_TEMP:
                    tx_data = ultima_temperatura; // Respuesta instantánea
                    break;
                
                case CMD_FAN_ON:
                    DC_On();
                    break;
                    
                case CMD_FAN_OFF:
                    DC_Off();
                    break;
                
                case CMD_FAN_PWM:
                {
                    // Recibir el byte adicional de velocidad
                    while(!(TWCR & (1 << TWINT))); 
                    uint8_t speed = TWDR;
                    TWCR = (1<<TWEN)|(1<<TWEA)|(1<<TWINT);
                    DC_SetSpeed(speed);
                }
                break;
            }        
        }
        
        // Pequeño delay para el contador del timer
        _delay_ms(1);
        dht_timer++;
    }
}