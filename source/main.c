/*	Author: agonz250 
 *  Partner(s) Name: 
 *	Lab Section: 028
 *	Assignment: Lab #10  Exercise #1
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
unsigned char tmpB;

unsigned char i; //
unsigned char arr[3] = {0x01, 0x00, 0x04}; //array for lights to light up, TL_State
enum TL_States { TL_Start, A_TL} TL_state;
void TL_Tick() {
	switch(TL_state) {
		case TL_Start:
			TL_state = A_TL;
			threeLEDs = 0;
			i = 0;
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
			blinkingLED = 0;
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
			blinkingLED = 0x08;
			break;

		case B:
			blinkingLED = 0;
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
		case A_CL: //Not sure if I have this as a state action or trans action
			tmpB = threeLEDs;
			tmpB = (tmpB & 0x07) | blinkingLED;
			break;
		default: 
			break;
	}
}



int main(void) {
    /* Insert DDR and PORT initializations */
	DDRB = 0xFF; PORTB = 0x00; //output

	TL_state = TL_Start;
	BL_state = BL_Start;
	CL_state = CL_Start;
	threeLEDs = 0;
	blinkingLED = 0;
	tmpB = 0x00;

	TimerSet(1000);
	TimerOn();
    while (1) {

	    TL_Tick();
	    BL_Tick();
	    CL_Tick();

	    PORTB = tmpB;

	    while(!TimerFlag) {}
	    TimerFlag = 0;
    }
    return 1;
}
