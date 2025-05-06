/*
 * File:   Codigo_fuente.c
 * Author: Rodrigo C.C
 *
 * Created on May 3, 2025, 1:07 PM
 */


#define _XTAL_FREQ 4000000
#include <xc.h>
#include <pic16f877a.h>
#include "fusibles.h"
#include <stdbool.h>
#define Frec_visualizacion  5           //Frecuencia de visualizacion para la multiplexacion 200Hz
#define VER_NUMEROS 0                   //Opcion a escoger ver numeros 
#define VER_ESPERA  1                   //Opcion a escoger ver espera
#define BUTTON_ON_OFF   PORTBbits.RB0   //Boton on-off
#define BUTTON_35W      PORTBbits.RB1   //Boton para 35w y para disminuir el tiempo 
#define BUTTON_50W      PORTBbits.RB2   //Boton para 50w y para aumentar el tiempo
#define ACTIVA_35W      PORTEbits.RE0
#define ACTIVA_50W      PORTEbits.RE1
#define PORT_VISUALIZAR PORTD           //Puerto en el cual se conectaran los display

#define TIEMPO 1000     //1Seg          //Tiempo del reloj a temporizar(Cuando decrementa)
#define TIEMPO_MENS_OFF 1000            //Tiempo de visualizacion del mensaje 'OFF'
#define TIEMPO_MENS_35W 1000            //Tiempo de visualizacion del mensaje para una seleccion de 35w
#define TIEMPO_MENS_50W 1000            //Tiempo de visualizacion del mensaje para una seleccion de 50w
int unid = 0, dece = 0, cent = 0, cont = 0;//Variables globales para almacenar las unidades, decenas, centenas y constate
static bool state = 0;                  //Variable estado para el pulsador BUTON_ON_OF
volatile uint32_t milisegundos = 0;  // Contador variable global de ms
uint32_t anterior = 0;
uint32_t mens_off = 0;
uint32_t anterior_50w = 0;
uint32_t mens_anterior_50w = 0;
uint32_t anterior_35w = 0;
uint32_t mens_anterior_35w = 0;

//Array de datos que contienen los numeros del 0-9
static const uint8_t display[10] = {0xC0,   //0
                                    0xf9,   //1
                                    0xA4,   //2
                                    0xB0,   //3
                                    0x99,   //4
                                    0x92,   //5
                                    0x82,   //6
                                    0xF8,   //7
                                    0x80,   //8
                                    0x90};  //9
//Array de datos que contiene el mensaje 'on' y 'off'
static const uint8_t mensaje_on[3] = {0xA3, //O -->0
                                      0xAB, //N -->1
                                      0x8E};//f -->1        10001110
