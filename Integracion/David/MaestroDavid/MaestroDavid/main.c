/*
 * Maestro.c
 *
 * Created: 12/02/2026 07:37:51 p. m.
 * Author : David Carranza
 * Revisado por: Mario Betancourt
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

/************************************************************************/
/*                        DIRECCIONES I2C                               */
/************************************************************************/

#define SLAVE_ACTUATORS_ADDR  0x30	// Nano 1
#define SLAVE_ENV_ADDR		  0x31	// Nano 2
#define TSL2561_ADDR          0x39	// Sensor de Luz

/************************************************************************/
/* COMANDOS NANO 1                                                      */
/************************************************************************/

#define CMD_PUMP_START	0x20
#define CMD_PUMP_STOP	0x21
#define CMD_SERVO_OPEN	0x10
#define CMD_SERVO_CLOSE	0x11

/************************************************************************/
/* COMANDOS NANO 2                                                      */
/************************************************************************/

#define CMD_READ_SOIL	0x40
#define CMD_FAN_ON		0x41
#define CMD_FAN_OFF		0X42
#define CMD_FAN_PWM		0x43

/************************************************************************/
/* COMANDOS SENSOR LUZ                                                                     */
/************************************************************************/

#define TSL2561_CMD_BIT       0x80
#define TSL2561_WORD_BIT      0x20
#define TSL2561_REG_CONTROL   0x00
#define TSL2561_REG_DATA0LO   0x0C
#define CMD_POWER_UP          (TSL2561_CMD_BIT | TSL2561_REG_CONTROL)
#define CMD_READ_DATA         (TSL2561_CMD_BIT | TSL2561_WORD_BIT | TSL2561_REG_DATA0LO)

/************************************************************************/
/* PARAMETROS                                                           */
/************************************************************************/

#define SOIL_THRESHOLD   150
#define RX_BUFFER_SIZE   32
#define SOIL_RAW_DRY   255
#define SOIL_RAW_WET   160

/************************************************************************/
/* VARIABLES UART                                                       */
/************************************************************************/

volatile char rx_buffer[RX_BUFFER_SIZE];
volatile uint8_t rx_index = 0;
volatile uint8_t command_ready = 0;

// PROTOTIPOS DE FUNCIONES
// Procesamiento de comandos
void Process_Command(void);
void Fan_SetSpeed(uint8_t speed);
uint8_t Read_Soil(void);

void I2C_SendCommand(uint8_t slave_addr, uint8_t command);
void TSL2561_Init(void);
uint16_t TSL2561_Read(void);

/************************************************************************/
/* LCD 4 BITS                                                                    */
/************************************************************************/

LCD_4b lcd = {
	.rs = { &PORTD, &DDRD, PORTD2 },
	.e  = { &PORTD, &DDRD, PORTD3 },
	.d4 = { &PORTD, &DDRD, PORTD5 },
	.d5 = { &PORTD, &DDRD, PORTD6 },
	.d6 = { &PORTD, &DDRD, PORTD7 },
	.d7 = { &PORTB, &DDRB, PORTB0 }
};


/************************************************************************/
/* SETUP                                                                */
/************************************************************************/
void setup(void)
{
	I2C_MasterInit(100000UL, 1);
	UART_Init(UART_BAUD_9600_16MHZ, UART_INTERRUPTS_ENABLED);
	LCD_Init_4b(&lcd);
	
	// Inicializar TSL2561
	// 	I2C_MasterStart();
	// 	I2C_Master_Write((TSL2561_ADDR << 1) | I2C_WRITE);
	// 	I2C_Master_Write(0x80 | 0x00); // Control Register
	// 	I2C_Master_Write(0x03);        // Power Up
	// 	I2C_MasterStop();

	sei();   // Habilitar interrupciones globales

	LCD_Clear(&lcd);
	LCD_SetCursor(&lcd, 0, 0);
	LCD_WriteString(&lcd, "INVERNADERO OK");
	
	UART_SendString("Sistema listo\r\n");
	
	//uint8_t current_hum = 0;
	char lcd_buf[17];
}

