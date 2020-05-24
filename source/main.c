/*	Author: agonz250 
 *  Partner(s) Name: 
 *	Lab Section: 028
 *	Assignment: Lab #10  Exercise #4
 *	Exercise Description: [optional - include for your own benefit]
 *
 *	I acknowledge all content contained herein, excluding template or example
 *	code, is my own original work.
 */
#include <avr/io.h>
#include <avr/interrupt.h>
#ifdef _SIMULATE_
#include "simAVRHeader.h"
#endif

//TIMER STUFF
///////////////////////////////////////////////////////////////////////
volatile unsigned char TimerFlag = 0; //stuff added 

unsigned long _avr_timer_M = 1;
unsigned long _avr_timer_cntcurr = 0;

void TimerOn() {
	//AVR timer/counter controller register ....
	TCCR1B = 0x0B;

	//AVR output compare register ....
	OCR1A = 125;

	//AVR timer interrupt mask register
	TIMSK1 = 0x02;

	//initialize avr counter
	TCNT1=0;

	_avr_timer_cntcurr = _avr_timer_M;
	//TimerIRS called every _avr ... milliseconds
	//
	
	//enable global interrupts
	SREG |= 0x80; //0x80: 1000000
}

void TimerOff() {
	TCCR1B = 0x00; //timer off
}

void TimerISR() {
	TimerFlag = 1;
}

//TimerISR() calls this 
ISR(TIMER1_COMPA_vect) {
	//cpu calls when TCNT1 == OCR1
	_avr_timer_cntcurr--;	//counts down to 0

	if (_avr_timer_cntcurr == 0) {
		TimerISR();	//calls ISR that user uses
		_avr_timer_cntcurr = _avr_timer_M;

	}
}

//Set TimerISR() to tick every M ms
void TimerSet(unsigned long M) {
	_avr_timer_M = M;
	_avr_timer_cntcurr = _avr_timer_M;
}


//////////////////////////////////////////////////////
//STATE STUFF

unsigned char threeLEDs;	//shared variables with combineLEDs
unsigned char blinkingLED;	
unsigned char sound; 		//used for speaker SM

unsigned char tmpA;	//input
unsigned char tmpB;

unsigned char i; //
unsigned char arr[3] = {0x01, 0x02, 0x04}; //array for lights to light up, TL_State
enum TL_States { TL_Start, A_TL} TL_state;
void TL_Tick() {
	switch(TL_state) {
		case TL_Start:
			TL_state = A_TL;
			i = 0;
			//threeLEDs = arr[i]; //displays bit 1 and then bit 2, in this config
			//i++;
			break;

		case A_TL:
			TL_state = A_TL;

			threeLEDs = arr[i]; 
			if (i < 2) {
				i++;
			}
			else { // i >= 2
				i = 0;
			}
			break;

		default:
			break;
	}
	//Prob don't need state acrions
}


//Blinking LED
enum BL_States { BL_Start, A_BL, B} BL_state; 
void BL_Tick() {
	switch(BL_state) {
		case BL_Start:
			BL_state = A_BL;
			//blinkingLED = 0;
			break;

		case A_BL:
			BL_state = B;
			break;

		case B:
			BL_state = A_TL;
			break;

		default:
			break;
	}

	switch (BL_state) { //actions
		case A_TL:
			blinkingLED = 0; //these were orignally switched 
			break;

		case B:
			blinkingLED = 1;
			break;
		default:
			break;
	}
}


//Combine LED
enum CL_States { CL_Start, A_CL} CL_state; 
void CL_Tick() {
	switch(CL_state) {
		case CL_Start:
			CL_state = A_CL;
			tmpB = 0;
			break;

		case A_CL:
			CL_state = A_CL;
			break;

		default:
			break;
	}

	switch (CL_state) {
		case A_CL:						//Added Speaker output
			tmpB = (threeLEDs & 0x07) | (blinkingLED << 3) | ((sound & 0x01) << 4);
			break;
		default: 
			break;
	}
}


