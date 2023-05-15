#include <MKL46Z4.h>
#include <stdint.h>
#include "lcd.h"
#include "utils.h"

#define SM1_Reset 0
#define SM1_Locked 1
#define SM1_Unlocked 2


/* ----------------------------------------------------------------------
 Note the different characters around the library names.
 The <> characters are used to include system libraries
 The "" characters are used to include your own libraries
 ------------------------------------------------------------------------*/


const int RED_LED_PIN = 29;
const int GREEN_LED_PIN = 5;
const int SWITCH_1_PIN = 3;
const int SWITCH_3_PIN = 12;
const unsigned int DOT = 300;
const unsigned int DASH = 1000;
SIM_Type* global_SIM = SIM;
PORT_Type* global_PORTE = PORTE;
GPIO_Type* global_PTE = PTE;
PORT_Type* global_PORTC = PORTC;
GPIO_Type* global_PTC = PTC;


volatile unsigned int current_time;
volatile unsigned int local_time;
volatile unsigned int switch3_presses;
volatile unsigned int switch1_presses;
volatile unsigned int code_ptr = 0;
unsigned int code[10];
volatile unsigned int morse_ptr = 0;
unsigned int morse[5];
unsigned int input_password[10]; //second time when one tries to unlock
//enum SM1_STATES { SM1_Reset, SM1_Locked, SM1_Unlocked} SM1_STATE;
int SM1_STATE;
int first_pass = 1;


// declare functions used to run the infinite loop for the switch
void operate_switch_polling();
void operate_switch_interrupts();

/*
 Main program: entry point
 */
int main(void) {
	// setup variables so we can see them in debugger
	// if you get rid of this it seems the compiler just optimizes the variables away
	// this is for educational purposes
	global_SIM = global_SIM;
	global_PORTE = global_PORTE;
	global_PTE = global_PTE;
	global_PORTC = global_PORTC;
	global_PTC = global_PTC;



	// setup red led
	SIM->SCGC5 |= SIM_SCGC5_PORTE_MASK; //Enable the clock to port E
	PORTE->PCR[RED_LED_PIN] = PORT_PCR_MUX(0b001); //Set up PTE29 as GPIO
	PTE->PDDR |= GPIO_PDDR_PDD(1 << RED_LED_PIN); // make it output
	PTE->PSOR |= GPIO_PSOR_PTSO(1 << RED_LED_PIN); // turn off LED

	//setup green led
	SIM->SCGC5 |= SIM_SCGC5_PORTD_MASK;
	PORTD->PCR[GREEN_LED_PIN] = PORT_PCR_MUX(001);
	PTD->PDDR |= GPIO_PDDR_PDD_MASK;
	PTD->PSOR |= (1<<5);


	// setup switch 3
	SIM->SCGC5 |= SIM_SCGC5_PORTC_MASK; //Enable the clock to port C
	PORTC->PCR[SWITCH_3_PIN] &= ~PORT_PCR_MUX(0b111); // Clear PCR Mux bits for PTC12
	PORTC->PCR[SWITCH_3_PIN] |= PORT_PCR_MUX(0b001); // Set up PTC12 as GPIO
	PTC->PDDR &= ~GPIO_PDDR_PDD(1 << SWITCH_3_PIN); // make it input
	PORTC->PCR[SWITCH_3_PIN] |= PORT_PCR_PE(1); // Turn on the pull enable
	PORTC->PCR[SWITCH_3_PIN] |= PORT_PCR_PS(1); // Enable the pullup resistor
	PORTC->PCR[SWITCH_3_PIN] &= ~PORT_PCR_IRQC(0b1111); // Clear IRQC bits for PTC12
	PORTC->PCR[SWITCH_3_PIN] |= PORT_PCR_IRQC(0b1011); // Set up the IRQC to interrupt on either edge (i.e. from high to low or low to high)


	// setup switch 1
	PORTC->PCR[SWITCH_1_PIN] &= ~PORT_PCR_MUX(0b111); // Clear PCR Mux bits for PTC3
	PORTC->PCR[SWITCH_1_PIN] |= PORT_PCR_MUX(0b001); // Set up PTC3 as GPIO
	PTC->PDDR &= ~GPIO_PDDR_PDD(1 << SWITCH_1_PIN); // make it input
	PORTC->PCR[SWITCH_1_PIN] |= PORT_PCR_PE(1); // Turn on the pull enable
	PORTC->PCR[SWITCH_1_PIN] |= PORT_PCR_PS(1); // Enable the pullup resistor
	PORTC->PCR[SWITCH_1_PIN] &= ~PORT_PCR_IRQC(0b1111); // Clear IRQC bits for PTC3
	PORTC->PCR[SWITCH_1_PIN] |= PORT_PCR_IRQC(0b1011); // Set up the IRQC to interrupt on either edge (i.e. from high to low or low to high)

	//operate_switch_polling();
 	operate_switch_interrupts();

	return 0;
}

void operate_switch_polling() {
	while(1) {
		if((PORTC->PCR[SWITCH_3_PIN] & PORT_PCR_ISF(1)) != 0) {
			PTE->PTOR = GPIO_PTOR_PTTO(1 << RED_LED_PIN); // button pressed, toggle LED
			PORTC->PCR[SWITCH_3_PIN] |= PORT_PCR_ISF(1);  // clear the interrupt status flag by writing 1 to it
		}
	}
}

int compare_arrays(unsigned int a[], unsigned int b[], int size) {
    for(int i = 0; i < size; i++) {
        if(a[i] != b[i]) {
            return 0; // arrays are not equal
        }
    }
    return 1; // arrays are equal
}

