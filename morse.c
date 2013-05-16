/*-------------------------------------------------------------
| Evan Sapienza
| Steve Hwang
|
| EC450 - Final Project
|
| Morse Code Translator
|
| Input Mode 0:
|
| Number is selected on seven segment display
| and then translated to morse code (Flashing LED)
|
| Input Mode 1:
|
| Pushbutton is used to enter morse code of a number
|the number is then shown on the display
-------------------------------------------------------------
*/

//header files
#include <msp430.h> 
#include <signal.h>

//Define LED
#define LED BIT2; //2.2

//Define 7 segments to pins
#define SEGA BIT0 //1.0
#define SEGB BIT6 //1.6
#define SEGC BIT4 //1.4
#define SEGD BIT2 //1.2
#define SEGE BIT1 //1.1
#define SEGF BIT7 //1.7
#define SEGG BIT5 //1.5

//Define Digits
#define DIG1 BIT4 //2.4
#define DIG3 BIT3 //2.3

//Define BUTTON
#define BUTTON BIT3 //1.3

unsigned int button_pressed = 0;
unsigned char last_button_state;   // state of the button the last time it was read

int inputState = 0; //tracks if button was pressed for ~3 seconds
int state; //determines input mode

//7seg
int numSelect; //number displayed
int digitSwitch; //bool for switching digit
int timer; //timer for input (~4 seconds of no inputs will take in numSelect as input)
int number; //stores inputed number

//Morse
unsigned int buttonPresses; //number of times the button has been pressed
unsigned int pressTime; //how long the button was pressed
unsigned int buffIt; //buffer iterator
char buffer[5]; //buffer for long and short characters
int number; //number translated from morse code
int digit; //which digit to display

//7seg
void display_number(int, unsigned int);
void morse_code(int);
void dash();
void dot();

//Morse
int morse_number(char[]);

void main(void)
{
	//WDTCTL = WDTPW + WDTHOLD;                 // Stop watchdog timer

	  // setup the watchdog timer as an interval timer
	  WDTCTL =(WDTPW + // (bits 15-8) password
	                   // bit 7=0 => watchdog timer on
	                   // bit 6=0 => NMI on rising edge (not used here)
	                   // bit 5=0 => RST/NMI pin does a reset (not used here)
	           WDTTMSEL + // (bit 4) select interval timer mode
	           WDTCNTCL +  // (bit 3) clear watchdog timer counter
	  		          0 // bit 2=0 => SMCLK is the source
	  		          +1 // bits 1-0 = 01 => source/8K ~ 134 Hz <-> 7.4ms.
	  		   );
	  IE1 |= WDTIE;		// enable the WDT interrupt (in the system interrupt register IE1)


    //Set pins used for segments to output
	P1DIR |= SEGA;
	P1DIR |= SEGB;
	P1DIR |= SEGC;
	P1DIR |= SEGD;
	P1DIR |= SEGE;
	P1DIR |= SEGF;
	P1DIR |= SEGG;

	//Set digits and LED
	P2DIR |= DIG1;
	P2DIR |= DIG3;
	P2DIR |= LED;

	P1OUT  = 0xFF;                            // start 7 segment off
	P1REN |= BUTTON;                            // P1.3 pullup

	//set default values to variables

	//7seg
	numSelect = 0;
	timer = 0;
	number = 0;
	digitSwitch = 1;


	//Morse
	buttonPresses = 0;
	pressTime = 0;
	buffIt = 0;
	digit = 1;

	state = -1; //start in morse code input mode 1

	__bis_SR_register(CPUOFF + GIE);        // LPM0 with interrupts enabled
}

//function determines which digit and what number to display
void display_number(int display_this, unsigned int digit)
{
	P1OUT  = BUTTON;
	display_this = display_this % 10;

	//select digit
	switch (digit) {
		case 1:
			P2OUT = DIG1;
			break;
		case 3:
			P2OUT = DIG3;
			break;
		default:
			P2OUT = DIG1;
	}

	//select number
	switch (display_this) {
		case 0:
			P1OUT |= SEGG;
			break;
		case 1:
			P1OUT |= SEGA | SEGB | SEGC | SEGD | SEGG;
			break;
		case 2:
			P1OUT |= SEGC | SEGF;
			break;
		case 3:
			P1OUT |= SEGE | SEGF;
			break;
		case 4:
			P1OUT |= SEGA | SEGD | SEGE;
			break;
		case 5:
			P1OUT |= SEGB | SEGE;
			break;
		case 6:
			P1OUT |= SEGB;
			break;
		case 7:
			P1OUT |= SEGD | SEGE | SEGF | SEGG;
			break;
		case 8:
			P1OUT |= 0x00;
			break;
		case 9:
			P1OUT |= SEGD | SEGE;
			break;
		default:
			P1OUT |= SEGB | SEGC;
	}
}

