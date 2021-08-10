/*
 * File:   main.c
 * Author: Pablo
 * Ejemplo de uso de I2C Esclavo
 * Created on 17 de febrero de 2020, 10:32 AM
 */
//*****************************************************************************
// Palabra de configuraci n
//*****************************************************************************
// CONFIG1
#pragma config FOSC = INTRC_NOCLKOUT// Oscillator Selection bits (RCIO oscillator: I/O function on RA6/OSC2/CLKOUT pin, RC on RA7/OSC1/CLKIN)
#pragma config WDTE = OFF       // Watchdog Timer Enable bit (WDT disabled and can be enabled by SWDTEN bit of the WDTCON register)
#pragma config PWRTE = OFF      // Power-up Timer Enable bit (PWRT disabled)
#pragma config MCLRE = OFF      // RE3/MCLR pin function select bit (RE3/MCLR pin function is digital input, MCLR internally tied to VDD)
#pragma config CP = OFF         // Code Protection bit (Program memory code protection is disabled)
#pragma config CPD = OFF        // Data Code Protection bit (Data memory code protection is disabled)
#pragma config BOREN = OFF      // Brown Out Reset Selection bits (BOR disabled)
#pragma config IESO = OFF       // Internal External Switchover bit (Internal/External Switchover mode is disabled)
#pragma config FCMEN = OFF      // Fail-Safe Clock Monitor Enabled bit (Fail-Safe Clock Monitor is disabled)
#pragma config LVP = OFF        // Low Voltage Programming Enable bit (RB3 pin has digital I/O, HV on MCLR must be used for programming)

// CONFIG2
#pragma config BOR4V = BOR40V   // Brown-out Reset Selection bit (Brown-out Reset set to 4.0V)
#pragma config WRT = OFF        // Flash Program Memory Self Write Enable bits (Write protection off)

// #pragma config statements should precede project file includes.
// Use project enums instead of #define for ON and OFF.

//*****************************************************************************
// Definici n e importaci n de librer as
//*****************************************************************************
#include <stdint.h>
#include <pic16f887.h>
#include "I2C_1.h"      //Llamar librerías
#include "ADC_3.h"
#include <xc.h>
//*****************************************************************************
// Definici n de variables
//*****************************************************************************
#define _XTAL_FREQ 8000000
uint8_t z;
uint8_t dato;
uint8_t anlec;
uint8_t nowadc;
//*****************************************************************************
// Definici n de funciones para que se puedan colocar despu s del main de lo 
// contrario hay que colocarlos todas las funciones antes del main
//*****************************************************************************
void setup(void);
void ADCL(void);
//*****************************************************************************
// C digo de Interrupci n 
//*****************************************************************************
void __interrupt() isr(void){
    if(INTCONbits.TMR0IF == 1){ //Interrupción del timer 0
        INTCONbits.TMR0IF = 0;
        TMR0 = 216;
        nowadc++; //Se va sumando el valor de la variable de la ADC cada vez que el timer 0 hace un ciclo
    }
    if (PIR1bits.ADIF == 1){ //Si la conversión AD fue realizada se regresa la bandera a 0
        PIR1bits.ADIF = 0;
        anlec = ADRESH; //Señal analógica        
    }
   if(PIR1bits.SSPIF == 1){ 

        SSPCONbits.CKP = 0;
       
        if ((SSPCONbits.SSPOV) || (SSPCONbits.WCOL)){
            z = SSPBUF;                 // Read the previous value to clear the buffer
            SSPCONbits.SSPOV = 0;       // Clear the overflow flag
            SSPCONbits.WCOL = 0;        // Clear the collision bit
            SSPCONbits.CKP = 1;         // Enables SCL (Clock)
        }

        if(!SSPSTATbits.D_nA && !SSPSTATbits.R_nW) {
            z = SSPBUF;                 // Lectura del SSBUF para limpiar el buffer y la bandera BF
            PIR1bits.SSPIF = 0;         // Limpia bandera de interrupci n recepci n/transmisi n SSP
            SSPCONbits.CKP = 1;         // Habilita entrada de pulsos de reloj SCL
            while(!SSPSTATbits.BF);     // Esperar a que la recepci n se complete
            PORTD = SSPBUF;             // Guardar en el PORTD el valor del buffer de recepci n
            __delay_us(10);
            
        }else if(!SSPSTATbits.D_nA && SSPSTATbits.R_nW){
            z = SSPBUF;
            BF = 0;
            SSPBUF = anlec;
            SSPCONbits.CKP = 1;
            __delay_us(10);
            while(SSPSTATbits.BF);
        }
       
        PIR1bits.SSPIF = 0;    
    }
}
//*****************************************************************************
// Main
//*****************************************************************************
void main(void) {
    setup();
    //*************************************************************************
    // Loop infinito
    //*************************************************************************
    while(1){
        ADCL(); //Función de conversión AD
        PORTB = anlec;
    }
    return;
}
//*****************************************************************************
// Funci n de Inicializaci n
//*****************************************************************************
void setup(void){
    ANSEL = 0;
    ANSELH = 0;
    ANSEL = 0b0000001; //Puertos analógicos y digitales
    TRISA0 = 1; //Inputs para las señales analógicas
    TRISB = 0;
    TRISD = 0;
    PORTA = 0;
    PORTB = 0;
    PORTD = 0;
    INTCONbits.T0IE = 1; //Interrupción del timer 0
    PIR1bits.ADIF = 0; //Función AD lisa para comenzar
    OSCCONbits.IRCF0 = 1; //Configuración del oscilador (8MHz)
    OSCCONbits.IRCF1 = 1;
    OSCCONbits.IRCF2 = 1;
    ADC_init(20, 20);   //Se escoge velocidad y canal para ADC
    ADCON1 = 0;
    TMR0 = 216; //Donde comienza el timer 0
    OPTION_REG = 0b01010111; //Configuración de timer 0 y pull ups
    PIE1bits.ADIE = 1;  //Habilitar ADC
    I2C_Slave_Init(0x50);   
}
void ADCL(void){ //Función de ADC
    if (nowadc > 5){ //Si la variable nowadc es mayor a 5 regresará a 0 y comienza la converción AD
     nowadc = 0;
     ADCON0bits.GO_nDONE = 1;
     ADC_init(1, 20); //Se escoge el canal
    }
}