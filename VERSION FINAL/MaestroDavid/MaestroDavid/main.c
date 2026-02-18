/*
 * Maestro.c
 * Author : David Carranza / Mario Betancourt
 */ 

#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <stdlib.h>
#include <stdio.h>
#include <avr/interrupt.h>
#include "I2C.h"
#include "UART.h"
#include "HD44780_4b.h"

// --- DIRECCIONES I2C ---
#define SLAVE_ACTUATORS_ADDR  0x30
#define SLAVE_ENV_ADDR        0x31
#define TSL2561_ADDR          0x39

// --- COMANDOS ---
#define CMD_PUMP_START  0x20
#define CMD_PUMP_STOP   0x21
#define CMD_SERVO_OPEN  0x10
#define CMD_SERVO_CLOSE 0x11
#define CMD_READ_SOIL   0x40
#define CMD_FAN_ON      0x41
#define CMD_FAN_OFF     0X42
#define CMD_FAN_PWM     0x43

// --- PARÁMETROS DE SENSOR ---
#define SOIL_RAW_DRY    255
#define SOIL_RAW_WET    160
#define RX_BUFFER_SIZE  32

// --- VARIABLES GLOBALES ---
volatile char rx_buffer[RX_BUFFER_SIZE];
volatile uint8_t rx_index = 0;
volatile uint8_t command_ready = 0;
char lcd_buf[17]; // Buffer para la LCD definido globalmente

// --- ESTRUCTURA LCD ---
LCD_4b lcd = {
    .rs = { &PORTD, &DDRD, PORTD2 },
    .e  = { &PORTD, &DDRD, PORTD3 },
    .d4 = { &PORTD, &DDRD, PORTD5 },
    .d5 = { &PORTD, &DDRD, PORTD6 },
    .d6 = { &PORTD, &DDRD, PORTD7 },
    .d7 = { &PORTB, &DDRB, PORTB0 }
};

// --- PROTOTIPOS ---
void Process_Command(void);
void Fan_SetSpeed(uint8_t speed);
uint8_t Read_Soil(void);
uint8_t Map_Soil_Percentage(uint8_t raw_value);
void I2C_SendCommand(uint8_t slave_addr, uint8_t command);
void TSL2561_Init(void);
uint16_t TSL2561_Read(void);
void TSL2561_WriteRegister(uint8_t reg, uint8_t data);
void Test_Sensor_ID(void);

// --- SETUP ---
void setup(void)
{
	I2C_MasterInit(100000UL, 1);
	UART_Init(UART_BAUD_9600_16MHZ, UART_INTERRUPTS_ENABLED);
	LCD_Init_4b(&lcd);
	
	TSL2561_Init();    // Configura el sensor
	Test_Sensor_ID();  // <-- ¡PONLO AQUÍ! Diagnóstico al arrancar
	
	sei();   // Habilitar interrupciones globales

	LCD_Clear(&lcd);
	LCD_SetCursor(&lcd, 0, 0);
	LCD_WriteString(&lcd, "INVERNADERO OK");
	UART_SendString("Sistema listo\r\n");
	
	_delay_ms(500);
	LCD_Clear(&lcd);
}

// --- MAIN ---
int main(void) {
    setup();

    while(1) {
        if(command_ready) {
            Process_Command();
            command_ready = 0;
        }
        
        static uint16_t refresh_timer = 0;
        if(refresh_timer++ > 500) { // Cada ~500ms
            uint16_t current_lux = TSL2561_Read();
            uint8_t percentage = Map_Soil_Percentage(Read_Soil());

            // Actualizar LCD - Fila 1: Humedad
            LCD_SetCursor(&lcd, 0, 1);
            sprintf(lcd_buf, "Hum: %3d%%      ", percentage);
            LCD_WriteString(&lcd, lcd_buf);
            
            // Actualizar LCD - Fila 0: Luz (después del mensaje OK)
            LCD_SetCursor(&lcd, 10, 0);
            sprintf(lcd_buf, "L:%4u", current_lux);
            LCD_WriteString(&lcd, lcd_buf);
            
            refresh_timer = 0;
        }
        _delay_ms(1);
    }
}

// --- FUNCIONES AUXILIARES ---

// Nueva función para evitar repetir la matemática de la humedad
uint8_t Map_Soil_Percentage(uint8_t raw_value) {
    if(raw_value >= SOIL_RAW_DRY) return 0;
    if(raw_value <= SOIL_RAW_WET) return 100;
    return 100 - ((raw_value - SOIL_RAW_WET) * 100) / (SOIL_RAW_DRY - SOIL_RAW_WET);
}

