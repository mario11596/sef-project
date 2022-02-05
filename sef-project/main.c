#define F_CPU 7372800UL
#include <avr/io.h>
#include <stdlib.h>
#include <util/delay.h>
#include <stdio.h>
#include <string.h>
#include <avr/interrupt.h>

#include "lcd.h"

#define KEY_PRT 	PORTA
#define KEY_DDR		DDRA
#define KEY_PIN		PINA

unsigned char keypad[4][4] = {	{'1','4','7','*'},
								{'2','5','8','0'},
								{'3','6','9','#'},
								{'A','B','C','D'}};
	
static char output[4];
static int counter = 3;
const char *provjera_lozinke = "1111";
unsigned char colloc, rowloc;

char keyfind(){
	while(1){
		KEY_DDR = 0xF0;           /* set port direction as input-output */
		KEY_PRT = 0xFF;

		do{
			KEY_PRT &= 0x0F;      /* mask PORT for column read only */
			asm("NOP");
			colloc = (KEY_PIN & 0x0F); /* read status of column */
		}while(colloc != 0x0F);
		
		do{
			do{
				_delay_ms(20);             /* 20ms key debounce time */
				colloc = (KEY_PIN & 0x0F); /* read status of column */
				}while(colloc == 0x0F);        /* check for any key press */
				
				_delay_ms (40);	            /* 20 ms key debounce time */
				colloc = (KEY_PIN & 0x0F);
			}while(colloc == 0x0F);

			/* now check for rows */
			KEY_PRT = 0xEF;            /* check for pressed key in 1st row */
			asm("NOP");
			colloc = (KEY_PIN & 0x0F);
			if(colloc != 0x0F)
			{
				rowloc = 0;
				break;
			}

			KEY_PRT = 0xDF;		/* check for pressed key in 2nd row */
			asm("NOP");
			colloc = (KEY_PIN & 0x0F);
			if(colloc != 0x0F)
			{
				rowloc = 1;
				break;
			}
			
			KEY_PRT = 0xBF;		/* check for pressed key in 3rd row */
			asm("NOP");
			colloc = (KEY_PIN & 0x0F);
			if(colloc != 0x0F)
			{
				rowloc = 2;
				break;
			}

			KEY_PRT = 0x7F;		/* check for pressed key in 4th row */
			asm("NOP");
			colloc = (KEY_PIN & 0x0F);
			if(colloc != 0x0F)
			{
				rowloc = 3;
				break;
			}
		}

		if(colloc == 0x0E)
		return(keypad[rowloc][0]);
		else if(colloc == 0x0D)
		return(keypad[rowloc][1]);
		else if(colloc == 0x0B)
		return(keypad[rowloc][2]);
		else
		return(keypad[rowloc][3]);
}


ISR(TIMER0_COMP_vect) {
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
}


void buzzerDetection(){
	OCR0 = 100;
	TIMSK = _BV(OCIE0);
	
}
	
void provjera(){
	if(strncmp(output, provjera_lozinke, 4)){
		lcd_clrscr();
		lcd_puts("Netocna lozinka!");	
		buzzerDetection();	
	} else  {
		lcd_clrscr();
		lcd_puts("Tocna lozinka!");
	}
	memset(output, 0, sizeof(output));
}


void debounce() {
	_delay_ms(100);
	
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

int main(void)
{
	DDRD = _BV(4);
	DDRA = 0xff;
	PORTA = 0x00;
	
	DDRC = _BV(0); //buzzer
	PORTC = 0xff;
	
	TCCR1A = _BV(COM1B1) | _BV(WGM10);
	TCCR1B = _BV(WGM12) | _BV(CS11);
	OCR1B = 70;
	
	TCCR0 = _BV(WGM01) | _BV(CS02) | _BV(CS00);
	sei();

	lcd_init(LCD_DISP_ON);
	lcd_clrscr();
	
	while (1){
		keyPassword();	
	}
}
