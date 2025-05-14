/*
 * File:   Codigo_fuente.c
 * Author: Rodrigo C.C
 *
 * Created on May 3, 2025, 1:07 PM
 * El siguiente programa realiza el control de una limpiadora ultrasonica
 * este limpiador contiene tres botones estos dos botones controlan las potencias
 * y el ajuste del tiempo.
 * 
 */


#define _XTAL_FREQ 4000000
#include <xc.h>
#include <pic16f877a.h>
#include "fusibles.h"
#include <stdbool.h>

#define MUX_TIME            5       //Tiempo de multiplexado
#define TIEMPO_MOVIMIENTO   200     //Tiempo que se encarga para la visualizacion del movimiento de espera
#define PINES_MUX           PORTA   //Pines que activaran los display
#define DISPLAY_MASK        0xF8    // Bits 0-3 para displays(Para la mascara de seguridad)
#define PORT_VISUALIZADOR   PORTD   //Puerto por donde se visualizara los displays
#define NUM_DISPLAY         3       //Cantidad de display utilizados 
//Botones 
#define BUTTON_START_STOP   PORTBbits.RB0
#define BUTTON_35W          PORTBbits.RB1 
#define BUTTON_50W          PORTBbits.RB2
//Salidas
#define RELAY_35W           PORTCbits.RC0
#define RELAY_50W           PORTCbits.RC1

//Variables
uint32_t milisegundos = 0;
uint8_t display_state = 0;  //Estado de cada pin que activa cada display
bool on_off = false;
static bool temporizador_activo = true; //Variable para activar o apagar la temporizacion
static uint8_t cont = 0;    //Variable para el control de veces de parpadeo de un mensaje
//Variables para el control de la temporizacion
static uint8_t unidades = 0;
static uint8_t decenas = 8;
static uint8_t centenas = 1;

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
    0xFF, // Espacio (apagado) (0x00 invertido)
    //ESPECIALES LIMPIADOR
    0xFE,   //SOMBRERO
    0xF7,   //GUIO_BAJO
    0xF9,   //BARRA_VERTICAL
    0xE7,   // |_ Esquina izquierda abajo
    0xFC,   // -| Esquina derecha arriba
    0xDE,   // |- Esquina izquierda arriba
    0xF3,   // _| Esquina derecha abajo
    0xF6    // _- SOMBRERO Y GUION BAJO
};
//Estructura enum para la seleccion de mensajes para los tiempos
typedef enum{
    ms_tiempo_ninguno = 0,
    ms_tiempo_35w,
    ms_tiempo_50w
}MENSAJE_TIEMPO;
MENSAJE_TIEMPO ms_tiempo = ms_tiempo_ninguno;

//Estructura  enum para la seleccion de modos de seleccion de potencia
typedef enum{
    modo_ninguno = 0,
    modo_35w,
    modo_50w
}MODO;
MODO modo_proceso = modo_ninguno;

//Estructura enum para mensajes de seleccion de potencia
typedef enum{
    MS_INIT = 0,
    MS_P35W,
    MS_P50W,
    MS_ON
}MENSAJES;
MENSAJES ms_state = MS_INIT;

//Estructura enum para crear los caracteres de forma mas visual y amigable
typedef enum{
    CHAR_0 = 0,
    CHAR_3 = 3,
    CHAR_5 = 5,
    CHAR_F = 15,
    CHAR_N = 23,
    CHAR_O = 24,
    CHAR_I = 18,
    CHAR_P = 25,
    CHAR_CLEAR = 39,
    CHAR_GUION = 36,
    CHAR_GSOMB = 40,
    CHAR_GBAJO = 41,
    CHAR_BVERTICAL = 42,
    CHAR_EZIZBAJO = 43,
    CHAR_EZDEARRIBA = 44,
    CHAR_EZIZARRIBA = 45,
    CHAR_EZDEABAJO = 46,
    CHAR_SOM_GBAJO = 47
}CARACTER;
CARACTER display_buffer[NUM_DISPLAY] = {CHAR_O,CHAR_F,CHAR_F};

