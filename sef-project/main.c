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
#define CONFIRM_SOUND 2
#define ALARM_SOUND 6
#define WRONG_PASS 3
#define ADC_PLUS 51
#define ADC_MINUS 49
#define PASS_LENGHT 4
#define FIRST_ROW 0
#define SECOND_ROW 1
#define THIRD_ROW 2
#define FOURTH_ROW 3
#define MIDDLE_POSITION 5

unsigned char keypad[4][4] = {{'1','4','7','*'},{'2','5','8','0'},{'3','6','9','#'},{'A','B','C','D'}};
static char output[5] = {'*','*','*','*'};
static int buzzerCounter;
static int doorOpen = 0;
static int flagPot = 0;
static int pot1;
static int pot2;
static char potChar[16];
static int flagWrongPass = 0;
static int wrongPassCounter = 3;
const char *passwordCheck = "1598";
const char *passwordLock = "####";
const char confirm = 'A';
unsigned char column, row;


char keyfind(){
	while(1){
		KEY_DDR = 0xF0;          
		KEY_PRT = 0xFF;

		do{
			KEY_PRT &= 0x0F;     
			asm("NOP");
			column = (KEY_PIN & 0x0F); 
		}while(column != 0x0F);
		
		do{
			do{
				_delay_ms(20);             
				column = (KEY_PIN & 0x0F); 
			}while(column == 0x0F);       
				
			_delay_ms (40);	            
			column = (KEY_PIN & 0x0F);
		}while(column == 0x0F);

		KEY_PRT = 0xEF;           
		asm("NOP");
		column = (KEY_PIN & 0x0F);
			
		if(column != 0x0F){
			row = FIRST_ROW;
			break;
		}

		KEY_PRT = 0xDF;
		asm("NOP");
		column = (KEY_PIN & 0x0F);
		
		if(column != 0x0F){
			row = SECOND_ROW;
			break;
		}
			
		KEY_PRT = 0xBF;
		asm("NOP");
		column = (KEY_PIN & 0x0F);
		
		if(column != 0x0F){
			row = THIRD_ROW;
			break;
		}

		KEY_PRT = 0x7F;		
		asm("NOP");
		column = (KEY_PIN & 0x0F);
			
		if(column != 0x0F){
			row = FOURTH_ROW;
			break;
		}
	}

	if(column == 0x0E){
		return(keypad[row][0]);
	}else if(column == 0x0D){
		return(keypad[row][1]);
	}else if(column == 0x0B) {
		return(keypad[row][2]);
	}else{
		return(keypad[row][3]);
	}
}


void buzzerDetection(){
	if(flagWrongPass == 0){
		while(buzzerCounter > 0){
			PORTC ^= _BV(7);
			_delay_ms(125);
			buzzerCounter--;
		}
	} else if(flagWrongPass == 1){
		while(1){
			PORTC ^= _BV(7) | _BV(0) | _BV(1) | _BV(6);
			PORTA ^= _BV(3) | _BV(4) | _BV(5);
			
			_delay_ms(125);
			
			if(!(PINB & _BV(PB0))){
				if((PINB & _BV(PB0)) == 0){
					flagWrongPass = 0;
					wrongPassCounter = WRONG_PASS;
					PORTC |= _BV(7);
					
					if(doorOpen == 0){
						PORTC |= _BV(0) | _BV(1) | _BV(6); // turn on red LED diodes
						PORTA &= ~(_BV(3) | _BV(4) | _BV(5)); // turn off green LED diodes
					} else {
						PORTC &= ~(_BV(0) | _BV(1) | _BV(6)); // turn off red LED diodes
						PORTA |= _BV(3) | _BV(4) | _BV(5); // turn on green LED diodes
					}		
					break;
				}
			}
		}
	}
}


void turnServo() {
	if(doorOpen == 1){
		PORTD ^= _BV(5);
		OCR1A = 276; // position +90°
	} else if(doorOpen == 0){
		PORTD ^= _BV(5);
		OCR1A = 59; // position -90°
	}
}