//Prototipo de funciones
void config_registros(void);        //Configura los registros de PORT, TRIS, y el TMR0
void visualizar(uint8_t VER, uint8_t unidad, uint8_t decena, uint8_t centena);//Se encarga de la visualizacion de display
void incremento_time(void); //Incrementa el tiempo 
void decremento_time(void); //Decrementa el tiempo
void mensajes(void);        //Se encarga de visualizar los diferentes mensajes
//estructura de datos enumerados para activar los display
typedef enum{
    off = 0,        //0 El display esta desactiva
    on_unidad,      //1 El display unidad esta activado
    on_decena,      //2 El display decena esta activado
    on_centena      //3 El display centena esta activado
}Activar_displays;  //Nombre de la estructura
//Estructura de datos enumerados para las opciones escogidas
typedef enum{
    ninguno = 0,    //0 No se a seleccionado ninguna opcion
    opcion_35w,     //1 Se a seleccionado la opcion para 35w
    opcion_50w      //2 Se a seleccionado la opcion para 50w
}opcion;            //Nombre de la estructura
//Estructura de datos enumerados para activar los mensajes
typedef enum{
    mensaje_espera = 0, //0 Es para escoger el mensaje de espera
    mensaje_35w,        //1 Es para escoger el mensaje de seleccion 35w
    mensaje_50w         //2 Es para escoger el mensaje de seleccion de 50w
}mensaje;               //Nombre de la estructura
//Inicializacion de los diferentes tipos de estructuras y datos
Activar_displays on_display = off;          //La variable on_display esta en off
mensaje mensaje_display = mensaje_espera;   //La variable mensaje_display esta en mensaje_espera
opcion seleccion = ninguno;                 //La variable seleccion esta en ninguno
//Funcion de interrupcion del TMR0
void __interrupt() INT_TMR0(void){
    if(INTCONbits.T0IF == 1){       //Verifica si la bandera de interrupcio por desbordamiento del TMR0 esta activado
        INTCONbits.T0IF = 0;        //Limpia la bander de interrupion del TMR0
        //Carga al TMR0 para 1ms
        TMR0 = 131;
        milisegundos ++;            //Incremenata la variable 
    }
}
void main(void) {
    config_registros();
    while(1){
        //Boton para Potencia de 35w y decremento del tiempo
        if(BUTTON_35W == 0 && state == false){
            __delay_ms(20);
            if(BUTTON_35W == 0 && state == false){
                cont--;
                if(seleccion == ninguno){
                    unid = 0;
                    dece = 2;
                    cent = 1;
                }
                if(seleccion != opcion_50w){    //Bloquea al boton de 50w para 
                    seleccion = opcion_35w;
                    mensaje_display = mensaje_35w;
                }
                
                while(BUTTON_35W == 0 && state == false){visualizar(VER_NUMEROS,on_unidad,on_decena,on_centena);}
            }
        }
        //Boton para potencia de 50w y incremento del tiempo
        if(BUTTON_50W == 0 && state == false){
            __delay_ms(20);
            if(BUTTON_50W == 0 && state == false){
                cont++;
                if(seleccion == ninguno){
                    unid = 0;
                    dece = 4;
                    cent = 1;
                }
                if(seleccion != opcion_35w){    //Bloquea al boton de 35w
                    seleccion = opcion_50w;
                    mensaje_display = mensaje_50w; 
                }
                while(BUTTON_50W == 0 && state == false){visualizar(VER_NUMEROS,on_unidad,on_decena,on_centena);}
            }
        }
        //Boton para iniciar el proceso
        if(BUTTON_ON_OFF == 0){
            __delay_ms(20);
            if(BUTTON_ON_OFF == 0){
                state = !state;
                while(BUTTON_ON_OFF == 0){visualizar(VER_NUMEROS,on_unidad,on_decena,on_centena);}
            }
        }
        //mensajes
        if(state == true && mensaje_display != mensaje_espera){
            mensajes();
        }
        //Activa rele para 35w
        if(state == true && seleccion == opcion_35w){
            ACTIVA_35W = true;
        }
        //Activa rele para 50w
        if(state == true && seleccion == opcion_50w){
            ACTIVA_50W = true;
        }
        //Si el boton esta en off
        if(state == false){
            ACTIVA_35W = 0;
            ACTIVA_50W = 0;
        }
        //Empieza el decremento de la variable cont cuando state = 1
        if(state == true){
            if(milisegundos - anterior >= TIEMPO) {
                anterior = milisegundos;
                cont--;
                decremento_time();
            }
        }
        //Con la variable cont que se decrementa a qui ingresa a una funcion que decrementa el tiempo
        if(state == false || seleccion == opcion_35w){//<--------------
            decremento_time();
        }
        if(state == false || seleccion == opcion_50w){//<------------
            incremento_time();
        }
        //Mensaje 'OFF' cuando halla acabado la temporizacion
        if(unid == 0 && dece == 0 && cent == 0){
            ACTIVA_35W = 0;
            ACTIVA_50W = 0;
            seleccion = ninguno;
            mensajes();
        }else{
            visualizar(VER_NUMEROS,on_unidad,on_decena,on_centena);//Visualiza los numeros cuando se a seleccionado una opcion
        }
    }
}
void config_registros(void){
    TRISA = 0x00;
    PORTA = 0x00;
    TRISE = 0x00;
    PORTE = 0x00;
    TRISB = 0xFF;
    PORTB = 0X00;
    TRISD = 0X00;
    PORTD = 0X00;
    OPTION_REGbits.nRBPU = 0;
    //Configuracion del TMR0
    OPTION_REGbits.PS = 0b010;
    OPTION_REGbits.PSA = 0;
    OPTION_REGbits.T0CS = 0;
    INTCONbits.GIE = 1;
    INTCONbits.T0IE = 1;
    //Carga al TMR0 para 1ms
    TMR0 = 131;
}
void visualizar(uint8_t VER, uint8_t unidad, uint8_t decena, uint8_t centena){
    //Unidad
    if(on_unidad == unidad){
        PORTA = 0XFF;     // Primero apagar todos los displays para evitar ghosting
        switch(VER){
            case VER_NUMEROS:
                PORT_VISUALIZAR = display[unid];
                break;
            case VER_ESPERA:
                PORT_VISUALIZAR = mensaje_on[unid];
                break;
        }
        PORTA = (unsigned char)(~(1<<0));   //Esto le dice al compilador: 
                                            //"Sé lo que estoy haciendo, convierte 
                                            //esto a unsigned char explícitamente".
    }
    __delay_ms(Frec_visualizacion);
    //Decena
    if(on_decena == decena){
        PORTA = 0XFF;     // Primero apagar todos los displays para evitar ghosting
        switch(VER){
        case VER_NUMEROS:
            PORT_VISUALIZAR = display[dece];
            break;
        case VER_ESPERA:
            PORT_VISUALIZAR = mensaje_on[dece];
            break;
        }
        PORTA = (unsigned char)(~(1<<1));   //Esto le dice al compilador: 
                                            //"Sé lo que estoy haciendo, convierte 
                                            //esto a unsigned char explícitamente".
    }
    __delay_ms(Frec_visualizacion);
    //Centena
    if(on_centena == centena){
        PORTA = 0XFF;     // Primero apagar todos los displays para evitar ghosting
        switch(VER){
            case VER_NUMEROS:
                PORT_VISUALIZAR = display[cent];
                break;
            case VER_ESPERA:
                PORT_VISUALIZAR = mensaje_on[cent];
                break;
        }
        PORTA = (unsigned char)(~(1<<2));   //Esto le dice al compilador: 
                                            //"Sé lo que estoy haciendo, convierte 
                                            //esto a unsigned char explícitamente".
    }
    __delay_ms(Frec_visualizacion);
}
void incremento_time(){
    unid = cont;
        if(cont>= 10){
            dece++;
            cont = 0;
            //unid = 0;
            if(dece >= 10){
                cent++;
                dece = 0;
                if(cent >=10){
                    cent = 0;
                }
            }
        }
}
void decremento_time(void){
    unid = cont;
    if(unid < 0){
        cont = 9;
        unid = cont;
        dece--;
        if(dece < 0){
            dece = 9;
            //cont = 9;
            cent--;
            if(cent < 0){
                cent = 0;
                unid = 0;
                cont = 0;
                dece = 0;
                state = false;
            }
        }
    }
}
void mensajes(void){
    //Variables que se guardan
    static int guarda_u, guarda_d, guarda_c;
    guarda_u = unid;
    guarda_d = dece;
    guarda_c = cent;
   switch(mensaje_display){
       case mensaje_espera:
           //mens_off = milisegundos;
           //while(milisegundos - mens_off <= TIEMPO_MENS_OFF){
               unid = 2;
               dece = 2;
               cent = 0;
               visualizar(VER_ESPERA,on_unidad, on_decena, on_centena);
           //}
           break;
       case mensaje_35w:
           mens_anterior_35w = milisegundos;
           while(milisegundos - mens_anterior_35w <= TIEMPO_MENS_35W){
               unid = 5;
               dece = 3;
               visualizar(VER_NUMEROS,on_unidad,on_decena,off);
           }
           anterior_35w = milisegundos;
           while(milisegundos - anterior_35w <= TIEMPO_MENS_35W){
               unid = 1;
               dece = 0;
               visualizar(VER_ESPERA,on_unidad,on_decena,off);
           }
           mensaje_display = mensaje_espera;
           break;
       case mensaje_50w:
           mens_anterior_50w = milisegundos;
           while(milisegundos - mens_anterior_50w <= TIEMPO_MENS_50W){
               unid = 0;
               dece = 5;
               visualizar(VER_NUMEROS, on_unidad, on_decena, off);
           }
           
           anterior_50w = milisegundos;
           while(milisegundos - anterior_50w <= TIEMPO_MENS_50W){
               unid = 1;
               dece = 0;
               visualizar(VER_ESPERA,on_unidad, on_decena, off);
           }
           mensaje_display = mensaje_espera;
           break;
   }
   unid = guarda_u;
   dece = guarda_d;
   cent = guarda_c;
}