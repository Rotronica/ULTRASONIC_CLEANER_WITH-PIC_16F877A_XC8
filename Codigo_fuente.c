/*
 * File:   Codigo_fuente.c
 * Author: Rodrigo C.C
 *
 * Created on May 3, 2025, 1:07 PM
 * El siguiente program realiza el control de una limpiadora ultrasonica
 * este limpiador contien un tres botones el cual dos botones controla las potencias
 * y el ajuste del tiempo 
 * 
 */


#define _XTAL_FREQ 4000000
#include <xc.h>
#include <pic16f877a.h>
#include "fusibles.h"
#include <stdbool.h>

#define MUX_TIME            5       //Tiempo de multiplexado
#define PINES_MUX           PORTA   //Pines que activaran los display
#define DISPLAY_MASK        0x0F    // Bits 0-3 para displays(Para la mascara de seguridad)
#define PORT_VISUALIZADOR   PORTD   //Puerto por donde se visualizara los displays
#define NUM_DISPLAY         3       //Cantidad de display utilizados 

uint32_t milisegundos = 0;
uint8_t display_state = 0;  //Estado de cada pin que activa cada display
// Tabla de caracteres (ánodo común)
static const uint8_t DATOS[] = {
    // Números 0-9 (invertidos)
    0xC0, // 0 (0x3F invertido)
    0xF9, // 1 (0x06 invertido)
    0xA4, // 2 (0x5B invertido)
    0xB0, // 3 (0x4F invertido)
    0x99, // 4 (0x66 invertido)
    0x92, // 5 (0x6D invertido)
    0x82, // 6 (0x7D invertido)
    0xF8, // 7 (0x07 invertido)
    0x80, // 8 (0x7F invertido)
    0x90, // 9 (0x6F invertido)
    
    // Letras A-Z (invertidas)
    0x88, // A (0x77 invertido)
    0x83, // b (0x7C invertido)
    0xC6, // C (0x39 invertido)
    0xA1, // d (0x5E invertido)
    0x86, // E (0x79 invertido)
    0x8E, // F (0x71 invertido)
    0xC2, // G (0x3D invertido)
    0x89, // H (0x76 invertido)
    0xCF, // I (0x30 invertido)
    0xE1, // J (0x1E invertido)
    0x89, // K (igual a H)
    0xC7, // L (0x38 invertido)
    0xC8, // M (0x37 invertido)
    0xAB, // n (0x54 invertido)
    0xC0, // O (0x3F invertido)
    0x8C, // P (0x73 invertido)
    0x98, // q (0x67 invertido)
    0xAF, // r (0x50 invertido)
    0x92, // S (igual a 5)
    0x87, // t (0x78 invertido)
    0xC1, // U (0x3E invertido)
    0xC1, // V (igual a U)
    0xD5, // W (0x2A invertido)
    0x89, // X (igual a H)
    0x91, // Y (0x6E invertido)
    0xA4, // Z (igual a 2)
    
    // Símbolos especiales (invertidos)
    0xBF, // Guión (-) (0x40 invertido)
    0x7F, // Punto decimal (.) (0x80 invertido)
    0x9C, // Grados (°) (0x63 invertido)
    0xFF  // Espacio (apagado) (0x00 invertido)
};
typedef enum{
    CHAR_0 = 0,
    CHAR_3 = 3,
    CHAR_5 = 5,
    CHAR_F = 15,
    CHAR_O = 24,
    CHAR_CLEAR = 39,
    CHAR_GUION = 36
}CARACTER;

CARACTER display_buffer[NUM_DISPLAY] = {CHAR_GUION,CHAR_GUION,CHAR_GUION};

/* ============================================
 *      Prototipo de funcion
 * ============================================
 */
void configurar_hardware(void);
void configurar_tmr0(void);
uint32_t millis(void);
//void __interrupt() INT_TMR0(void);
void visualizar_display(void);
void configurar_hardware(void){
    TRISA = 0x00;
    TRISD = 0x00;
    PORTA = 0x00;
    PORTD = 0x00;
}
void configurar_tmr0(void){
    //Configurarndo el TMR0
    OPTION_REGbits.PS = 0b010; //Conu un prescaler 1:8
    OPTION_REGbits.PSA = 0;     //Asignado al TMR0
    OPTION_REGbits.T0CS = 0;    //Modo temporizador
    //Configurar Interrupcion TMR0
    INTCONbits.GIE = 1;
    INTCONbits.T0IE = 1;
    TMR0 = 131; //Carga al TMR0 para una temporizacion de 1ms
}
void __interrupt() INT_TMR0(void){
    if(INTCONbits.T0IF == 1){
        INTCONbits.T0IF = 0;    //Limpiar la bandera de interrupcion
        TMR0 = 131;             //Se carga nuevamente el valor para 1ms
        milisegundos++;
    }
}
uint32_t millis(void){
    uint32_t m;
    INTCONbits.GIE = 0;
    m = milisegundos;
    INTCONbits.GIE = 1;
    return m;
}
void visualizar_display(void){
    static uint32_t last_mux = 0;
    uint32_t now = millis();
    //Multiplexacion cada 5ms(200Hz de tasa de refresco total)
    if((now - last_mux) >= MUX_TIME){
        last_mux = now;
        //Apagar todos los display
        PINES_MUX &= ~DISPLAY_MASK;  // Solo afecta bits de displays
        //Mostrar el caracter actual
        PORT_VISUALIZADOR = DATOS[display_buffer[display_state]];//Aqui dice que primero ingrese al array display_buffer tomara un numero y con ese numero tomara el valor del array DATOS 
        //Activar el display correspondiente
        PINES_MUX |= ~(1 << display_state); // Enciende el bit correspondiente para cada display con seguridad implementando un or como mascara esto evitara el ghosting
        //Avanzar al siguiente display
        display_state = (display_state + 1) % NUM_DISPLAY; //Con esta sentencia aumente display_state 1,2,3,4 a la vez controla el limite que es la cantidad de display utilizados cuando llega a 4 automaticamente lo lleva al primer displey   
    }
}
void main(void){
    configurar_hardware();
    configurar_tmr0();
    while(true){
        visualizar_display();
    }
}