void writeLCD(uint16_t adc) {
	char adcStr[16];
	itoa(round(adc), adcStr, 10);
	
	if(flagPot == 0){
		lcd_clrscr();
		
		lcd_gotoxy(6,FIRST_ROW);
		lcd_puts(adcStr);
	} else if(flagPot == 1){
		lcd_clrscr();
		
		lcd_gotoxy(6,FIRST_ROW);
		lcd_puts(potChar);
		lcd_gotoxy(6,SECOND_ROW);
		lcd_puts(adcStr);
	}
	
	if(pot1 == VALUE_POT && pot2 == VALUE_POT){
		lcd_clrscr();
		lcd_puts("Otvoren sef!");
	}
}


void readPotentiometer(){
	uint16_t adcConversion;
	
	while(1){
		
		adcConversion = ADC/10;
			
		if((adcConversion<= ADC_PLUS && adcConversion >= ADC_MINUS) && flagPot == 0){
			if(!(PINB & _BV(PB0))){
				if((PINB & _BV(PB0)) == 0){
					ADMUX |= _BV(MUX0); // switch to second potentiometer
					flagPot = 1;
					pot1 = VALUE_POT;
					itoa(pot1, potChar, 10);
					
					PORTC ^= _BV(0);
					PORTA ^= _BV(4);
					
					buzzerCounter = CONFIRM_SOUND;
					buzzerDetection();
				}
			}
		} else if((adcConversion <= ADC_PLUS && adcConversion >= ADC_MINUS) && flagPot == 1){
			if(!(PINB & _BV(PB0))){
				if((PINB & _BV(PB0)) == 0){
					pot2 = VALUE_POT;
					
					PORTC ^= _BV(1);
					PORTA ^= _BV(3);
					
					buzzerCounter = CONFIRM_SOUND;
					buzzerDetection();
				}
			}
		}
		ADCSRA |= _BV(ADSC);

		while (!(ADCSRA & _BV(ADIF)));

		writeLCD(adcConversion);
		
		_delay_ms(150);
		
		if(pot1 == VALUE_POT && pot2 == VALUE_POT){
			doorOpen = 1;
			ADMUX &= ~_BV(MUX0); // switch to first potentiometer
			turnServo();
			break;
		}
	}
}
	
	
void checkPassword(){
	if((!strncmp(output, passwordLock, PASS_LENGHT)) && doorOpen == 1){
		lcd_clrscr();
		lcd_puts("Sef zatvoren!");
		
		doorOpen = 0;
		turnServo();
		buzzerCounter = CONFIRM_SOUND;
		buzzerDetection();
		
		PORTC ^= _BV(0) | _BV(1) | _BV(6);
		PORTA ^= _BV(3) | _BV(4) | _BV(5);
		
		pot1 = POT_ZERO;
		pot2 = POT_ZERO;
		memset(potChar, 0, sizeof(potChar));
		flagPot = 0;
		
	} else if(strncmp(output, passwordCheck, PASS_LENGHT)){
		lcd_clrscr();
		lcd_puts("Netocna lozinka!");
		
		buzzerCounter = ALARM_SOUND;
		buzzerDetection();
		wrongPassCounter--;
		
		if(wrongPassCounter == 0){
			flagWrongPass = 1;
			buzzerDetection();
		}	
	} else  {
		lcd_clrscr();
		lcd_puts("Tocna lozinka!");
		
		buzzerCounter = CONFIRM_SOUND;
		buzzerDetection();
		
		PORTC ^= _BV(6);
		PORTA ^= _BV(5);
		
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
	
	for(i = 0; i < PASS_LENGHT; i++){
		char password = keyfind();
		
		output[i] = password;
		lcd_gotoxy(i+MIDDLE_POSITION,SECOND_ROW);
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
	if(doorOpen == 1){
		PORTD ^= _BV(5);
	}
}


void initMain(){
	DDRA |= _BV(6); // contrast
	
	DDRB = 0xff; // keypad
	PORTB = 0x00;
	
	DDRC = _BV(7); // active buzzer
	
	DDRC |= _BV(0) | _BV(1) | _BV(6); // LED diodes red
	PORTC = _BV(7) | _BV(0) | _BV(1) | _BV(6);
	
	DDRA |= _BV(3) | _BV(4) | _BV(5); // LED diodes green
	
	DDRD |= _BV(5); // servo motor
	TCNT1 = 0;
	ICR1 = 2303; // 50Hz (20 ms) PWM period
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