/************************************************************************/
/* MAIN                                                                 */
/************************************************************************/

int main(void)
{
	setup();

	while(1)
	{
		// Comandos UART
		if(command_ready)
		{
			Process_Command();
			command_ready = 0;
		}
		
		// B. Actualizar datos de sensores cada cierto tiempo (sin usar delay largo)
		static uint16_t refresh_timer = 0;
		if(refresh_timer++ > 500) 
		
		{ 
			// Aproximadamente cada 500ms
			/*uint16_t current_lux = TSL2561_Read_Luminosity();*/
			uint8_t soil = Read_Soil();

			uint8_t percentage;

			if(soil >= SOIL_RAW_DRY)
			percentage = 0;
			else if(soil <= SOIL_RAW_WET)
			percentage = 100;
			else
			{
				percentage = 100 -
				((soil - SOIL_RAW_WET) * 100) /
				(SOIL_RAW_DRY - SOIL_RAW_WET);
			}

			
			// Actualizar LCD Fila 2
 			LCD_SetCursor(&lcd, 0, 10);
 			sprintf(lcd_buf, "L: %5u lux  ", current_lux);
 			LCD_WriteString(&lcd, lcd_buf);
			
			LCD_SetCursor(&lcd, 0, 1);
			sprintf(lcd_buf, "H: %3d%%  ", percentage);
			LCD_WriteString(&lcd, lcd_buf);
			
			refresh_timer = 0;
		}
		
		_delay_ms(1);
	}
}

/************************************************************************/
/* ISR                                                                  */
/************************************************************************/

ISR(USART_RX_vect)
{
	char c = UDR0;

	if(c == '\n' || c == '\r')
	{
		rx_buffer[rx_index] = '\0';
		command_ready = 1;
		rx_index = 0;
	}
	else
	{
		if(rx_index < RX_BUFFER_SIZE - 1)
		rx_buffer[rx_index++] = c;
	}
}

/************************************************************************/
/* fUNCIONES AUXILIARES                                                 */
/************************************************************************/
// Procesar comandos recibidos por UART
void Process_Command(void)
{
	// El switch selecciona la categoría del comando basado en el primer carácter
	switch (rx_buffer[0])
	{
		// -- CONSULTAR HUMEDAD DE SUELO ('Q') --
		case 'Q':
		{
			// Lectura de humedad de suelo (Byte)
			uint8_t soil = Read_Soil();

			// Cálculo de porcentaje relativo de humedad
			uint8_t percentage;
			
			// Truncamiento de valor y cálculo en región de sensado
			if(soil >= SOIL_RAW_DRY) percentage = 0;
			else if(soil <= SOIL_RAW_WET) percentage = 100;
			else percentage = 100 - ((soil - SOIL_RAW_WET) * 100) / (SOIL_RAW_DRY - SOIL_RAW_WET);
			
			char buffer[40];
			sprintf(buffer, "Soil: %d (%d%%)\r\n", soil, percentage);
			UART_SendString(buffer);

			if(percentage > 40) UART_SendString("No necesita agua\r\n");
			else UART_SendString("Necesita agua\r\n");
		}
		break;

		// -- CONSULTAR LUZ AMBIENTE ('L') --
		case 'L':
		{
			char buffer[40];
			// Usando la función de lectura de 16 bits que configuramos antes
			uint16_t lux = TSL2561_Read();
			sprintf(buffer, "Luminosidad: %u\r\n", lux);
			UART_SendString(buffer);
		}
		break;

		// -- MOTOR STEPPER : BOMBA DE RIEGO ('P') --
		case 'P':
		if(rx_buffer[1] == '1'){
			I2C_SendCommand(SLAVE_ACTUATORS_ADDR, CMD_PUMP_START);
			UART_SendString("Pump START\r\n");
		}
		else if(rx_buffer[1] == '0'){
			I2C_SendCommand(SLAVE_ACTUATORS_ADDR, CMD_PUMP_STOP);
			UART_SendString("Pump STOP\r\n");
		}
		break;

		// -- SERVOMOTOR : PUERTA ('S') --
		case 'S':
		if(rx_buffer[1] == '1'){
			I2C_SendCommand(SLAVE_ACTUATORS_ADDR, CMD_SERVO_OPEN);
			UART_SendString("Servo OPEN\r\n");
		}
		else if(rx_buffer[1] == '0'){
			I2C_SendCommand(SLAVE_ACTUATORS_ADDR, CMD_SERVO_CLOSE);
			UART_SendString("Servo CLOSE\r\n");
		}
		break;

		// -- MOTOR DC : VENTILADOR ('F') --
		case 'F':
		if(rx_buffer[1] == '1'){
			I2C_SendCommand(SLAVE_ENV_ADDR, CMD_FAN_ON);
			UART_SendString("Fan ON\r\n");
		}
		else if(rx_buffer[1] == '0'){
			I2C_SendCommand(SLAVE_ENV_ADDR, CMD_FAN_OFF);
			UART_SendString("Fan OFF\r\n");
		}
		break;

		// -- MOTOR DC : CONTROL DE VELOCIDAD ('V') --
		case 'V':
		{
			uint8_t speed = (uint8_t)atoi((char*)&rx_buffer[1]);
			if(speed > 255) speed = 255;

			Fan_SetSpeed(speed);

			char buffer[40];
			sprintf(buffer, "Fan speed: %d\r\n", speed);
			UART_SendString(buffer);
		}
		break;

		// Caso opcional para comandos no reconocidos
		default:
		UART_SendString("Comando desconocido\r\n");
		break;
	}
}