/* ============================================
 *      Prototipo de funcion
 * ============================================
 */
void configurar_hardware(void);             //Configurar registros port y tris
void configurar_tmr0(void);                 //Configuracion del modulo TMRO
uint32_t millis(void);                      //Creacion de la funcion millis()
void __interrupt() INT_TMR0(void);          //Funcion que atiende la subrutina de interrupcion por TMR0
void visualizar_display(void);              //Funcion que se encarga de visualizar el display y multiplexar 
void mostrar_mensajes(const CARACTER *mensaje, uint8_t longitud);   //Funcion que se encarga de procesar el mensaje que creemos
void diferentes_mensajes(void);             //Funcion que selecciona los diferentes mensajes para diferentes opciones
void efecto_titilar(const CARACTER *mensaje, uint8_t longitud, uint32_t tiempo);    //Funcion para hacer parpadear un mensaja
void botones(void);                         //Funcion que se encarga de procesar los pulsadores
void proceso_on(void);                      //Funcion que se encarga de dar inicio al proceso segun las opciones que se hallan escogido
void actualizar_temporizador(void);         //Funcion que se encarga de actualizar la temporizacion en decremento cuando se da inicio al proceso
void convertir_tiempo_a_display(void);      //Funcion que se encarga de convertir los datos de la funcio actualizar_temporizador() para poder ser visualizado en los display
void verificar_fin_temporizador(void);      //Funcion que se encarga de detener el proceso y mostrar un mensaje de "off" cuando la temporizacion halla terminado
void incremento_temporizador(void);         //Funcion que se encarga de incrementar de forma manual
void decremento_temporizador(void);         //Funcion que se encarga de decrementar de forma manual
void condiciones_iniciales(void);           //Funcion que contiene todas las variables en las condiciones iniciales

/*
 * =============================================================================
 *              Desarrollo del Codigo
 * =============================================================================
 */
