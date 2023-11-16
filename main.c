#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdbool.h>
#include "nokia5110.h"
#include <stdio.h>
#include <string.h>

#define TIMER_CLK       F_CPU / 1024
#define IRQ_FREQ    100
#define BOTAO_B2    PD2 
#define BOTAO_B1    PD3 
volatile bool cronometroON = false; 
uint8_t zero[] = { 0x3e, 0x51, 0x49, 0x45, 0x3e };
uint8_t glyph[] = { 0b00010000, 0b00100100, 0b11100000, 0b00100100, 0b00010000 };

int contadorCentesimoUnidade = 0;
int contadorCentesimoDezena = 0;
int contadorSegundoUnidade = 0;
int contadorSegundoDezena = 0;
int contadorMinutoUnidade = 0;
int contadorMinutoDezena = 0;

uint8_t charl[][5]  = {
    { 0x3e, 0x51, 0x49, 0x45, 0x3e }, // 30 0
    { 0x00, 0x42, 0x7f, 0x40, 0x00 }, // 31 1
    { 0x42, 0x61, 0x51, 0x49, 0x46 }, // 32 2
    { 0x21, 0x41, 0x45, 0x4b, 0x31 }, // 33 3
    { 0x18, 0x14, 0x12, 0x7f, 0x10 }, // 34 4
    { 0x27, 0x45, 0x45, 0x45, 0x39 }, // 35 5
    { 0x3c, 0x4a, 0x49, 0x49, 0x30 }, // 36 6
    { 0x01, 0x71, 0x09, 0x05, 0x03 }, // 37 7
    { 0x36, 0x49, 0x49, 0x49, 0x36 }, // 38 8
    { 0x06, 0x49, 0x49, 0x29, 0x1e } // 39 9
}; 

void lcd_atualizar() {

    nokia_lcd_clear();

    nokia_lcd_write_string("\n\nCronometro\n\n\n\n",1);

    nokia_lcd_custom(3,charl[contadorSegundoDezena]);
    nokia_lcd_custom(4,charl[contadorSegundoUnidade]);
    nokia_lcd_custom(2,charl[contadorCentesimoUnidade]);
    nokia_lcd_custom(1,charl[contadorCentesimoDezena]);
    nokia_lcd_custom(5,charl[contadorMinutoUnidade]);
    nokia_lcd_custom(6,charl[contadorMinutoDezena]);

    nokia_lcd_write_string("\n\006\005:\003\004:\001\002", 1);
    nokia_lcd_set_cursor(42, 24);
 
    if(!cronometroON){
        nokia_lcd_write_string("\n\nstop\n\n\n\n",1);
        nokia_lcd_render();
    }
    else{
        nokia_lcd_write_string("\n\non\n\n\n\n",1);
        nokia_lcd_render();
    }
}

void timer_init() {
 	TCCR1A = 0;
	TCCR1B = 0;
	TCNT1  = 0;
    TCCR1B |= _BV(WGM12); 
    TCCR1B |= _BV(CS12) | _BV(CS10);  
    OCR1A = (TIMER_CLK / IRQ_FREQ) - 1; 
    TIMSK1 |= _BV(OCIE1A); 
}

ISR(TIMER1_COMPA_vect){
    if(cronometroON){
        contadorCentesimoUnidade++;
        if(contadorCentesimoUnidade>9){
            contadorCentesimoDezena++;
            if(contadorCentesimoDezena>9){
                contadorCentesimoDezena = 0;
                contadorSegundoUnidade++;
                if(contadorSegundoUnidade>9){
                    contadorSegundoDezena++;
                    if(contadorSegundoDezena>5){
                        contadorSegundoDezena = 0;
                        contadorMinutoUnidade++;
                        if(contadorMinutoUnidade>9){
                            contadorMinutoDezena++;
                            if(contadorMinutoDezena>9){
                                contadorMinutoDezena = 0;
                            }
                            contadorMinutoUnidade = 0;
                        }
                    }
                    contadorSegundoUnidade = 0;
                }
            }
            contadorCentesimoUnidade = 0;
        }
    }
}

int main(){
   
    cli();
    nokia_lcd_init();

    DDRD &= ~(_BV(BOTAO_B2) | _BV(BOTAO_B1));
    PORTD |= _BV(BOTAO_B2) | _BV(BOTAO_B1);

    
    timer_init();
    sei();

    while(1){
        // verifica se o BOTAO_B1 (inicio) foi pressionado
        if(!(PIND & _BV(BOTAO_B1))){
            cronometroON = true; // sinaliza que o cronometro está ligado
        }


        while((PIND & _BV(BOTAO_B1)) && cronometroON == true){
            cronometroON = false; // sinaliza que o cronometro está desligado
        }

        // verifica se o BOTAO_B2 (resete) foi pressionado e o cronometro estava pausado
        if(!(PIND & _BV(BOTAO_B2)) && cronometroON == false){ 
            contadorCentesimoUnidade = 0;
            contadorCentesimoDezena = 0;
            contadorSegundoUnidade = 0;
            contadorSegundoDezena = 0;
            contadorMinutoUnidade = 0;
            contadorMinutoDezena = 0;  
        }

        lcd_atualizar();
    }
}