//Speaker SM
enum SP_States { SP_Start, SP_OFF, SP_ON} SP_state; 
void SP_Tick() {
	switch(SP_state) {
		case SP_Start:
			SP_state = SP_OFF;
			sound = 0;
			break;

		case SP_OFF:
			if ((tmpA & 0x04) == 0x04) { 	//IF PA2 is on
				SP_state = SP_ON;
			}
			else {
				SP_state = SP_OFF;
			}
			break;

		case SP_ON:
			if ((tmpA & 0x04) == 0x04) { 	//IF PA2 is on
				SP_state = SP_ON;
				sound = !sound; //swaps betwen 0 and 1, GENERATING SOUND
			}
			else {
				SP_state = SP_OFF;
			}
			break;
			
		default:
			SP_state = SP_OFF;
			break;
	}

	switch (SP_state) {
		case SP_OFF: 
			sound = 0;
			break;

		case SP_ON:
			break;

		default: 
			break;
	}
}



unsigned char freq; // freq initally 2 
//Frequency SM
enum FR_States { FR_Start, Wait, Inc, Dec} FR_state; 

void FR_Tick() {
	switch(FR_state) {
		case FR_Start:
			FR_state = Wait;
			freq = 2;
			break;

		case Wait:
			if ((tmpA & 0x03) == 0x01) { 	//IF PA0 is on
				FR_state = Inc;

				if (freq < 10) { freq++; } //Not sure what to set max to 
			}
			else if ((tmpA & 0x03) == 0x02) { //PA1 is on
				FR_state = Dec;

				if (freq > 0) {freq--;} //probably needed 
			}
			else {
				FR_state = Wait;
			}
			break;

		case Inc:
			if ((tmpA & 0x03) == 0x01) { 	//IF PA0 is on
				FR_state = Inc;
				//sound = !sound; //swaps betwen 0 and 1, GENERATING SOUND
			}
			else {
				FR_state = Wait;
			}
			break;
			
		case Dec: 
			if ((tmpA & 0x03) == 0x02) { 	//IF PA1 is on
				FR_state = Dec;
			}
			else {
				FR_state = Wait;
			}
			break;
			 
		default:
			FR_state = Wait;
			break;
	}

	switch (SP_state) {
		case Wait: 
			//sound = 0;
			break;

		case Inc:
			break;

		case Dec: 
			break;
		default: 
			break;
	}
}




int main(void) {
    /* Insert DDR and PORT initializations */
	DDRA = 0x00; PORTA = 0xFF; //input
	DDRB = 0xFF; PORTB = 0x00; //output

	unsigned long TL_elapsedTime = 0;
	unsigned long BL_elapsedTime = 0;
	unsigned long CL_elapsedTime = 0;
	unsigned long SP_elapsedTime = 0; 
	unsigned long FR_elapsedTime = 0;
	const unsigned long timerPeriod = 1;

	TL_state = TL_Start;
	BL_state = BL_Start;
	CL_state = CL_Start;
	SP_state = SP_Start;
	FR_state = FR_Start;
	//threeLEDs = arr[i];
	//blinkingLED = 0;
	tmpA = 0;
	tmpB = 0x00;

	freq = 2;

	TimerSet(timerPeriod);
	TimerOn();
    while (1) {
	    tmpA = ~PINA;

	    if (TL_elapsedTime >= 300) {
	    	TL_Tick();
		TL_elapsedTime = 0;
	    }
	    if (BL_elapsedTime >= 1000) {
		 BL_Tick();
		 BL_elapsedTime = 0;
	    }
	    if (SP_elapsedTime >= freq) { // orig 2 ms, Changes as freq timer changes
		SP_Tick();
		SP_elapsedTime = 0;
	    }

	    if (FR_elapsedTime >= 50) {
		FR_Tick();
		FR_elapsedTime = 0;
	    }

	    if (CL_elapsedTime >= 1) {//Need to change too, to acc for speaker
	    	CL_Tick();
		CL_elapsedTime = 0;
	    }

	    PORTB = tmpB;

	    while(!TimerFlag) {}
	    TimerFlag = 0;
	    TL_elapsedTime += timerPeriod;
	    BL_elapsedTime += timerPeriod;
	    CL_elapsedTime += timerPeriod;
	    SP_elapsedTime += timerPeriod;
	    FR_elapsedTime += timerPeriod;
	
    }
    return 1;
}