void configurar_hardware(void){
    //Los puertos A,C y D se configura como salida de señal 
    TRISA = 0x00;
    TRISD = 0x00;
    TRISC = 0X00;
    OPTION_REGbits.nRBPU = 0;   //Se acftivan las resistencias pull-ups
    TRISB = 0XFF;   //Puerto B se configura para entrada de señal
    //Se limpian los puertos
    PORTA = 0x07;
    PORTD = 0xFF;
    PORTB = 0X00;
    PORTC = 0X00;
}
void configurar_tmr0(void){
    //Configurarndo el TMR0
    OPTION_REGbits.PS = 0b010; //Con un prescaler 1:8
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

//Funcion millis
uint32_t millis(void){
    uint32_t m;
    INTCONbits.GIE = 0;
    m = milisegundos;
    INTCONbits.GIE = 1;
    return m;
}

//Funcion que visualiza y multiplexa los mensajes
void visualizar_display(void){
    static uint32_t last_mux = 0;
    uint32_t now = millis();
    //Multiplexacion cada 5ms(200Hz de tasa de refresco total)
    if((now - last_mux) >= MUX_TIME){
        last_mux = now;
        //Apagar todos los display
        PINES_MUX = 0XFF;  // Solo afecta bits de displays
        //Mostrar el caracter actual
        PORT_VISUALIZADOR = DATOS[display_buffer[display_state]];//Aqui dice que primero ingrese al array display_buffer tomara un numero y con ese numero tomara el valor del array DATOS 
        //Activar el display correspondiente
        PINES_MUX = (unsigned char)(~(1 << display_state)); // Enciende el bit correspondiente para cada display con seguridad implementando un or como mascara esto evitara el ghosting
        //Avanzar al siguiente display
        display_state = (display_state + 1) % NUM_DISPLAY; //Con esta sentencia aumente display_state 1,2,3,4 a la vez controla el limite que es la cantidad de display utilizados cuando llega a 4 automaticamente lo lleva al primer displey   
    }
}

//Funcion para procesar el mensaje y prepararlo para la funcio visualizar_display()
void mostrar_mensajes(const CARACTER *mensaje, uint8_t longitud){
    for(uint8_t i = 0; i < NUM_DISPLAY; i++){
        display_buffer[i] = (i < longitud) ? mensaje[i]: CHAR_CLEAR;
    }
}

//Funcion que contiene los mensajes para ser mostrados segun la selecion que se halla realizado
void diferentes_mensajes(void){
    //Secuencia de inicio(Mensaje que se encarga de mostrar una secuencia de modo espera)
    static const CARACTER sec1[] = {CHAR_GSOMB,CHAR_SOM_GBAJO,CHAR_GBAJO};
    static const CARACTER sec2[] = {CHAR_EZIZARRIBA,CHAR_CLEAR,CHAR_EZDEABAJO};
    static const CARACTER sec3[] = {CHAR_I,CHAR_CLEAR,CHAR_BVERTICAL};
    static const CARACTER sec4[] = {CHAR_EZIZBAJO,CHAR_CLEAR,CHAR_EZDEARRIBA};
    static const CARACTER sec5[] = {CHAR_GBAJO,CHAR_SOM_GBAJO,CHAR_GSOMB};
    
    //Mensajes para las opciones de seleccion de potencia
    static const CARACTER ms_para_35w[] = {CHAR_P,CHAR_3,CHAR_5};
    static const CARACTER ms_para_50w[] = {CHAR_P,CHAR_5,CHAR_0};
    static const CARACTER ms_inicio_proceso[] = {CHAR_CLEAR, CHAR_O, CHAR_N};
    
    //Aqui se trata de reproducir una movimiento con las secuencias creadas
    uint32_t now = millis();
    static uint32_t last_change = 0;
    static uint8_t mostrar_ms = 0;
    if(ms_state == MS_INIT && on_off == false && ms_tiempo == ms_tiempo_ninguno){
        if((now - last_change) >= TIEMPO_MOVIMIENTO){
            last_change = now;
            mostrar_ms = (mostrar_ms + 1) % 5;
            switch(mostrar_ms){
                case 0: mostrar_mensajes(sec1,3); break;
                case 1: mostrar_mensajes(sec2,3); break;
                case 2: mostrar_mensajes(sec3,3); break;
                case 3: mostrar_mensajes(sec4,3); break;
                case 4: mostrar_mensajes(sec5,3); break;
            }
        }
    }
    //Logica de eleccion de los mensajes para las opciones de potencia
    if(ms_tiempo == ms_tiempo_ninguno){
        switch(ms_state){
        case MS_P35W:   //Mensaje para la opcion, potencia de 35w
                efecto_titilar(ms_para_35w,3,500); //Parpadeo del mensaje
                if(cont >=5){cont = 0;ms_state = MS_INIT; ms_tiempo = ms_tiempo_35w;} //Funcion if que se encarga de controlar el tiempo de parpadeo para luego apagarlos
            break;
        case MS_P50W:   //Mensaje para la opcion, potencia de 50w
            efecto_titilar(ms_para_50w,3,500);  //Parpadeo del mensaje
            if(cont >=5){cont =0;ms_state = MS_INIT; ms_tiempo = ms_tiempo_50w;}//Funcion if que se encarga de controlar el tiempo de parpadeo para luego apagarlos
            break;
        }
    }
    //Este if se encarga de visualizar el timpo de inicio que es 180s, por que si no hay este if no muestra
    //el tiempo, cuando se eleccione la opcion de potencia de 35w, mostrarar el mensaje que se a seleccionado la potencia de 35w 
    //pero luego el display estara todo apagado. Con este if solucionamos ese problem volviendo a mostrar el
    //el tiempo de 180s.
    if(ms_tiempo == ms_tiempo_35w && ms_state == MS_INIT){
        convertir_tiempo_a_display();
    }
    //Este if se encarga de visualizar el timpo de inicio que es 180s, por que si no hay este if no muestra
    //el tiempo, cuando se eleccione la opcion de potencia de 50w, mostrarar el mensaje que se a seleccionado la potencia de 50w 
    //pero luego el display estara todo apagado. Con este if solucionamos ese problem volviendo a mostrar el
    //el tiempo de 180s.
    if(ms_tiempo == ms_tiempo_50w && ms_state == MS_INIT){
        convertir_tiempo_a_display();
    }
    
    //Aqui visualiza el mensaje MS_ON
    if(ms_state == MS_ON){
        efecto_titilar(ms_inicio_proceso,3,500);    //El mensaje parpadea cada 500ms
        if(cont>=5){cont = 0;ms_state = MS_INIT; on_off = !on_off;} //Este if controla la cantidad de parapadeo del mensaje
    }
}

//Funcion que se encarga del efecto de parpadeo de los mensajes
void efecto_titilar(const CARACTER *mensaje, uint8_t longitud, uint32_t tiempo){
    static uint32_t last_change = 0;
    static bool mostrar = true;
    
    uint32_t now = millis();
    if ((now - last_change) >= tiempo) {
        last_change = now;
        mostrar = !mostrar;
        cont ++;
        if(mostrar) {
            mostrar_mensajes(mensaje, longitud);
        } else {
            const CARACTER apagado[NUM_DISPLAY] = {CHAR_CLEAR, CHAR_CLEAR, CHAR_CLEAR};
            mostrar_mensajes(apagado, NUM_DISPLAY);
        }
    }
}

void botones(void){
    //Boton para 35w
    if(BUTTON_35W == false && on_off == false){
        __delay_ms(20);
        if(BUTTON_35W == false && on_off == false){
            if(modo_proceso != modo_50w){
                modo_proceso = modo_35w;
                ms_state = MS_P35W;
            }
            if(ms_tiempo != ms_tiempo_ninguno){
                    incremento_temporizador();
                }
            while(BUTTON_35W == false && on_off == false){visualizar_display();}
        }
    }
    //Boton para 50w
    if(BUTTON_50W == false && on_off == false){
        __delay_ms(20);
        if(BUTTON_50W == false && on_off == false){
            if(modo_proceso != modo_35w){
                modo_proceso = modo_50w;
                ms_state = MS_P50W;
            }
            if(ms_tiempo != ms_tiempo_ninguno){
                decremento_temporizador();
            }
            while(BUTTON_50W == false && on_off == false){visualizar_display();}
        }
    }
    if(BUTTON_START_STOP == false && modo_proceso != modo_ninguno ){
        __delay_ms(20);
        if(BUTTON_START_STOP == false){
            if(on_off == false){
                ms_state = MS_ON;       //Al mostrar el mensaje on pasa a activar on_off, 
                                        //ver funcion diferentes_mensajes()
            }else{
                on_off = !on_off;
            }
            
            while(BUTTON_START_STOP == false){visualizar_display();}
        }
    }
}
void proceso_on(void){
    if(on_off == true){
        switch(modo_proceso){
            case modo_35w:
                RELAY_35W = true;
                RELAY_50W = false;
                actualizar_temporizador();
                break;
            case modo_50w:
                RELAY_35W = false;
                RELAY_50W = true;
                actualizar_temporizador();
                break;
        }
    }else{
        RELAY_50W = false;
        RELAY_35W = false;
    }
}

void actualizar_temporizador(void) {
    static uint32_t last_second = 0;
    uint32_t now = millis();
    
    if (!temporizador_activo) return;
    
    // Actualizar cada 1000ms (1 segundo)
    if ((now - last_second) >= 1000) {
        last_second = now;
             
        // Decrementar 
        if (unidades > 0) {
            unidades--;
        } else {
            // Si segundos llegan a 0, decrementar minutos
            if (decenas > 0) {
                decenas--;
                unidades = 9;
            } else {
                if(centenas > 0){
                    centenas --;
                    decenas = 9;
                }else{
                    // Temporizador llegó a cero
                    temporizador_activo = false;
                    verificar_fin_temporizador();
                }
            }
        }
        // Actualizar display
        convertir_tiempo_a_display();
    }
}

//En esta funcion se encarga de acomodar las variables unidades, decenas y centenas
// en nuestro buffer lo cual quedaria de la siguiente manera nuestro:
//display_buffer[] = {centenas,decenas,unidades};
void convertir_tiempo_a_display(void) {
    // Minutos (dos dígitos)
    display_buffer[0] = centenas;    // Decenas de minutos
    display_buffer[1] = decenas;    // Unidades de minutos
    
    // Segundos (dos dígitos)
    display_buffer[2] = unidades;   // Decenas de segundos
}

void verificar_fin_temporizador(void) {
    // Aquí puedes añadir efectos visuales o sonidos cuando el temporizador llega a cero
    // Por ejemplo, hacer parpadear el display o activar un buzzer
    PORTC = 0X00;
    uint32_t last_change = 0;
    bool mostrar_dos_ms = false;
    const CARACTER MS_OFF[]={CHAR_O,CHAR_F,CHAR_F};
    const CARACTER MS_CLEAR[] = {CHAR_CLEAR,CHAR_CLEAR,CHAR_CLEAR};
    // Ejemplo: parpadeo rápido del display
    while(on_off == true) {
        visualizar_display();
        uint32_t now = millis();
        if((now - last_change) >= 500){
            last_change = now;
            mostrar_dos_ms = !mostrar_dos_ms;
            mostrar_mensajes((mostrar_dos_ms == true)? MS_OFF:MS_CLEAR, 4);
        }
        if(BUTTON_START_STOP == false){
            __delay_ms(20);
            if(BUTTON_START_STOP == false){
                condiciones_iniciales();
                while(BUTTON_START_STOP == false){visualizar_display();}
            }
        }
    }
}
void incremento_temporizador(void){
    unidades ++;
    if(unidades >= 10){
        unidades = 0;
        decenas ++;
        if(decenas >= 10){
            decenas = 0;
            centenas ++;
            if(centenas >= 10){
                centenas = 0;
            }
        }
    }
    // Actualizar display
    convertir_tiempo_a_display();
}
void decremento_temporizador(void){
    // Decrementar 
    if (unidades > 0) {
        unidades--;
    } else {
        // Si segundos llegan a 0, decrementar minutos
        if (decenas > 0) {
            decenas--;
            unidades = 9;
        } else {
            if(centenas > 0){
                centenas --;
                decenas = 9;
            }else{
                condiciones_iniciales();
            }
        }
    }
    // Actualizar display
    convertir_tiempo_a_display();
}

void condiciones_iniciales(void){
    PORTC = 0X00;
    on_off = false;
    ms_tiempo = ms_tiempo_ninguno;
    modo_proceso = modo_ninguno;
    RELAY_35W = false;
    RELAY_50W = false;
    temporizador_activo = true;
    unidades = 0;
    decenas = 8;
    centenas = 1;
}

//Funcion principal
void main(void){
    configurar_hardware();  //Realiza el llamado de la funcion para que configure los registros TRIS y PORT  
    configurar_tmr0();      //Realiza el llamado de la funcion para la configuracion del modulo TMR0 
    while(true){    
        visualizar_display();   //Realiza el llamado a la funcion para poder visualizar y multiplexar los mensajes
        diferentes_mensajes();  //Aqui se escoge el mensaje seleccionado segun loas opciones que hallamos escogido
        botones();              //Aqui se procesan los pulsadores y  se configura el modo de funcionamiento del equipo
        proceso_on();           //Una vez configurado las opciones del limpiador pasa a procesar y encender el equipo
    }
}