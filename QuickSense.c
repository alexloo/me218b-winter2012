/****************************************************************************


 Description
         QuickSense.c is the source file for the custom sensor control 
         module. All sensors contained in this module are interrupt based.
         The sensors included are:
         -  Ball counter (PT0)
         -  Left tape sensor (PT1)
         -  Right tape sensor (PT2)
         -  Front bumper limit switch (PT3)
         -  Rear bumper limit switch (PT4)

 History
 When           Who	What/Why
 -------------- ---	--------
 02/21/12 14:53 adl  First pass
****************************************************************************/

// Includes ******************************************************
#include "ES_Configure.h"  /* get the typedefs for the components of an event*/
#include "ES_Framework.h"
#include <hidef.h>         /* common defines and macros */
#include <mc9s12e128.h>     /* derivative information */
#include <S12e128bits.h>    /* bit definitions  */
#include <Bin_Const.h>
#include <stdio.h>			/* standard i/o for print screens */
#include "MasterMachine.h"	/* prototypes for posting events */
#include "QuickSense.h"
#include "S12eVec.h"
#include "TimingConstants.h"
#include "BeaconDetection.h"
#include "stdint.h"

// Module Defines ************************************************
//#define DEBUG
#define DEBOUNCE_INTERVAL (100L * (unsigned long)(1 _MS_))
// Module Private Functions **************************************

// Module Variables **********************************************
static unsigned char BallsCollected = 0;
static unsigned int TIM0_uOverFlows = 0;
static unsigned int TIM1_uOverFlows = 0;

// Module Code ***************************************************
/****************************************************************************
Function
	QS_Initialize
	
Parameters
	None

Returns
	None

Description
	This function initializes the interrupt parameters for the ball counter,
	limit switches, and tape sensors.
****************************************************************************/
void QS_Initialize(void)
{
	// Initialize the timer system for interrupts on PT0 - PT4
	TIM0_TSCR1 = _S12_TEN; // Enable Timer 0
	TIM1_TSCR1 = _S12_TEN; // Enable Timer 1
	
	// Set the pre-scale to divide by 8 --> 24 MHz / 8 = 3 MHz
	// This results in 3 scaled clock ticks = 1 us
	TIM0_TSCR2 = _S12_PR1|_S12_PR0; // Divide by 8
	TIM1_TSCR2 = _S12_PR1|_S12_PR0; // Divide by 8
	
	// Set timer channels that will be output compare: NONE
	TIM0_TIOS = 0;
	TIM1_TIOS = 0;
	
	// Set timer channels for input capture
	// PT0 --> Timer 0, Channel 4
	// Ball Counter --> idles HIGH --> set to capture falling edges
	TIM0_TCTL3 |= _S12_EDG4B;
	
	// PT1 --> Timer 0, Channel 5 and PT2 --> Timer 0, Channel 6
	// Tape Sensors --> idles high --> set to capture falling edges
	TIM0_TCTL3 |= _S12_EDG5B|_S12_EDG6B;
	
	// PT3 --> Timer 0, Channel 7 and PT4 --> Timer1, Channel 4
	// Limit switches --> idle HIGH --> set to capture falling edges
	TIM0_TCTL3 |= _S12_EDG7B;
	TIM1_TCTL3 |= _S12_EDG4B;
	
	//Clear all flags
	TIM0_TFLG1 = _S12_C4F; // clear flag for Timer 0, Channel 4
	TIM0_TFLG1 = _S12_C5F; // clear flag for Timer 0, Channel 5
	TIM0_TFLG1 = _S12_C6F; // clear flag for Timer 0, Channel 6
	TIM0_TFLG1 = _S12_C7F; // clear flag for Timer 0, Channel 7
	TIM1_TFLG1 = _S12_C4F; // clear flag for Timer 1, Channel 4
	
	TIM0_TFLG2 = _S12_TOF; // Clear the Timer 0 overflow flag
	TIM1_TFLG2 = _S12_TOF; // Clear the Timer 1 overflow flag
	
	// Enable interrupts for Channels 4-7 on Timer 0
	//TIM0_TIE |= _S12_C4I|_S12_C5I|_S12_C6I|_S12_C7I;
	// Enable interrupts for Channel 4 on Timer 1
	//TIM1_TIE |= _S12_C4I;
	
	// Listed separately for debugging
	TIM0_TIE |= _S12_C4I; // ball counter
	//TIM0_TIE |= _S12_C5I|_S12_C6I; // tape sensors
	TIM0_TIE |= _S12_C7I; // front bumper
	TIM1_TIE |= _S12_C4I; // rear bumper
	
	// Enable interrupts for timer overflows on Timer 0 and Timer 1
	TIM0_TSCR2 |= _S12_TOI;
	TIM1_TSCR2 |= _S12_TOI;
	
	// Initialize the interrupts for beacon detection
	BeaconDetection_Init();
	
	EnableInterrupts; // Enable global interrupts
	
	printf("\r\nQuick sensing module initialization complete.");
}

