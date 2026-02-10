/*
 * TIMER1.c (REV. FINAL)
 * Librería para utilizar TIMER1 en ATMEGA328P
 * Adaptable para otros microcontroladores
 *
 * Creada: 10/02/2026 15:06:40
 * Autor: mario
 * Modificado: Gemini (Correcciones de bits y math)
 */ 

#include "TIMER1.h"

// Inicialización de TIMER1
// Los prefijos s denotan "selector"
void Timer1Init(TIMER1_MODE sMode, TIMER1_PRESCALERS sPrescaler, TIMER1_EN_INTERRUPTS enInterrupts, TIMER1_COMPARE_OUTPUT_MODE sComOutMode)
{
    // Limpiar registros previos
    TCCR1A = 0; TCCR1B = 0; TIMSK1 = 0;

    // Aplicar máscara de bits a selectores de modo y prescaler
    uint8_t mode10 = sMode & 0x03;          // Bits 1 y 0 para TCCR1A
    
    // CORRECCIÓN: Desplazamiento de bits WGM13 y WGM12
    // En el enum están en pos 3 y 2. En TCCR1B van en pos 4 y 3.
    // Se desplaza 1 a la izquierda (<< 1), no 3.
    uint8_t mode32 = (sMode & 0x0C) << 1;   
    
    uint8_t prescaler = sPrescaler & 0x07;  // Obtener últimos 3 bits de prescaler
    uint8_t cout_mode = (sComOutMode & 0x03) << 6;
    
    TCCR1A |= mode10 | cout_mode;
    TCCR1B |= mode32 | prescaler;
    
    // Habilitar interrupciones de manera selectiva y segura
    if(enInterrupts == TIMER1_INTERRUPTS_ENABLED) 
    {
        // Interrupciones por Comparación (OCR1A)
        if (sMode == TIMER1_MODE4_CTC_OCR1A || sMode == TIMER1_MODE15_FASTPWM_OCR1A || sMode == TIMER1_MODE9_PWM_PCF_OCR1A || sMode == TIMER1_MODE11_PWM_PC_OCR1A) 
        {
            TIMSK1 |= (1 << OCIE1A);
        }
        // Interrupciones por Captura (ICR1) - Usado como TOP en otros modos
        else if (sMode == TIMER1_MODE12_CTC_ICR1 || sMode == TIMER1_MODE14_FASTPWM_ICR1A || sMode == TIMER1_MODE8_PWM_PCF_ICR1 || sMode == TIMER1_MODE10_PWM_PC_ICR1)
        {
            TIMSK1 |= (1 << ICIE1); 
        }
        // Overflow para modo Normal
        else if (sMode == TIMER1_MODE0_NORMAL)
        {
            TIMSK1 |= (1 << TOIE1);
        }
    }
    
    // Nota: Configuración de pines físicos (DDR) se suele dejar al usuario
    // pero si deseas forzarlos aquí:
    // DDRB |= (1 << DDB1) | (1 << DDB2); // OC1A y OC1B en Atmega328p
}

// Función interna para leer el modo configurado
static TIMER1_MODE getTimer1WGMode(void)
{
    uint8_t mode10 = TCCR1A & 0x03;
    // Recuperar bits WGM13(4) y WGM12(3) y devolverlos a posición 3 y 2
    uint8_t mode32 = (TCCR1B & 0x18) >> 1; 
    TIMER1_MODE wgMode = (TIMER1_MODE) (mode10 | mode32);
    return  wgMode;
}

// Establecer frecuencia, calculando automáticamente el TOP según el modo
void Timer1SetFrequency(uint16_t frequency)
{
    // Determinar prescaler
    // CORRECCIÓN: Máscara 0x07 para leer los 3 bits
    uint8_t sPrescaler = TCCR1B & 0x07;
    uint32_t prescaler = 1; // Usamos uint32_t para evitar desbordes en multiplicaciones
    
    switch(sPrescaler)
    {
        case 1:  prescaler = 1;     break;
        case 2:  prescaler = 8;     break;
        case 3:  prescaler = 64;    break;
        case 4:  prescaler = 256;   break;
        case 5:  prescaler = 1024;  break;
        default: prescaler = 1;     break;
    }
    
    // --- CÁLCULOS DE VARIABLES TEMPORALES ---
    // Usamos (uint32_t)F_CPU para asegurar precisión
    
    // 1. CTC: f = F_CPU / (2 * N * (1+OCR)). Despejamos OCR.
    uint16_t temp_CTC       = (F_CPU / (2 * prescaler * frequency)) - 1;
    
    // 2. FAST PWM: f = F_CPU / (N * (1+TOP)). Despejamos TOP.
    uint16_t temp_FASTPWM   = (F_CPU / (prescaler * frequency)) - 1;
    
    // 3. PHASE CORRECT PWM: f = F_CPU / (2 * N * TOP). Despejamos TOP.
    // (No lleva el -1 en la fórmula estándar de Phase Correct)
    uint16_t temp_PWM_PC    = (F_CPU / (2 * prescaler * frequency));
    
    // 4. PHASE & FREQ CORRECT: Igual que Phase Correct
    uint16_t temp_PWM_PCF   = (F_CPU / (2 * prescaler * frequency));
    
    
    TIMER1_MODE wgMode = (TIMER1_MODE) getTimer1WGMode();
    
    // --- ASIGNACIÓN DE VALORES SEGÚN EL MODO (CASE COMPLETO) ---
    switch(wgMode)
    {
        // Modos que usan OCR1A como tope
        case TIMER1_MODE4_CTC_OCR1A:        OCR1A = temp_CTC;       break;  
        case TIMER1_MODE9_PWM_PCF_OCR1A:    OCR1A = temp_PWM_PCF;   break;
        case TIMER1_MODE11_PWM_PC_OCR1A:    OCR1A = temp_PWM_PC;    break;
        case TIMER1_MODE15_FASTPWM_OCR1A:   OCR1A = temp_FASTPWM;   break;

        // Modos que usan ICR1 como tope
        case TIMER1_MODE12_CTC_ICR1:        ICR1 = temp_CTC;        break;
        case TIMER1_MODE8_PWM_PCF_ICR1:     ICR1 = temp_PWM_PCF;    break;
        case TIMER1_MODE10_PWM_PC_ICR1:     ICR1 = temp_PWM_PC;     break;
        case TIMER1_MODE14_FASTPWM_ICR1A:   ICR1 = temp_FASTPWM;    break;
        
        // Modos fijos (8, 9, 10 bits) no pueden cambiar frecuencia arbitraria
        default:
            // No hacemos nada o reseteamos a max
            break;
    }
}
