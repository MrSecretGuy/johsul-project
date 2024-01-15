/******************************************************************************
*
* Copyright (C) 2009 - 2014 Xilinx, Inc.  All rights reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* Use of the Software is limited solely to applications:
* (a) running on a Xilinx device, or
* (b) that interact with a Xilinx device through a bus or interconnect.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
* OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
* Except as contained in this notice, the name of the Xilinx shall not be used
* in advertising or otherwise to promote the sale, use or other dealings in
* this Software without prior written authorization from Xilinx.
*
******************************************************************************/

/*
 *
 *
 * This application configures UART 16550 to baud rate 9600.
 * PS7 UART (Zynq) is not initialized by this application, since
 * bootrom/bsp configures it to baud rate 115200
 *
 * ------------------------------------------------
 * | UART TYPE   BAUD RATE                        |
 * ------------------------------------------------
 *   uartns550   9600
 *   uartlite    Configurable only in HW design
 *   ps7_uart    115200 (configured by bootrom/bsp)
 */

// Main program for exercise

//****************************************************
//By default, every output used in this exercise is 0
//****************************************************
#include <stdio.h>
#include "platform.h"
#include "xil_printf.h"
#include "sleep.h"
#include "xgpiops.h"
#include "xttcps.h"
#include "xscugic.h"
#include "xparameters.h"
#include "Pixel.h"
#include "Interrupt_setup.h"

//own includes
#include <stdbool.h>

//********************************************************************
//***************TRY TO READ COMMENTS*********************************
//********************************************************************

//***Hint: Use sleep(x)  or usleep(x) if you want some delays.****
//***To call assembler code found in blinker.S, call it using: blinker();***


//Comment this if you want to disable all interrupts
#define enable_interrupts

//Own global variables:
#define rgb_leds *(uint8_t *) 0x41240000
#define button_leds *(uint8_t *) 0x41200000

//which channel is open
int channel_open = 0;

//Cannon variables
unsigned int cannon;
bool can_move = true;
unsigned int stabilizing_movement = 0;

//Alien variables
unsigned int alien = 0;
bool alien_direction = true;
unsigned int alien_slower = 0;
bool alien_hit = false;
unsigned int alien_flash_counter = 0;
unsigned int timesAlienHit = 0;


//Projectile variables
unsigned int projectileY = 7;
unsigned int projectileX;
bool projectile_shot = false;

// Boolean for winning the game
bool victory = false;


/***************************************************************************************
Name: [REDACTED]
Student number: [REDACTED]

Name: [REDACTED]
Student number: [REDACTED]

Name:
Student number:

Tick boxes that you have coded

Led-matrix driver		Game		    Assembler
	[x]					[x]					[]

Brief description:

*****************************************************************************************/




int main()
{
	//**DO NOT REMOVE THIS****
	    init_platform();
	//************************


#ifdef	enable_interrupts
	    init_interrupts();
#endif


	    //setup screen
	    setup();

	    //testing code
	    // Stored some good colors for the game
	    //SetPixel(7,0,186,85,211);
	    //SetPixel(7,1,255,0,0);
	    //SetPixel(7,2,0,255,0);
	    //SetPixel(7,3,0,0,211);
	    //SetPixel(7,4,201,30,180);
	    //SetPixel(7,6,0,180,190);

	    //Drawing spaceship
	    cannon = 4;
	    DrawShip(cannon);
	    button_leds |= 0b1011;


	    Xil_ExceptionEnable();



	    //Try to avoid writing any code in the main loop.
		while(1){


		}


		cleanup_platform();
		return 0;
}


//Timer interrupt handler for led matrix update. Frequency is 800 Hz
void TickHandler(void *CallBackRef){
	//Don't remove this
	uint32_t StatusEvent;

	//exceptions must be disabled when updating screen
	Xil_ExceptionDisable();



	//****Write code here ****
	if(channel_open <= 7){
		open_line(8);
		run(channel_open);
		open_line(channel_open);
		channel_open++;

	} else {
		channel_open = 0;
		open_line(8);
		run(channel_open);
		open_line(channel_open);
	}

	// Ship can only move in 0,2 seconds intervals
	if (!can_move){

		stabilizing_movement++;
		if(stabilizing_movement == 150){
				can_move = true;
				stabilizing_movement = 0;
		}
	}



	//****END OF OWN CODE*****************

	//*********clear timer interrupt status. DO NOT REMOVE********
	StatusEvent = XTtcPs_GetInterruptStatus((XTtcPs *)CallBackRef);
	XTtcPs_ClearInterruptStatus((XTtcPs *)CallBackRef, StatusEvent);
	//*************************************************************
	//enable exceptions
	Xil_ExceptionEnable();
}


//Timer interrupt for moving alien, shooting... Frequency is 10 Hz by default
void TickHandler1(void *CallBackRef){

	//Don't remove this
	uint32_t StatusEvent;

	//****Write code here ****

	//Drawing an moving alien

	if (alien_slower > 1 && !alien_hit && !victory){
		MoveAlien();
		alien_slower = 0;
	} else {
		alien_slower++;
	}

	if (alien_hit && !victory){
		FlashAlien();
	}


	if (projectile_shot && !victory){
		MoveProjectile(projectileX);
	}

	if (timesAlienHit == 4){
		victory = true;
		// Setting old pixels off
		EraseShip(cannon);
		// Setting new pixels on
		VictoryCelebration(true);
		button_leds = 0;
		timesAlienHit++;
	}





	//****END OF OWN CODE*****************
	//clear timer interrupt status. DO NOT REMOVE
	StatusEvent = XTtcPs_GetInterruptStatus((XTtcPs *)CallBackRef);
	XTtcPs_ClearInterruptStatus((XTtcPs *)CallBackRef, StatusEvent);

}