//flash LED long
void dash() {
	P2OUT = LED;
	__delay_cycles(600000);
	P2OUT = 0x00;
	__delay_cycles(600000);
}

//flash LED short
void dot() {
	P2OUT = LED;
	__delay_cycles(200000);
	P2OUT = 0x00;
	__delay_cycles(600000);
}

//takes in a number and fills buffer with morse code equivalent
//buffer is then read and LED flashes morse code
void morse_code(int number){
	char buffer[5];

	switch (number) {
		case 0:
			buffer[0] = '-';
			buffer[1] = '-';
			buffer[2] = '-';
			buffer[3] = '-';
			buffer[4] = '-';
			break;
		case 1:
			buffer[0] = '.';
			buffer[1] = '-';
			buffer[2] = '-';
			buffer[3] = '-';
			buffer[4] = '-';
			break;
		case 2:
			buffer[0] = '.';
			buffer[1] = '.';
			buffer[2] = '-';
			buffer[3] = '-';
			buffer[4] = '-';
			break;
		case 3:
			buffer[0] = '.';
			buffer[1] = '.';
			buffer[2] = '.';
			buffer[3] = '-';
			buffer[4] = '-';
			break;
		case 4:
			buffer[0] = '.';
			buffer[1] = '.';
			buffer[2] = '.';
			buffer[3] = '.';
			buffer[4] = '-';
			break;
		case 5:
			buffer[0] = '.';
			buffer[1] = '.';
			buffer[2] = '.';
			buffer[3] = '.';
			buffer[4] = '.';
			break;
		case 6:
			buffer[0] = '-';
			buffer[1] = '.';
			buffer[2] = '.';
			buffer[3] = '.';
			buffer[4] = '.';
			break;
		case 7:
			buffer[0] = '-';
			buffer[1] = '-';
			buffer[2] = '.';
			buffer[3] = '.';
			buffer[4] = '.';
			break;
		case 8:
			buffer[0] = '-';
			buffer[1] = '-';
			buffer[2] = '-';
			buffer[3] = '.';
			buffer[4] = '.';
			break;
		case 9:
			buffer[0] = '-';
			buffer[1] = '-';
			buffer[2] = '-';
			buffer[3] = '-';
			buffer[4] = '.';
			break;
		default:
			buffer[0] = '-';
			buffer[1] = '-';
			buffer[2] = '-';
			buffer[3] = '-';
			buffer[4] = '-';
	}

	int i;
	for (i = 0; i < 5; i++){
		if (buffer[i] == '-')
			dash();
		else if (buffer[i] == '.')
			dot();
		else {
			__delay_cycles(600000);
		}
		__delay_cycles(600000);
	}

}

//translate long and short characters to a number
int morse_number(char buffer[]) {
	if ((buffer[0] == '-') && (buffer[1] == '-') && (buffer[2] == '-') && (buffer[3] == '-') && (buffer[4] == '-'))
		return 0;
	else if ((buffer[0] == '.') && (buffer[1] == '-') && (buffer[2] == '-') && (buffer[3] == '-') && (buffer[4] == '-'))
		return 1;
	else if ((buffer[0] == '.') && (buffer[1] == '.') && (buffer[2] == '-') && (buffer[3] == '-') && (buffer[4] == '-'))
		return 2;
	else if ((buffer[0] == '.') && (buffer[1] == '.') && (buffer[2] == '.') && (buffer[3] == '-') && (buffer[4] == '-'))
		return 3;
	else if ((buffer[0] == '.') && (buffer[1] == '.') && (buffer[2] == '.') && (buffer[3] == '.') && (buffer[4] == '-'))
		return 4;
	else if ((buffer[0] == '.') && (buffer[1] == '.') && (buffer[2] == '.') && (buffer[3] == '.') && (buffer[4] == '.'))
		return 5;
	else if ((buffer[0] == '-') && (buffer[1] == '.') && (buffer[2] == '.') && (buffer[3] == '.') && (buffer[4] == '.'))
		return 6;
	else if ((buffer[0] == '-') && (buffer[1] == '-') && (buffer[2] == '.') && (buffer[3] == '.') && (buffer[4] == '.'))
		return 7;
	else if ((buffer[0] == '-') && (buffer[1] == '-') && (buffer[2] == '-') && (buffer[3] == '.') && (buffer[4] == '.'))
		return 8;
	else if ((buffer[0] == '-') && (buffer[1] == '-') && (buffer[2] == '-') && (buffer[3] == '-') && (buffer[4] == '.'))
		return 9;
	else
		return 99;
}

