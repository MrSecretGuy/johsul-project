/*
 * Pixel.c
 *
 *  Created on: 23-10-2023
 *      Author: [REDACTED]
 */

#include "Pixel.h"
// Own global variables

#define channel *(uint8_t *) 0x41220000
#define control *(uint8_t *) 0x41220008



//Table for pixel dots.
//				 dots[X][Y][COLOR]
volatile uint8_t dots[8][8][3]={0};


// Here the setup operations for the LED matrix will be performed
void setup(){


	//reseting screen at start is a MUST to operation (Set RST-pin to 1).

	// Written "dumb" pointers for the channels/ banks??????
	*(uint8_t*)0x41220008=0;
	*(uint8_t*)0x41220000=0;
	//changing control signal RST to 1 and SDA to 1
	usleep(500);
	*(uint8_t*)0x4122008 |= 0b00000001;
	usleep(500);
	*(uint8_t*)0x41220008 &=~0b00000001;
	usleep(500);
	*(uint8_t*)0x41220008 |= 0b00000001;
	*(uint8_t*)0x41220008 |= 0b00010000;


	//Write code that sets 6-bit values in register of DM163 chip. Recommended that every bit in that register is set to 1. 6-bits and 24 "bytes", so some kind of loop structure could be nice.
	//24*6 bits needs to be transmitted

	for (uint8_t counter=0; counter < 6; counter++){
		for (uint8_t leds= 0; leds < 8; leds++){
			for (uint8_t rgb = 0; rgb < 3; rgb++){
				*(uint8_t*)0x41220008 |= 0b00001000;
				*(uint8_t*)0x41220008 &=~0b00001000;
			}
		}
	}



	//Final thing in this function is to set SB-bit to 1 to enable transmission to 8-bit register.

	*(uint8_t*)0x41220008 |= 0b00000100;

}

//Change value of one pixel at led matrix. This function is only used for changing values of dots array
void SetPixel(uint8_t x,uint8_t y, uint8_t r, uint8_t g, uint8_t b){

	//Hint: you can invert Y-axis quite easily with 7-y
	dots[x][y][0]=b;
	//Write rest of two lines of code required to make this function work properly (green and red colors to array).
	dots[x][y][1]=g;
	dots[x][y][2]=r;


}


//Put new data to led matrix. Hint: This function is supposed to send 24-bytes and parameter x is for channel x-coordinate.
void run(uint8_t x){



	//Write code that writes data to led matrix driver (8-bit data). Use values from dots array
	//Hint: use nested loops (loops inside loops)
	//Hint2: loop iterations are 8,3,8 (pixels,color,8-bit data)

	// making sure the value of latch is 0
	control &= 0b11111101;


	//sending data to 8-bit register
	for (uint8_t y = 0; y < 8; y++){
		for (uint8_t color = 0; color < 3; color++){
			// Reading some fucking dots
			uint8_t runner;
			runner = dots[x][y][color];

			for (uint8_t byte = 0; byte < 8; byte++){

				// Datan arvon tulkinta 0 tai 1
				if(runner &= 0b10000000){
					control |= 0b10000;
				} else {
					control &= ~0b10000;
				}
				// MSB kirjoittaminen reksiteriin
				control |= 0b1000;
				control &= ~0b1000;

				taisto <<= 1;
			}
		}
	}

	// Latch() operation
	latch();

	//making sure the sck-bit is 0
	control &=~ 0b1000;


}

//Latch signal. See colorsshield.pdf or DM163.pdf in project folder on how latching works
void latch(){

	*(uint8_t*)0x41220008 |= 0b00000010;
	*(uint8_t*)0x41220008 &=~0b00000010;

}


//Set one line (channel) as active, one at a time.
void open_line(uint8_t x){



	// Switch case for different channels
	switch(x){

		case 0: channel = 0b1; break;
		case 1: channel = 0b10; break;
		case 2: channel = 0b100; break;
		case 3: channel = 0b1000; break;
		case 4: channel = 0b10000; break;
		case 5: channel = 0b100000; break;
		case 6: channel = 0b1000000; break;
		case 7: channel = 0b10000000; break;
		default: channel = 0;
	}

}