void reset_input(){
	input_password[0] = 0;
	input_password[1] = 0;
	input_password[2] = 0;
	input_password[3] = 0;
}

void tickfct(void) {
	NVIC_DisableIRQ(PIT_IRQn);
	switch(SM1_STATE){
	case SM1_Reset:
		if (switch1_presses > 0){
			SM1_STATE = SM1_Locked;
		}
		break;
	case SM1_Locked:
		if (compare_arrays(code, input_password, 4)){
			SM1_STATE = SM1_Unlocked;
			reset_input();
		}

		break;
	case SM1_Unlocked:
		if (compare_arrays(code, input_password, 4)){
			SM1_STATE = SM1_Locked;
			reset_input();
		}
		break;
	}
	NVIC_EnableIRQ(PIT_IRQn);
}

void operate_switch_interrupts() {

	current_time = 0;

	local_time = 0;

	switch3_presses = 0;

	switch1_presses = 0;

	SM1_STATE = SM1_Reset;

	SIM->SCGC6 = SIM_SCGC6_PIT_MASK;

	PIT->MCR = 0x00;

	PIT->CHANNEL[1].LDVAL = DEFAULT_SYSTEM_CLOCK * 0.001;

	PIT->CHANNEL[1].TCTRL |= PIT_TCTRL_TIE_MASK;

	PIT->CHANNEL[1].TCTRL |= PIT_TCTRL_TEN_MASK;

	NVIC_EnableIRQ(PIT_IRQn);
	NVIC_EnableIRQ(PORTC_PORTD_IRQn); // configure NVIC so that interrupt is enabled

	init_lcd();


	while(1){
		tickfct();
	}
}

void PIT1_Service(void) {
	current_time++;
	PIT->CHANNEL[1].TFLG |= 0;
}

int morse_to_int(unsigned int morse[]){
	if (morse[0] == 1 && morse[1] == 1 && morse[2] == 1 && morse[3] == 1 && morse[4] == 1){
		return 0;
	}
	else if(morse[0] == 0 && morse[1] == 1 && morse[2] == 1 && morse[3] == 1 && morse[4] == 1){
		return 1;
	}
	else if(morse[0] == 0 && morse[1] == 0 && morse[2] == 1 && morse[3] == 1 && morse[4] == 1){
		return 2;
	}
	else if(morse[0] == 0 && morse[1] == 0 && morse[2] == 0 && morse[3] == 1 && morse[4] == 1){
		return 3;
	}
	else if(morse[0] == 0 && morse[1] == 0 && morse[2] == 0 && morse[3] == 0 && morse[4] == 1){
		return 4;
	}
	else if(morse[0] == 0 && morse[1] == 0 && morse[2] == 0 && morse[3] == 0 && morse[4] == 0){
		return 5;
	}
	else if(morse[0] == 1 && morse[1] == 0 && morse[2] == 0 && morse[3] == 0 && morse[4] == 0){
		return 6;
	}
	else if(morse[0] == 1 && morse[1] == 1 && morse[2] == 0 && morse[3] == 0 && morse[4] == 0){
		return 7;
	}
	else if(morse[0] == 1 && morse[1] == 1 && morse[2] == 1 && morse[3] == 0 && morse[4] == 0){
		return 8;
	}
	else if(morse[0] == 1 && morse[1] == 1 && morse[2] == 1 && morse[3] == 1 && morse[4] == 0){
		return 9;
	}
	else{
		return -1;
	}
}
void PORTC_PORTD_IRQHandler(void) {
	//case where SW3 is pressed
	if((PORTC->PCR[SWITCH_3_PIN] & PORT_PCR_ISF(1)) != 0) {
		switch3_presses ++;
		//button presses must be even
		if (switch3_presses % 2 == 0){
			//morse dot
			if ((current_time-local_time) < DOT){
				//PTE->PTOR = GPIO_PTOR_PTTO(1 << RED_LED_PIN); // button pressed, toggle red LED
				morse[morse_ptr] = 0;
				morse_ptr++;

			}
			//morse dash
			else if ((current_time-local_time) < DASH) {
				//LEDGreen_Toggle();
				morse[morse_ptr] = 1;
				morse_ptr++;
			}
			//local_time = 0;
			if (switch3_presses % 10 == 0){
				int code_dig = morse_to_int(morse);
				if (switch1_presses > 0){
					input_password[code_ptr] = code_dig;
				}
				else{
					code[code_ptr] = code_dig;
				}
				code_ptr ++;
				displayDigit(code_ptr, code_dig);
				morse_ptr = 0;
			}
		}
		PORTC->PCR[SWITCH_3_PIN] |= PORT_PCR_ISF(1);  // clear the interrupt status flag by writing 1 to it
		local_time = current_time;
	}
	//case where SW1 is pressed
	else if((PORTC->PCR[SWITCH_1_PIN] & PORT_PCR_ISF(1)) != 0){
		NVIC_DisableIRQ(PIT_IRQn);
		switch1_presses++;
		if (SM1_STATE == SM1_Locked){
			LED_Off();
			LEDRed_On();
		}
		else if(SM1_STATE == SM1_Unlocked){
			LED_Off();
			LEDGreen_On();
		}
		first_pass = 0;

		morse_ptr = 0;
		code_ptr = 0;
		clearDisplay();
		PORTC->PCR[SWITCH_1_PIN] |= PORT_PCR_ISF(1);  // clear the interrupt status flag by writing 1 to it
		NVIC_EnableIRQ(PIT_IRQn);
	}

}
