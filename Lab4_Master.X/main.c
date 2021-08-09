/*
 * File:   main.c
 * Author: Pablo Herrarte
 * Ejemplo de uso de I2C Master
 * Created on 9 de agosto de 2021
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
#include "I2C.h"
#include "LCD.h"
#include "ASCII_2.h"
#include <xc.h>
//*****************************************************************************
// Definici n de variables
//*****************************************************************************
#define _XTAL_FREQ 8000000
uint8_t POT = 0;
uint8_t CONT = 0;
uint8_t TEMP = 0;
uint8_t NUM = 0;
char Cen1 = 0;  //Dígito de potenciómetros, contador y temperatura
char Cen2 = 0;
char Cen3 = 0;
char Dec1 = 0;
char Dec2 = 0;
char Dec3 = 0;
char Un1 = 0;   
char Un2 = 0;
char Un3 = 0;
char AC1 = 0;
char AC2 = 0;
char AC3 = 0;
char AD1 = 0;
char AD2 = 0;
char AD3 = 0;
char AU1 = 0;
char AU2 = 0;
char AU3 = 0;
//*****************************************************************************
// Definici n de funciones para que se puedan colocar despu s del main de lo 
// contrario hay que colocarlos todas las funciones antes del main
//*****************************************************************************
void setup(void);
void LECT1(void);   //Separación de dígitos y lectura del potenciómetro 1
void LECT2(void);   //Separación de dígitos y lectura del potenciómetro 2
void LECT3(void);   //Separación de digitos y lectura del contador
const char* conver(char A, char B, char C); //Datos que recibirá la LCD
//*****************************************************************************
// Main
//*****************************************************************************
void main(void) {
    setup();
    Lcd_Init(); //Inicialización de 8 bits para LCD
    while(1){
        Lcd_Set_Cursor(1,1);    //Cursor en primera línea
        Lcd_Write_String("S1    S2    S3"); //Escribir S1 S2 S3
        Lcd_Set_Cursor(2,1);    //Cursor en segunda línea
        Lcd_Write_String(conver(AC3, AD3, AU3));    //Escribir los datos para el LCD con esa función
        LECT1();    
        LECT2();
        LECT3();
        
        I2C_Master_Start();
        I2C_Master_Write(0x50);
        I2C_Master_Write(0);
        I2C_Master_Stop();
        __delay_ms(10);
       
        I2C_Master_Start();
        I2C_Master_Write(0x51);
        POT = I2C_Master_Read(0);
        I2C_Master_Stop();
        __delay_ms(10);
        
        I2C_Master_Start();
        I2C_Master_Write(0x60);
        I2C_Master_Write(0);
        I2C_Master_Stop();
        __delay_ms(10);
       
        I2C_Master_Start();
        I2C_Master_Write(0x61);
        CONT = I2C_Master_Read(0);
        I2C_Master_Stop();
        __delay_ms(10);
        
        I2C_Master_Start();
        I2C_Master_Write(0x80);
        I2C_Master_Write(0xF3);
        I2C_Master_Stop();
        __delay_ms(100);
        
        I2C_Master_Start();
        I2C_Master_Write(0x81);
        TEMP = I2C_Master_Read(0);
        I2C_Master_Stop();
        __delay_ms(100);
    }
    return;
}
//*****************************************************************************
// Funci n de Inicializaci n
//*****************************************************************************
void setup(void){
    ANSEL = 0;
    ANSELH = 0;
    TRISA = 0;
    TRISB = 0;
    TRISD = 0;
    TRISE = 0;
    PORTA = 0;
    PORTB = 0;
    PORTD = 0;
    PORTE = 0;
    OSCCONbits.IRCF0 = 1; //Configuración del oscilador (8MHz)
    OSCCONbits.IRCF1 = 1;
    OSCCONbits.IRCF2 = 1;
    I2C_Master_Init(100000);        // Inicializar Comuncaci n I2C
}
const char* conver(char AC3, char AD3, char AU3){   //Datos que recivirá la LCD
    char temporal[16];
    temporal[0] = AC1;
    temporal[1] = 0x2E;
    temporal[2] = AD1;
    temporal[3] = AU1;
    temporal[4] = 0x76;
    temporal[5] = 0x20;
    temporal[6] = 0x20;
    temporal[7] = AD2;
    temporal[8] = AU2;
    temporal[9] = 0X20;
    temporal[10] = 0x20;
    temporal[11] = AC3;
    temporal[12] = AD3;
    temporal[13] = AU3;
    temporal[14] = 0xDF;
    temporal[15] = 67;
    return temporal;
}
void LECT1(void){ //Para el primer puerto analógica
    Cen1 = POT/51; //Se crea bit de centena de 5V a 0V
    Dec1 = ((10*POT)/51-Cen1*10); //Se crea el bit de decena
    Un1  = (100*POT)/51-(Cen1*100+Dec1*10); //Se crea el bit de unidad
    AC1 = num_ascii(Cen1); //Se hace conversión de números a su forma ascii para que lea la LCD
    AD1 = num_ascii(Dec1);
    AU1 = num_ascii(Un1);
}
void LECT2(void){ //Se hace lo mismo para el segundo potenciómetro
    Dec2 = CONT/10;
    Un2 = CONT-Dec2*10;
    AD2 = num_ascii(Dec2);
    AU2 = num_ascii(Un2);
}
void LECT3(void){
    if (TEMP>=68){
        NUM = 24*TEMP/35-1632/35;
        Cen3 = NUM/100;
        Dec3 = (NUM-Cen3*100)/10;
        Un3 = NUM-Cen3*100-Dec3*10;
        AC3 = num_ascii(Cen3);
        AD3 = num_ascii(Dec3);
        AU3 = num_ascii(Un3);
    }
    else{
        NUM = (877/19)-13*TEMP/19;
        AC3 = 0x2D;
        Dec3 = NUM/10;
        Un3 = NUM - Dec3*10;
        AD3 = num_ascii(Dec3);
        AU3 = num_ascii(Un3);
    }
}