void Process_Command(void) {
    switch (rx_buffer[0]) {
        case 'Q': {
            //uint8_t percentage = Map_Soil_Percentage(Read_Soil());
            //sprintf(lcd_buf, "Soil Hum: %d%%\r\n", percentage);
			sprintf(lcd_buf, "Soil Hum: %d%%\r\n", Read_Soil());
            UART_SendString(lcd_buf);
            //UART_SendString(percentage > 40 ? "Status: OK\r\n" : "Status: DRY\r\n");
        } break;

        case 'L': {
            uint16_t lux = TSL2561_Read();
            sprintf(lcd_buf, "Luz: %u\r\n", lux);
            UART_SendString(lcd_buf);
        } break;

        case 'P':
            if(rx_buffer[1] == '1') I2C_SendCommand(SLAVE_ACTUATORS_ADDR, CMD_PUMP_START);
            else I2C_SendCommand(SLAVE_ACTUATORS_ADDR, CMD_PUMP_STOP);
            break;

        case 'S':
            if(rx_buffer[1] == '1') I2C_SendCommand(SLAVE_ACTUATORS_ADDR, CMD_SERVO_OPEN);
            else I2C_SendCommand(SLAVE_ACTUATORS_ADDR, CMD_SERVO_CLOSE);
            break;

        case 'F':
            if(rx_buffer[1] == '1') I2C_SendCommand(SLAVE_ENV_ADDR, CMD_FAN_ON);
            else I2C_SendCommand(SLAVE_ENV_ADDR, CMD_FAN_OFF);
            break;

        case 'V': {
            uint8_t speed = (uint8_t)atoi((char*)&rx_buffer[1]);
            Fan_SetSpeed(speed);
        } break;
    }
}

// --- IMPLEMENTACIÓN I2C / SENSORES ---

void I2C_SendCommand(uint8_t slave_addr, uint8_t command) {
    if (I2C_MasterStart()) {
        I2C_Master_Write((slave_addr << 1) | I2C_WRITE);
        I2C_Master_Write(command);
        I2C_MasterStop();
    }
}

void Fan_SetSpeed(uint8_t speed) {
    if (I2C_MasterStart()) {
        I2C_Master_Write((SLAVE_ENV_ADDR << 1) | I2C_WRITE);
        I2C_Master_Write(CMD_FAN_PWM);
        I2C_Master_Write(speed);
        I2C_MasterStop();
    }
}

uint8_t Read_Soil(void) {
    uint8_t val = 0;
    if (I2C_MasterStart()) {
        I2C_Master_Write((SLAVE_ENV_ADDR << 1) | I2C_WRITE);
        I2C_Master_Write(CMD_READ_SOIL);
        I2C_MasterRepeatedStart();
        I2C_Master_Write((SLAVE_ENV_ADDR << 1) | I2C_READ);
        I2C_MasterRead(&val, I2C_NACK);
        I2C_MasterStop();
    }
    return val;
}

void TSL2561_Init(void) {
	// 1. POWER UP: Escribir 0x03 en el registro 0x00
	TSL2561_WriteRegister(0x00, 0x03);
	_delay_ms(10); // Esperar a que el regulador interno estabilice

	// 2. CONFIGURAR TIMING: Escribir 0x12 (Gain 16x, 402ms) en el registro 0x01
	// Esto es vital para que no dé 0 en interiores
	TSL2561_WriteRegister(0x01, 0x12);
}

uint16_t TSL2561_Read(void) {
    uint8_t low = 0, high = 0;
    if (I2C_MasterStart()) {
        I2C_Master_Write((TSL2561_ADDR << 1) | I2C_WRITE);
        I2C_Master_Write(0x80 | 0x20 | 0x0C); 
        I2C_MasterRepeatedStart();
        I2C_Master_Write((TSL2561_ADDR << 1) | I2C_READ);
        I2C_MasterRead(&low, I2C_ACK);
        I2C_MasterRead(&high, I2C_NACK);
        I2C_MasterStop();
    }
    return (uint16_t)((high << 8) | low);
}

// Función para escribir un valor en un registro específico del sensor
void TSL2561_WriteRegister(uint8_t reg, uint8_t data) {
	if (I2C_MasterStart()) {
		I2C_Master_Write((TSL2561_ADDR << 1) | I2C_WRITE);
		I2C_Master_Write(0x80 | reg); // Bit de comando + dirección del registro
		I2C_Master_Write(data);       // El valor a escribir (ej. 0x03 para encender)
		I2C_MasterStop();
	}
}

void Test_Sensor_ID(void) {
	uint8_t id = 0;
	if (I2C_MasterStart()) {
		I2C_Master_Write((TSL2561_ADDR << 1) | I2C_WRITE);
		I2C_Master_Write(0x80 | 0x0A); // Dirección del registro ID
		I2C_MasterRepeatedStart();
		I2C_Master_Write((TSL2561_ADDR << 1) | I2C_READ);
		I2C_MasterRead(&id, I2C_NACK);
		I2C_MasterStop();
		
		char buf[30];
		sprintf(buf, "ID del Sensor: 0x%02X\r\n", id);
		UART_SendString(buf);
		
		// El TSL2561 suele devolver algo como 0x11 o 0x50 según la versión
		if(id != 0 && id != 0xFF) UART_SendString("Sensor VIVO!\r\n");
		else UART_SendString("Sensor NO RESPONDE\r\n");
	}
}

ISR(USART_RX_vect) {
    char c = UDR0;
    if(c == '\n' || c == '\r') {
        rx_buffer[rx_index] = '\0';
        command_ready = 1;
        rx_index = 0;
    } else if(rx_index < RX_BUFFER_SIZE - 1) {
        rx_buffer[rx_index++] = c;
    }
}