/****************************************************************************
Function
	QS_QueryBallCount
	
Parameters
	None

Returns
	BallsCollected

Description
	This function passes back the module level static variable of 
	BallsCollected. This is so that the state machine can query the number
	of balls in the bin at will.
****************************************************************************/
unsigned char QS_QueryBallCount(void)
{
	return BallsCollected;
}

/****************************************************************************
Function
	QS_QueryTIM0Overflow
	
Parameters
	None

Returns
	TIM0_uOverFlows

Description
	This function passes back the module level static variable of 
	TIM0_uOverFlows. This is so that other modules with ISRs can keep a long
	clock.
****************************************************************************/
unsigned int QS_QueryTIM0Overflow(void)
{
	return TIM0_uOverFlows;
}

/****************************************************************************
Function
	QS_QueryTIM1Overflow
	
Parameters
	None

Returns
	TIM1_uOverFlows

Description
	This function passes back the module level static variable of 
	TIM1_uOverFlows. This is so that other modules with ISRs can keep a long
	clock.
****************************************************************************/
unsigned int QS_QueryTIM1Overflow(void)
{
	return TIM1_uOverFlows;
}

/***************************************************************************
private functions
***************************************************************************/
 
/***************************************************************************
interrupt service routines
***************************************************************************/
/****************************************************************************
Interrupt Service Routine
	TIMx_OverFlowCounter

Description
	This interrupt service routine increments a timer overflow counter 
	register. This ISR responds to overflows of Timer x.
****************************************************************************/
void interrupt _Vec_tim0ovf TIM0_OverFlowCounter(void)
{
	//DisableInterrupts;
	TIM0_uOverFlows++; // Increment timer overflow counter
	TIM0_TFLG2 = _S12_TOF; // Clear the timer overflow flag
	//EnableInterrupts;
	//printf("\r\nTimer 0 has overflowed.");
}

void interrupt _Vec_tim1ovf TIM1_OverFlowCounter(void)
{
	//DisableInterrupts;
	TIM1_uOverFlows++; // Increment timer overflow counter
	TIM1_TFLG2 = _S12_TOF; // Clear the timer overflow flag
	//EnableInterrupts;
	//printf("\r\nTimer 1 has overflowed.");
}

/****************************************************************************
Interrupt Service Routine
	BallCounter

Description
	This interrupt service routine increments a variable every time a ball
	interrupts the beam.
****************************************************************************/
void interrupt _Vec_tim0ch4 BallCounter(void)
{
	//DisableInterrupts;
	BallsCollected++; // Increment the ball counter
	TIM0_TFLG1 = _S12_C4F; // Clear the flag for Timer 0, Channel 4
	//EnableInterrupts;
	//printf("\r\nBall counter interrupt has triggered.");
}

/****************************************************************************
Interrupt Service Routine
	FoundTape_Left

Description
	This interrupt service routine posts an event when the left tape sensor 
	finds tape.
****************************************************************************/
void interrupt _Vec_tim0ch5 FoundTape_Left(void)
{
	static unsigned long TimeOfLastEdge = 0; // in clock ticks
	unsigned long TimeOfCurrentEdge = 0; // in clock ticks
	unsigned int CurrentICRegister = 0; // in clock ticks
	
	//DisableInterrupts;
	CurrentICRegister = TIM0_TC5;
	TIM0_TFLG1 = _S12_C5F; // Clear the flag for Timer 0, Channel 5
	//EnableInterrupts;

	// Calculate the time of the current edge in clock ticks
	TimeOfCurrentEdge = ((unsigned long)TIM0_uOverFlows<<16) + CurrentICRegister; 
	
	//	if ((TimeOfCurrentEdge - TimeOfLastEdge) > DEBOUNCE_INTERVAL)
   if (((TimeOfCurrentEdge - TimeOfLastEdge) > DEBOUNCE_INTERVAL) && (QueryMasterMachine() == Defending)) // check debounce interval
	{
		ES_Event ThisEvent; // Create an event
		ThisEvent.EventType = ES_LEFT_TAPE_DETECTED; // Save the current event type
		ThisEvent.EventParam = (uint16_t)(TimeOfCurrentEdge/(1 _SEC_)); // Pass input compare of the current event
		PostMasterMachine(ThisEvent); // Post the event to post list 00
	}
	TimeOfLastEdge = TimeOfCurrentEdge; // Update the last time, in clock ticks
	//printf("\r\nLEFT tape interrupt has triggered.");
	//EnableInterrupts;
}

