#include <avr/interrupt.h>
#include <avr/io.h> 
#include <util/delay.h>
#include <avr/eeprom.h>
#define Q1 			(PIND & _BV(4))>>4
#define Q2 			(PIND & _BV(3))>>2
#define Q3 			(PIND & _BV(1))<<1
#define Q4 			(PIND & _BV(0))<<3
#define STAR		 0x0B
#define HASH		0x0C
#define A			0x0D
#define B			0x0E
#define C			0x0F
#define D			0x00
#define SET_LED(x) 	PORTC &= ~_BV(x)
#define CHANGE_LED	PORTC ^= _BV(1)|_BV(2)|_BV(3)|_BV(4)
#define CLEAR_LED 	PORTC |= _BV(0)|_BV(1)|_BV(2)|_BV(3)|_BV(4)
#define RELAY_ON 	PORTC &= ~_BV(0)					
#define RELAY_OFF	PORTC |= _BV(0)
#define NEW_DTMF 	PIND & _BV(2)

/************ Variables**************/
volatile unsigned char flag;
unsigned char setPass[4];
unsigned char enterPass[4];
unsigned char changePass[4]={HASH, A, B, C};

ISR(INT0_vect){
flag=1;													
}
/************Ports initialization**************/
void init(void){
	DDRC  |= _BV(0)|_BV(1)|_BV(2)|_BV(3)|_BV(4);		//PC0-relay PC1-4 LED
	PORTC |= _BV(0)|_BV(1)|_BV(2)|_BV(3)|_BV(4);
	
	DDRD  &= ~(_BV(0)|_BV(1)|_BV(2)|_BV(3)|_BV(4));	
}
/*********************************************/

unsigned char decode(void){
	unsigned char tone =0;
	tone = Q1|Q2|Q3|Q4;
	return(tone);
}
void password(unsigned char tab[]){
	for(unsigned char i=1;i<5;i++){
		while(!(NEW_DTMF));
		while(NEW_DTMF);									
		if(flag==1){										
			tab[i-1]=decode();
			SET_LED(i);									
			flag=0;
			if(tab[i-1]==STAR){						
				i=0;									
				CLEAR_LED;
			}
		}
	}
	_delay_ms(200);
	CLEAR_LED;
}
unsigned char checkPassword(unsigned char buff[],unsigned char pass[]){
	unsigned char correct=0;
	for(unsigned char i=0;i<4;i++){
		if(buff[i]==pass[i]){
			correct++;
		}else
			c=0;
	}
	if(correct==4) 
		return(1);										
	else 
		return(0);										
}
void open(void){
	RELAY_ON ;											
	_delay_ms(6000);									
	RELAY_OFF;											
	CLEAR_LED;								 			
}
void savePassToEEPROM(void){
	cli();												//interruption disabled
	for(int i=0;i<4;i++)								
		eeprom_write_byte((uint8_t*)i,setpass[i]);		
	sei();												//interruption enabled
}
void readPassFromEEPROM(void){
	cli();												
	for(int i=0;i<4;i++)
		setpass[i]=eeprom_read_byte((uint8_t*)i);
	sei();												

}	
int main(void)
{
	GICR |= (1<<INT0);									
	MCUCR |=(1<<ISC01);									//ISC1=1 ISC0=0	(interruptions)
	init();
	readPassFromEEPROM();

	while(1){
		password(enterPass);							
		if(checkPassword(enterPass, setPass)){			
			open();
		}else if(checkPassword(enterPass, changePass)){
			password(setPass);							
			savePassToEEPROM();
		}else{
			for(unsigned char i=0;i<10;i++){
			CHANGE_LED;
			_delay_ms(500);
			}
			CLEAR_LED;
		}	
	}
}