//Interrupt handler for switches and buttons.
//Reading Status will tell which button or switch was used
//Bank information is useless in this exercise
void ButtonHandler(void *CallBackRef, u32 Bank, u32 Status){
	//****Write code here ****

	//Hint: Status==0x01 ->btn0, Status==0x02->btn1, Status==0x04->btn2, Status==0x08-> btn3, Status==0x10->SW0, Status==0x20 -> SW1

	//If true, btn0 was used to trigger interrupt
	// Button to move spaceship right
	if(Status==0x01){

		if (cannon < 6 && can_move && !victory){
			EraseShip(cannon);
			cannon++;
			DrawShip(cannon);
			can_move = false;
		}

	// Button to move spaceship left
	} else if (Status==0x02){

		if (cannon > 1 && can_move && !victory){
			EraseShip(cannon);
			cannon--;
			DrawShip(cannon);
			can_move = false;
		}

	// Button to shoot projectile
	} else if (Status==0x08){

		if (!alien_hit && !victory){
			projectileX = cannon;
			projectile_shot = true;
		}

	// Button to reset game
	} else if (Status==0x20){
		victory = false;
		VictoryCelebration(false);
		timesAlienHit = 0;
		DrawShip(cannon);
		rgb_leds = 0;
		button_leds |= 0b1011;
	}








	//****END OF OWN CODE*****************
}

//****Write code here ****


//Function to draw ship in new coordinates
void DrawShip(int coordinate){

	SetPixel(coordinate -1,7,0,180,190);
	SetPixel(coordinate,7,0,180,190);
	SetPixel(coordinate,6,0,180,190);
	SetPixel(coordinate + 1,7,0,180,190);
}


//Function to delete old ship in old coordinates
void EraseShip(int coordinate){

	SetPixel(coordinate -1,7,0,0,0);
	SetPixel(coordinate,7,0,0,0);
	SetPixel(coordinate,6,0,0,0);
	SetPixel(coordinate + 1,7,0,0,0);
}


//Function to see if alien is hit
bool IsAlienHit(){
	if (alien == projectileX){
		alien_hit = true;
		timesAlienHit++;

		if (timesAlienHit == 1){
			rgb_leds |= 0b000101;
		} else if (timesAlienHit == 2){
			rgb_leds |= 0b101000;
		}
		return true;

	} else {
		SetPixel(projectileX, projectileY-1, 201,30,180);
		return false;
	}
}


// Function to shoot a projectile
void MoveProjectile(unsigned int coordinate){
	// Changing color of ships cannon
	if (projectileY == 7){
		SetPixel(projectileX,6,201,30,180);
		projectileY--;

	// Moving projectile and resetting the color of cannon
	} else if (projectileY == 6){
		SetPixel(projectileX,6,0,180,190);
		SetPixel(projectileX,5,201,30,180);
		projectileY--;

	// Moving projectile forward and shutting previous LEDs
	} else if (projectileY < 6 && projectileY > 1){
		SetPixel(projectileX, projectileY, 0,0,0);
		SetPixel(projectileX, projectileY-1, 201,30,180);
		projectileY--;

	//Projectile will reach the same y-coordinate with alien
	} else if (projectileY == 1){
		SetPixel(projectileX, projectileY, 0,0,0);

		// making sure if alien is hit, else continuing loop
		if(IsAlienHit()){
			projectileY = 7;
			projectile_shot = false;
		} else {
			projectileY--;
		}

	} else {
		SetPixel(projectileX, projectileY,0,0,0);
		projectile_shot=false;
		projectileY = 7;
	}

}



//Flashing alien couple times when it is hit
void FlashAlien(){

	if (alien_flash_counter == 5){
		alien_hit = false;
		alien_flash_counter = 0;
		SetPixel(alien,0,0,0,0);

		//last time before victory cycle
		if (timesAlienHit == 3){
			timesAlienHit++;
		}

	} else if (alien_flash_counter % 2 == 0){
		SetPixel(alien,0,220,20,20);
		alien_flash_counter++;
	} else {
		SetPixel(alien,0,0,210,0);
		alien_flash_counter++;
	}
}

// Function to move alien
void MoveAlien(){

	// Alien moving right, value of alien growing
	if(alien_direction){

		SetPixel(alien,0,0,0,0);

		if(alien <= 6){
			alien++;

		} else {
			alien_direction = false;
			alien--;
		}

		SetPixel(alien,0,0,210,0);

	// Alien moving left, value of alien lowering
	} else {

		SetPixel(alien,0,0,0,0);

		if(alien >= 1){
			alien--;

		} else {
			alien++;
			alien_direction = true;
		}

		SetPixel(alien,0,0,210,0);
	}
}

// Cool celebration for winning the game
void VictoryCelebration(bool on){

	// Setting new victory pixels
	if (on){
		SetPixel(2,1,0,220,220);
		SetPixel(2,2,0,220,220);
		SetPixel(5,1,0,220,220);
		SetPixel(5,2,0,220,220);
		SetPixel(1,4,0,220,220);
		SetPixel(2,5,0,220,220);
		SetPixel(3,5,0,220,220);
		SetPixel(4,5,0,220,220);
		SetPixel(5,5,0,220,220);
		SetPixel(6,4,0,220,220);

	// if false, shutting them down
	} else {
		SetPixel(2,1,0,0,0);
		SetPixel(2,2,0,0,0);
		SetPixel(5,1,0,0,0);
		SetPixel(5,2,0,0,0);
		SetPixel(1,4,0,0,0);
		SetPixel(2,5,0,0,0);
		SetPixel(3,5,0,0,0);
		SetPixel(4,5,0,0,0);
		SetPixel(5,5,0,0,0);
		SetPixel(6,4,0,0,0);
	}
}


	//****END OF OWN CODE****