/****************************************************************************
Interrupt Service Routine
	FoundTape_Right

Description
	This interrupt service routine posts an event when the left tape sensor 
	finds tape.
****************************************************************************/
void interrupt _Vec_tim0ch6 FoundTape_Right(void)
{
	static unsigned long TimeOfLastEdge = 0; // in clock ticks
	unsigned long TimeOfCurrentEdge = 0; // in clock ticks
	unsigned int CurrentICRegister = 0; // in clock ticks
	
	//DisableInterrupts;
	CurrentICRegister = TIM0_TC6;
	TIM0_TFLG1 = _S12_C6F; // Clear the flag for Timer 0, Channel 6
	//EnableInterrupts;
	
	// Calculate the time of the current edge in clock ticks
	TimeOfCurrentEdge = ((unsigned long)TIM0_uOverFlows<<16) + CurrentICRegister;
	
	//	if ((TimeOfCurrentEdge - TimeOfLastEdge) > DEBOUNCE_INTERVAL)
	if (((TimeOfCurrentEdge - TimeOfLastEdge) > DEBOUNCE_INTERVAL) && (QueryMasterMachine() == Defending)) // check debounce interval
	{
		ES_Event ThisEvent; // Create an event
		ThisEvent.EventType = ES_RIGHT_TAPE_DETECTED; // Save the current event type
		ThisEvent.EventParam = (uint16_t)(TimeOfCurrentEdge/(1 _SEC_)); // Pass input compare of the current event
		PostMasterMachine(ThisEvent); // Post the event to post list 00
	}
	TimeOfLastEdge = TimeOfCurrentEdge; // Update the last time, in clock ticks
	//printf("\r\nRIGHT tape interrupt has triggered.");
	//EnableInterrupts;
}

/****************************************************************************
Interrupt Service Routine
	Bumper_Front

Description
	This interrupt service routine posts an event front bumper's limit 
	switch is triggered.
****************************************************************************/
void interrupt _Vec_tim0ch7 Bumper_Front(void)
{
	static unsigned long TimeOfLastEdge = 0; // in clock ticks
	unsigned long TimeOfCurrentEdge = 0; // in clock ticks
	unsigned int CurrentICRegister = 0; // in clock ticks
	
	//EnableInterrupts;
	CurrentICRegister = TIM0_TC7;
	TIM0_TFLG1 = _S12_C7F; // Clear the flag for Timer 0, Channel 7
	//EnableInterrupts;
	
	// Calculate the time of the current edge in clock ticks
	TimeOfCurrentEdge = ((unsigned long)TIM0_uOverFlows<<16) + CurrentICRegister;
	
	//if(1)
	if ((TimeOfCurrentEdge - TimeOfLastEdge) > DEBOUNCE_INTERVAL) // check debounce interval
	{
		ES_Event ThisEvent; // Create an event
		ThisEvent.EventType = ES_FRONT_BUMPED; // Save the current event type
		ThisEvent.EventParam = (uint16_t)(TimeOfCurrentEdge/(1 _SEC_)); // Pass input compare of the current event
		PostMasterMachine(ThisEvent); // Post the event to post list 00
	}
	TimeOfLastEdge = TimeOfCurrentEdge; // Update the last time, in clock ticks
	//printf("\r\nFRONT bumper interrupt has triggered.");
	//EnableInterrupts;
}

/****************************************************************************
Interrupt Service Routine
	Bumper_Rear

Description
	This interrupt service routine posts an event rear bumper's limit 
	switch is triggered.
****************************************************************************/
void interrupt _Vec_tim1ch4 Bumper_Rear(void)
{
	static unsigned long TimeOfLastEdge = 0; // in clock ticks
	unsigned long TimeOfCurrentEdge = 0; // in clock ticks
	unsigned int CurrentICRegister = 0; // in clock ticks
	
	//DisableInterrupts;
	CurrentICRegister = TIM1_TC4;
	TIM1_TFLG1 = _S12_C4F; // Clear the flag for Timer 1, Channel 4
	//EnableInterrupts;
	
	// Calculate the time of the current edge in clock ticks
	TimeOfCurrentEdge = ((unsigned long)TIM1_uOverFlows<<16) + CurrentICRegister;

   //if(1)
	if ((TimeOfCurrentEdge - TimeOfLastEdge) > DEBOUNCE_INTERVAL) // check debounce interval
	{

		ES_Event ThisEvent; // Create an event
		ThisEvent.EventType = ES_REAR_BUMPED; // Save the current event type
		ThisEvent.EventParam = (uint16_t)(TimeOfCurrentEdge/(1 _SEC_));; // Pass input compare of the current event
		PostMasterMachine(ThisEvent); // Post the event to post list 00	
	}

	TimeOfLastEdge = TimeOfCurrentEdge; // Update the last time, in clock ticks
	//printf("\r\nREAR bumper interrupt has triggered.");
	//EnableInterrupts;
}