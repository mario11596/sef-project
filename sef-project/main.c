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
static int senzor = 0;
static int flagPot = 0;
static int pot1;
static int pot2;
static char potChar1[16];
static char potChar2[16];
const char *password_check = "1111";
const char *password_lock = "####";
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


void buzzerDetection(){
	while(counter > 0){
		PORTC ^= _BV(7);
		_delay_ms(100);
		counter--;
	}	
}

void writeLCD(uint16_t adc) {
	
	char adcStr[16];
	itoa(round(adc), adcStr, 10);
	
	if(flagPot == 0){
		lcd_gotoxy(6,0);
		lcd_clrscr();
		lcd_puts(adcStr);
	} else if(flagPot == 1){
		lcd_clrscr();
		lcd_gotoxy(6,0);
		lcd_puts(potChar1);
		lcd_gotoxy(6,1);
		lcd_puts(adcStr);
	}
	
	if(pot1 == 200 && pot2 == 200){
		lcd_clrscr();
		lcd_puts("Otvoren sef!");
		
		
	}

}


void readPotentiometer(){
	while(1){
		
		if((ADC <= 205 && ADC >= 195) && flagPot == 0){
			ADMUX |= _BV(MUX0);
			flagPot = 1;
			pot1 = 200;
			itoa(pot1, potChar1, 10);
			} else if((ADC <= 205 && ADC >= 195) && flagPot == 1){
			pot2 = 200;
		}
		ADCSRA |= _BV(ADSC);

		while (!(ADCSRA & _BV(ADIF)));

		writeLCD(ADC);

		_delay_ms(150);
		
		if(pot1 == 200 && pot2 == 200){
			break;
		}
	}
}


void turn_servo() {
	if(vrata == 1){
		PORTD ^= _BV(5);
		OCR1A = 300; // position +90�
	} else{
		PORTD ^= _BV(5);
		OCR1A = 65; // position -90�
		
		pot1 = 0;
		pot2 = 0;
		lcd_clrscr();
		lcd_puts("Sef zatvoren!");
		
	}	
}
	
	
void checkPassword(){
	if(!strncmp(output, password_lock, 4) && vrata == 1){
		vrata = 0;
		turn_servo();
	}
	
	if(strncmp(output, password_check, 4)){
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
		turn_servo();
		
		_delay_ms(300);
		senzor = 1;
		readPotentiometer();		
	}
	
	memset(output, 0, sizeof(output));
}


void keyPassword(){
	uint8_t i;
	
	for(i = 0; i < 4; i++){
		char password = keyfind();
		
		output[i] = password;
		lcd_gotoxy(i,0);
		lcd_clrscr();
		lcd_puts(output);
	}
	output[4] = '\0';
	checkPassword();
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
	DDRA |= _BV(6); //contrast
	DDRB = 0xff; //keypad
	PORTB = 0x00;
	
	DDRC |= _BV(7); //buzzer
	PORTC = _BV(7);
	
	DDRD |= _BV(5); //servo motor
	TCNT1 = 0;
	ICR1 = 2499;
	
	
	TCCR1A = _BV(WGM11) | _BV(COM1A1); 
	TCCR1B = _BV(WGM12) | _BV(WGM13) | _BV(CS10) | _BV(CS11);
	OCR1A = 65; // position -90�
	
	
	TCCR0 = _BV(WGM01) | _BV(WGM00) | _BV(COM01) | _BV(CS00); 
	OCR0 = 200;
	TIMSK = _BV(OCIE0);

	sei();
	
	ADMUX = _BV(REFS0);
	ADCSRA = _BV(ADEN) | _BV(ADPS2) | _BV(ADPS1);
	
	lcd_init(LCD_DISP_ON);
	lcd_clrscr();
	
	lcd_puts("Upisite lozinku");
	
	memset(potChar1, 0, sizeof(potChar1));
	memset(potChar2, 0, sizeof(potChar2));
}


int main(void){
	initMain();
	
	while (1){
		keyPassword();
	}
}
