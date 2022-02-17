#define F_CPU 7372800UL
#include <avr/io.h>
#include <stdlib.h>
#include <util/delay.h>
#include <stdio.h>
#include <string.h>
#include <avr/interrupt.h>

#include "lcd.h"

#define KEY_PRT	PORTB
#define KEY_DDR	DDRB
#define KEY_PIN	PINB

#define VALUE_POT 50
#define POT_ZERO 0

unsigned char keypad[4][4] = {{'1','4','7','*'},{'2','5','8','0'},{'3','6','9','#'},{'A','B','C','D'}};
static char output[5] = {'*','*','*','*'};
static int buzzer_counter;
static int door = 0;
static int flagPot = 0;
static int pot1;
static int pot2;
static char potChar[16];
static int flagWrongPass = 0;
static int wrongPass_counter = 3;
const char *password_check = "1598";
const char *password_lock = "####";
const char confirm = 'A';
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
	if(flagWrongPass == 0){
		while(buzzer_counter > 0){
			PORTC ^= _BV(7);
			_delay_ms(125);
			buzzer_counter--;
		}
	} else if(flagWrongPass == 1){
		while(1){
			PORTC ^= _BV(7);
			_delay_ms(125);
			
			if(!(PINB & _BV(PB0))){
				if((PINB & _BV(PB0)) == 0){
					flagWrongPass = 0;
					wrongPass_counter = 3;
					PORTC |= _BV(7);
					break;
				}
			}
		}
	}
}


void turn_servo() {
	if(door == 1){
		PORTD ^= _BV(5);
		OCR1A = 276; // position +90°
		} else if(door == 0){
		PORTD ^= _BV(5);
		OCR1A = 59; // position -90°
	}
}


void writeLCD(uint16_t adc) {
	
	char adcStr[16];
	itoa(round(adc), adcStr, 10);
	
	if(flagPot == 0){
		lcd_clrscr();
		lcd_gotoxy(6,0);
		lcd_puts(adcStr);
	} else if(flagPot == 1){
		lcd_clrscr();
		lcd_gotoxy(6,0);
		lcd_puts(potChar);
		lcd_gotoxy(6,1);
		lcd_puts(adcStr);
	}
	
	if(pot1 == VALUE_POT && pot2 == VALUE_POT){
		lcd_clrscr();
		lcd_puts("Otvoren sef!");
	}
}


void readPotentiometer(){
	uint16_t adc_conversion;
	
	while(1){
		
		adc_conversion = ADC/10;
			
		if((adc_conversion<= 51 && adc_conversion >= 49) && flagPot == 0){
			if(!(PINB & _BV(PB0))){
				if((PINB & _BV(PB0)) == 0){
					ADMUX |= _BV(MUX0);
					flagPot = 1;
					pot1 = VALUE_POT;
					itoa(pot1, potChar, 10);
					
					PORTC ^= _BV(1);
					PORTA ^= _BV(4);
				}
			}
		} else if((adc_conversion <= 51 && adc_conversion >= 49) && flagPot == 1){
			if(!(PINB & _BV(PB0))){
				if((PINB & _BV(PB0)) == 0){
					pot2 = VALUE_POT;
					
					PORTC ^= _BV(6);
					PORTA ^= _BV(5);
				}
			}
		}
		ADCSRA |= _BV(ADSC);

		while (!(ADCSRA & _BV(ADIF)));

		writeLCD(adc_conversion);
		
		_delay_ms(150);
		
		if(pot1 == VALUE_POT && pot2 == VALUE_POT){
			door = 1;
			ADMUX &= ~_BV(MUX0);
			turn_servo();
			break;
		}
	}
}
	
	
void checkPassword(){
	if((!strncmp(output, password_lock, 4)) && door == 1){
		door = 0;
		turn_servo();
		buzzer_counter = 2;
		buzzerDetection();
		
		PORTC ^= _BV(0) | _BV(1) | _BV(6);
		PORTA ^= _BV(3) | _BV(4) | _BV(5);
		pot1 = POT_ZERO;
		pot2 = POT_ZERO;
		memset(potChar, 0, sizeof(potChar));
		flagPot = 0;
		
		lcd_clrscr();
		lcd_puts("Sef zatvoren!");
	} else if((strncmp(output, password_check, 4)) && (door == 0 || door == 1)){
		lcd_clrscr();
		lcd_puts("Netocna lozinka!");
		buzzer_counter = 6;
		buzzerDetection();
		wrongPass_counter--;
		
		if(wrongPass_counter == 0){
			flagWrongPass = 1;
			buzzerDetection();
		}	
	} else  {
		lcd_clrscr();
		lcd_puts("Tocna lozinka!");
		buzzer_counter = 2;
		buzzerDetection();
		PORTC ^= _BV(0);
		PORTA ^= _BV(3);
		
		_delay_ms(1000);
		readPotentiometer();		
	}
	_delay_ms(1500);
	memset(output, '*', sizeof(output)-1);
}


void keyPassword(){
	uint8_t i;
	char pass;
	
	lcd_clrscr();
	lcd_puts("Unesite lozinku!");
	for(i = 0; i < 4; i++){
		char password = keyfind();
		
		output[i] = password;
		lcd_gotoxy(i+5,1);
		lcd_puts(&output[i]);
	}
	output[4] = '\0';
	
	pass = keyfind();
	if(pass == confirm){
		checkPassword();
	}
}


ISR(TIMER0_COMP_vect){
	PORTA ^= _BV(6);
}


ISR(TIMER1_COMPA_vect){
	if(door == 1){
		PORTD ^= _BV(5);
	}
}


void initMain(){
	DDRA |= _BV(6); //contrast
	DDRB = 0xff; //keypad
	PORTB = 0x00;
	
	DDRC = _BV(7); //buzzer
	DDRC |= _BV(0) | _BV(1) | _BV(6); //led diodes red
	PORTC = _BV(7) | _BV(0) | _BV(1) | _BV(6);
	
	DDRA |= _BV(3) | _BV(4) | _BV(5); // led diodes green
	
	DDRD |= _BV(5); //servo motor
	TCNT1 = 0;
	ICR1 = 2303; // 50 Hz
	
	
	TCCR1A = _BV(WGM11) | _BV(COM1A1); 
	TCCR1B = _BV(WGM12) | _BV(WGM13) | _BV(CS10) | _BV(CS11);
	OCR1A = 59; // position -90°
	TIMSK = _BV(OCIE1A);
	
	
	TCCR0 = _BV(WGM01) | _BV(WGM00) | _BV(COM01) | _BV(CS00); 
	OCR0 = 200;
	TIMSK = _BV(OCIE0);

	sei();
	
	ADMUX = _BV(REFS0);
	ADCSRA = _BV(ADEN) | _BV(ADPS2) | _BV(ADPS1);
	
	lcd_init(LCD_DISP_ON);
	lcd_clrscr();
	
	memset(potChar, 0, sizeof(potChar));
}


int main(void){
	initMain();
	
	while (1){
		keyPassword();
	}
}