interrupt void WDT_interval_handler(){
	char current_button2;

	// poll the button to see if we need to change state
	current_button2=(P1IN & BUTTON); // read button bit
	//current_button2 = BUTTON2;
	if (current_button2==0){ // did the button go down?
		inputState++; //time and see how long button was pressed

		//if the button was held for ~3 seconds, change the state
		if (inputState > 300){
			inputState = 0;
			state++;
		}
	}
	else {
		inputState = 0; //if button isn't being pressed reset the timer
	}

	//Input Mode: input number
	if ((state % 2) == 0) {

		//Reset Morse variables
		buttonPresses = 0;
		pressTime = 0;
		buffIt = 0;
		digit = 1;

		char current_button;            // variable to hold the state of the button
		timer++; //timer to determine what number will be selected as input

		//determine which digit to show, alternates every interrupt
		if (digitSwitch) {
			display_number(0, 3); //a 0 on digit 3 to show that it is input mode 0
			digitSwitch = 0;
		}
		else {
			display_number(numSelect, 1);
			digitSwitch = 1;
		}

		//if there has been no button press in ~4 seconds, take number as input
		if (timer > 500){
			number = numSelect;
		}

		//wait ~2 seconds and then run number through morse_code function and then reset
		if (timer > 750){
			morse_code(number);
			number = 0;
			timer = 0;
			numSelect = 0;
		}


		// poll the button to see if we need to change state
		current_button=(P1IN & BUTTON); // read button bit
		if ((current_button==0) && last_button_state){ // did the button go down?
			timer = 0; //reset timer at each button press
			numSelect++; //increment number that will be displayed
		}
		last_button_state=current_button; // remember the new button state
	}

	//Input Mode: input morse code
	else {
		//Reset 7 segment variables
		numSelect = 0;
		timer = 0;
		number = 0;
		digitSwitch = 1;

		char current_button;            // variable to hold the state of the button

		current_button=(P1IN & BUTTON); // read button bit

		display_number(1, 3); //display 1 on digit 3 to show that we are in input mode 1

		//if the button has been pressed 5 times and the button isn't being held translate and show number
		if ((buttonPresses == 5) && (current_button)){
			number = morse_number(buffer);
			if (digit){
				display_number(number, 1);
				digit = 0;
			}
			else {
				display_number(1, 3);
				digit = 1;
			}
		}

		if ((current_button==0) && last_button_state){ // did the button go down?
				buttonPresses++;

				//reset the display for a new input
				if (buttonPresses == 6) {
					P1OUT = 0xFF;
					buttonPresses = 0;
					buffIt = 0;
				}
		}

		//determine how long the button was pressed
		if (current_button==0){
			pressTime++;
		}

		last_button_state=current_button; // remember the new button state

		//determines if button was held for a long time or short time
		if ((pressTime >= 30) && (current_button) && (buttonPresses > 0)){
			buffer[buffIt] = '-';
			buffIt++;
			pressTime = 0;
		}
		else if ((pressTime < 30) && (pressTime > 0) && (current_button) && (buttonPresses > 0)) {
			buffer[buffIt] = '.';
			buffIt++;
			pressTime = 0;
		}
	}
}

// DECLARE function WDT_interval_handler as handler for interrupt 10
// using a macro defined in the msp430g2553.h include file
ISR_VECTOR(WDT_interval_handler, ".int10")
