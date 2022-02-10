#define F_CPU 7372800UL
#include <avr/io.h>
#include <stdlib.h>
#include <util/delay.h>
#include <stdio.h>
#include <string.h>
#include <avr/interrupt.h>

#include "lcd.h"

#define KEY_PRT		PORTB
#define KEY_DDR		DDRB
#define KEY_PIN		PINB



unsigned char keypad[4][4] = {	{'1','4','7','*'},
								{'2','5','8','0'},
								{'3','6','9','#'},
								{'A','B','C','D'}};
	
static char output[4];
static int counter = 6;
static int vrata = 0;
const char *provjera_lozinke = "1111";
const char *zakljucavanje = "####";
unsigned char colloc, rowloc;

char keyfind(){
	while(1){
		KEY_DDR = 0xF0;          
		KEY_PRT = 0xFF;

		do{
			KEY_PRT &= 0x0F;     
			asm("NOP");
			colloc = (KEY_PIN & 0x0F); 
		}while(colloc != 0x0F);
		
		do{
			do{
				_delay_ms(20);             
				colloc = (KEY_PIN & 0x0F); 
				}while(colloc == 0x0F);       
				
				_delay_ms (40);	            
				colloc = (KEY_PIN & 0x0F);
			}while(colloc == 0x0F);

			KEY_PRT = 0xEF;           
			asm("NOP");
			colloc = (KEY_PIN & 0x0F);
			if(colloc != 0x0F){
				rowloc = 0;
				break;
			}

			KEY_PRT = 0xDF;		
			asm("NOP");
			colloc = (KEY_PIN & 0x0F);
			if(colloc != 0x0F){
				rowloc = 1;
				break;
			}
			
			KEY_PRT = 0xBF;		
			asm("NOP");
			colloc = (KEY_PIN & 0x0F);
			if(colloc != 0x0F){
				rowloc = 2;
				break;
			}

			KEY_PRT = 0x7F;		
			asm("NOP");
			colloc = (KEY_PIN & 0x0F);
			if(colloc != 0x0F){
				rowloc = 3;
				break;
			}
		}

		if(colloc == 0x0E){
			return(keypad[rowloc][0]);
		}else if(colloc == 0x0D){
			return(keypad[rowloc][1]);
		}else if(colloc == 0x0B) {
			return(keypad[rowloc][2]);
		}else{
			return(keypad[rowloc][3]);
		}
}


/*ISR(TIMER0_COMP_vect) {
		while(counter > 0){
			PORTC = 0xff;
			_delay_ms(100);
			PORTC = 0x00;
			_delay_ms(100);
			counter--;
		} 	
		TIMSK ^= _BV(OCIE0);
		PORTC = 0xff;
		counter = 3;
		
		if(counter > 0){
			PORTC ^= _BV(0x00);
			_delay_ms(100);
			counter--;
		}		
}+*/
/*
ISR(COMP){
	IF(FLAGPWM ){
		PORT |= _BV(NEKI);
	}
}

ISR(OVF)D{
	I++;
	
	IF(I >= N){
		FLAGPWM = 1;
		I = 0;
	}
	TU MORA ICI NEKA FUNKCIJA KOJAÆE GA DRZATI U JEDINICA NA OVRATKU, UGASITI PORT
	};
*/

void buzzerDetection(){
	while(counter > 0){
		PORTC ^= _BV(7);
		_delay_ms(100);
		counter--;
	}	
	//counter = 6;
}

void okreni_vrata() {
	if(vrata == 1){
		PORTD ^= _BV(5);
		OCR1A = 300;
	} else{
		PORTD ^= _BV(5);
		OCR1A = 65;
	}
	
}
	
void provjera(){
	if(!strncmp(output, zakljucavanje, 1)){
		vrata = 0;
		okreni_vrata();
	}
	
	if(strncmp(output, provjera_lozinke, 4)){
		lcd_clrscr();
		lcd_puts("Netocna lozinka!");
		counter = 6;
		buzzerDetection();	
	} else  {
		lcd_clrscr();
		lcd_puts("Tocna lozinka!");
		counter = 2;
		buzzerDetection();
		vrata = 1;
		okreni_vrata();
		
	}
	
	memset(output, 0, sizeof(output));
}

void keyPassword(){
	uint8_t i;
	
	for(i = 0; i < 4; i++){
		char lozinka = keyfind();
		
		output[i] = lozinka;
		lcd_gotoxy(i,0);
		lcd_clrscr();
		lcd_puts(output);
	}
	output[4] = '\0';
	provjera();
}

ISR(TIMER0_COMP_vect){
	PORTA ^= _BV(6);
}


ISR(TIMER1_COMPA_vect){
	if(vrata == 1){
		PORTD ^= _BV(5);
	}
}

void initMain(){
	DDRA |= _BV(6); //kontrast
	DDRB = 0xff; //tipke
	PORTB = 0x00;
	
	DDRC |= _BV(7); //buzzer
	PORTC = _BV(7);
	
	DDRD |= _BV(5); //servo
	TCNT1 = 0;
	ICR1 = 2499;
	
	
	TCCR1A = _BV(WGM11) | _BV(COM1A1);  //servo
	TCCR1B = _BV(WGM12) | _BV(WGM13) | _BV(CS10) | _BV(CS11);
	OCR1A = 65; // poèetna pozicija: -90 °
	
	
	TCCR0 = _BV(WGM01) | _BV(WGM00) | _BV(COM01) | _BV(CS00); //LCD
	OCR0 = 200;
	TIMSK = _BV(OCIE0);
	
	sei();
	

	lcd_init(LCD_DISP_ON);
	lcd_clrscr();
}

int main(void){
	initMain();
	
	while (1){
		keyPassword();
	}
}
