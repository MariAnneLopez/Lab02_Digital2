/* 
 * File:   Lab_2_main.c
 * Author: Marian Lopez
 *
 * Created on 18 de noviembre de 2023, 08:53 PM
 */

// CONFIG1
#pragma config FOSC = INTRC_CLKOUT// Oscillator Selection bits (RC oscillator: CLKOUT function on RA6/OSC2/CLKOUT pin, RC on RA7/OSC1/CLKIN)
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

#include <xc.h>
#include <stdlib.h>
#include <stdio.h>
#include "ADC.h"
#include "LCD.h"
#include "Map_adc.h"
#include "UART.h"

#define _XTAL_FREQ 8000000
#define RS RD2
#define EN RD3
#define D4 RD4
#define D5 RB5
#define D6 RD6
#define D7 RD7

char volt[4];
char cont[8];
char hiper;
int V;
int V_out;
int contador = 0;

// Prototipos
void setup (void);

void __interrupt() isr(void)
{
    if (ADIF)
    {
        adc_change_channel(1);
        V = adc_read();
        V_out =  map (V, 0, 255, 0, 5);
        ADIF =0;                            // Apagar la bandera de interrupcion
        return;
    }
    
    if (RCIF)
    {
        char opcion = RCREG;
        switch (opcion)
        {
            case '1': // En caso que presionemos 1, nos mande a leer el valor del potenciometro
                UART_write_char(volt);
                break; 
            case '+': // En caso que ingresemos +, incrementa el contador del puerto B
                contador++;
                break;
            case '-': // En caso que ingresemos -, incrementa el contador del puerto B
                contador--;
                break;
            default:
                UART_write_char("Opcion no definida\r");
                while(RCREG != 13); // Esperar hasta que se presione Enter
        }
    }

}

void main(void)
{
    setup();
    unsigned int a;
    Lcd_Init();
    char texto[] = "1. Leer potenciometro\r+. Incrementar puerto B\r-. Decrementar puerto B\r"; // Cadena de caracteres a enviar
    while (1)
    {
        if(ADCON0bits.GO == 0)
        {
           __delay_ms(10);
            ADCON0bits.GO = 1;  
        }
        
        UART_write_char(texto); // Llamada a la cadena
        
        Lcd_Clear();
        Lcd_Set_Cursor(1,1);
        Lcd_Write_String("Voltaje");
        Lcd_Set_Cursor(2,1);
        sprintf(volt, "%d\r", V_out);
        Lcd_Write_String(volt);
        
        Lcd_Set_Cursor(1,9);
        Lcd_Write_String("Contador");
        Lcd_Set_Cursor(2,9);
        sprintf(cont, "%d\r", contador);
        Lcd_Write_String(cont);
        
        if (!PORTCbits.RC0){
            while(!PORTCbits.RC0){ //Antirrebote
            }
            contador = contador + 1;
        }
        if (!PORTCbits.RC3){
            while(!PORTCbits.RC3){ //Antirrebote
            }
            contador = contador - 1;
        }
    
        PORTB = contador;
        __delay_ms(500);

    }
     return;
}

void setup(void)
{
    TRISA = 0b00000011; // Potenciometro
    TRISB = 0;
    TRISD = 0;
    TRISC = 0xFF; // Push buttons
    TRISE = 0;
    
    PORTA = 0;
    PORTB = 0;
    PORTC = 0;
    PORTD = 0;
    PORTE = 0;
    
    ANSEL = 0b00000011;
    ANSELH= 0;
    
    OSCCONbits.IRCF =0b111; 
    OSCCONbits.SCS = 1; 
    
    INTCONbits.GIE = 1;
    INTCONbits.PEIE = 1;
    INTCONbits.T0IE = 1; // Habilita las interrupciones por TMR0
    INTCONbits.T0IF = 0; // Apaga la bandera por TMR0
    PIE1bits.ADIE = 1; // Habilita las interrupciones por ADC
    PIR1bits.ADIF = 0; // Apaga la bandera por ADC
    PIE1bits.RCIE = 1; // Habilita las interrupciones por UART
    PIR1bits.RCIF = 0; // Apaga la bandera por UART
    
    adc_init(1);
    UART_RX_config(9600); 
    UART_TX_config(9600);
    
    return;
}

