/****************************************************************************
Description
                  SideID.h is the header file for the custom module that identifies
         which side that the robot is starting on.
         
 History
 When           Who	What/Why
 -------------- ---	--------
 02/24/12 15:38 adl  Changed RED_LED_PIN to Port P0; 
                     LED control now on physical neighbors
 02/22/12 16:29 adl  First pass
****************************************************************************/

// Includes ******************************************************
#include "ES_Configure.h"
#include "ES_Framework.h"
#include <hidef.h>         /* common defines and macros */
#include <mc9s12e128.h>     /* derivative information */
#include <S12e128bits.h>    /* bit definitions  */
#include <Bin_Const.h>
#include <stdio.h>			/* standard i/o for print screens */
#include "ES_Events.h"		/* definition for the ES_Event struct */
#include "ES_PostList.h"	/* prototypes for posting events */
#include "SideID.h"

// Module Defines ************************************************

// Module Private Functions **************************************

// Module Variables **********************************************
unsigned char TeamColor = 0;

// Module Code ***************************************************
/****************************************************************************
Function
	ID_Initialize
	
Parameters
	None

Returns
	None

Description
	This function initializes the module that identifies the robot's side.
****************************************************************************/
void ID_Initialize(void)
{
	// Set ports P2 and P3 to write
	DDRP |= BLUE_LED_PIN | RED_LED_PIN;
	// Write 0 to ports P2 and P3
	PTP &= ~(BLUE_LED_PIN | RED_LED_PIN);
}  //End of ID_Initialize

/****************************************************************************
Function
	ID_IdentifySide
	
Parameters
	BeaconSeen

Returns
	None

Description
	This function lights an indicator light based on the beacon detected 
	prior to the start of the game.
****************************************************************************/
void ID_IdentifySide(unsigned char BeaconSeen)
{   
	// If the robot is seeing a beacon on the red (left) side of the field
	if ((BeaconSeen == BS_BEACON1) | (BeaconSeen == BS_BEACON4))
	{
	   //Set E128 to light up RedLED
		PTP |= RED_LED_PIN;
		// Set TeamColor to Red
		TeamColor = RED_TEAM;
	}
	//Else if the robot is seeing a beacon on the blue (right) side of the field
	else if ((BeaconSeen == BS_BEACON2) | (BeaconSeen == BS_BEACON3))
	{
		// Set E128 to lightup BlueLED
		PTP |= BLUE_LED_PIN;
	   // Set TeamColor to Blue
		TeamColor = BLUE_TEAM;
	}  //Endif
}  //End of ID_IdentifySide

/****************************************************************************
Function
	ID_QuerySide
	
Parameters
	None

Returns
	TeamColor

Description
	This function returns the team color for the bot.
****************************************************************************/
unsigned char ID_QuerySide(void)
{
   //Return TeamColor
   return TeamColor;
}  //End of ID_QuerySide

/****************************************************************************
Function
	ID_AllLights
	
Parameters
	None

Returns
	None

Description
	This function turns on both lights to indicate game over.
****************************************************************************/
void ID_AllLights(void)
{
   // Turn on both the red and blue light
   PTP |= RED_LED_PIN|BLUE_LED_PIN;
}  //End of ID_AllLights