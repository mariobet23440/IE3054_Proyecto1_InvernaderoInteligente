/*
 * EjemploTimer1.c
 *
 * Created: 10/02/2026 16:04:27
 * Author : mario
 * Modified: Gemini
 */ 

#include <avr/io.h>
#include <avr/interrupt.h> // Necesario para sei() e ISR
#include "TIMER1.h"

int main(void)
{
    // Inicializar en MODO CTC (Clear Timer on Compare Match)
    // Usamos Prescaler 256. 
    // Cálculo: 16MHz / 256 = 62,500 ticks por segundo. Esto cabe en 16 bits (65535).
    // Si usáramos Prescaler 1, necesitaríamos 16 millones de ticks (imposible en 16 bits).
    Timer1Init(TIMER1_MODE4_CTC_OCR1A, TIMER1_PRESCALER256, TIMER1_INTERRUPTS_ENABLED, TIMER1_COMP_OUT_MODE0);
    
    // Establecer frecuencia de interrupción a 1 Hz (1 seg)
    Timer1SetFrequency(1);
    
    // Configurar LED en PB5
    DDRB |= (1 << DDB5);
    PORTB &= ~(1 << PORTB5); // Iniciar apagado
    
    // Habilitar interrupciones globales (¡Muy importante!)
    sei();
    
    /* Replace with your application code */
    while (1) 
    {
        // El bucle está vacío porque todo lo maneja la interrupción
    }
}

// Vector de interrupción para Comparación A (Modo CTC)
ISR(TIMER1_COMPA_vect)
{   
    // Toggle (invertir estado) del pin PB5
    // Usar XOR es más eficiente y limpio que leer, enmascarar y escribir
    PORTB ^= (1 << PORTB5);
}