void I2C_SendCommand(uint8_t slave_addr, uint8_t command) {
	if (I2C_MasterStart()) {
		I2C_Master_Write((slave_addr << 1) | I2C_WRITE);
		I2C_Master_Write(command);
		I2C_MasterStop();
	}
}


/************************************************************************/
/* FUNCIONES I2C                                                        */
/************************************************************************/
// Establecer velocidad de ventilador
void Fan_SetSpeed(uint8_t speed)
{
	I2C_MasterStart();
	I2C_Master_Write((SLAVE_ENV_ADDR<<1)|I2C_WRITE);
	I2C_Master_Write(CMD_FAN_PWM);
	I2C_Master_Write(speed);
	I2C_MasterStop();
}

// Leer sensor de humedad de suelo
uint8_t Read_Soil(void) {
	uint8_t soil_value = 0;
	if (I2C_MasterStart()) {
		I2C_Master_Write((SLAVE_ENV_ADDR << 1) | I2C_WRITE);
		I2C_Master_Write(CMD_READ_SOIL);
		I2C_MasterRepeatedStart();
		I2C_Master_Write((SLAVE_ENV_ADDR << 1) | I2C_READ);
		I2C_MasterRead(&soil_value, I2C_NACK);
		I2C_MasterStop();
	}
	return soil_value;
}

/************************************************************************/
/* CONTROL SENSOR TSL2561 (LUZ - 0x39)                                  */
/************************************************************************/
void TSL2561_Init(void) {
	// Encendido usando la función genérica
	I2C_SendCommand(TSL2561_ADDR, 0x80 | 0x00); // Registro Control
	
	// Configuración de timing (Gain 16x)
	if (I2C_MasterStart()) {
		I2C_Master_Write((TSL2561_ADDR << 1) | I2C_WRITE);
		I2C_Master_Write(0x80 | 0x01); // Registro Timing
		I2C_Master_Write(0x12);        // 402ms, 16x gain
		I2C_MasterStop();
	}
}

uint16_t TSL2561_Read(void) {
	uint8_t low = 0, high = 0;
	if (I2C_MasterStart()) {
		I2C_Master_Write((TSL2561_ADDR << 1) | I2C_WRITE);
		I2C_Master_Write(0x80 | 0x20 | 0x0C); // CMD + WORD + DATA0L
		I2C_MasterRepeatedStart();
		I2C_Master_Write((TSL2561_ADDR << 1) | I2C_READ);
		I2C_MasterRead(&low, I2C_ACK);
		I2C_MasterRead(&high, I2C_NACK);
		I2C_MasterStop();
	}
	return (uint16_t)((high << 8) | low);
}


