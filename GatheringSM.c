/****************************************************************************
 Module
   GatheringSM.c

 Revision
   1.0.1

 Description
   This is a template file for implementing flat state machines under the 
   Gen2 Events and Services Framework.

 Notes

 History
 When           Who     What/Why
 -------------- ---     --------
 02/25/12 17:44 adl     Tailoring to be a gathering mode FSM
 01/15/12 11:12 jec      revisions for Gen2 framework
 11/07/11 11:26 jec      made the queue static
 10/30/11 17:59 jec      fixed references to CurrentEvent in RunTemplateSM()
 10/23/11 18:20 jec      began conversion from SMTemplate.c (02/20/07 rev)
****************************************************************************/
/*----------------------------- Include Files -----------------------------*/
/* include header files for this state machine as well as any machines at the
   next lower level in the hierarchy that are sub-machines to this machine
*/
#include "ES_Configure.h"
#include "ES_Framework.h"
#include "GatheringSM.h"

// Module Headers
#include "MotorDriver.h"
#include "TimingConstants.h"
#include "BinControl.h"

#include <stdio.h>
#include "EventPrinter.h"

/*----------------------------- Module Defines ----------------------------*/
//#define DEBUG_WALL_PUSHING

#define WALL_BUMP_TOLERANCE 1
#define MAX_BUMPS 2

/*---------------------------- Module Functions ---------------------------*/
/* prototypes for private functions for this machine.They should be functions
   relevant to the behavior of this state machine
*/
static ES_Event DuringFSA(ES_Event);
static ES_Event DuringHSA(ES_Event);
static ES_Event DuringTurningLeft(ES_Event);
static ES_Event DuringTurningRight(ES_Event);
static ES_Event DuringFullReverse(ES_Event);
static ES_Event DuringHalfReverse(ES_Event);

static TurnDirection_t QueryNextTurnDirection(void);


/*---------------------------- Module Variables ---------------------------*/
// everybody needs a state variable, you may need others as well.
// type of state variable should match htat of enum in header file
static GatheringState_t CurrentState;
static TurnDirection_t TurnDirection = Right;

/*------------------------------ Module Code ------------------------------*/
/****************************************************************************
 Function
    RunGatheringSM

 Parameters
   ES_Event : the event to process

 Returns
   ES_Event, ES_NO_EVENT if no error ES_ERROR otherwise

 Description
   add your description here
 Notes
   uses nested switch/case to implement the machine.
 Author
   J. Edward Carryer, 01/15/12, 15:23
****************************************************************************/
ES_Event RunGatheringSM( ES_Event ThisEvent )
{
  	boolean MakeTransition = False; // are we making a state transition?
  	GatheringState_t NextState = CurrentState;
  	ES_Event ReturnEvent = ThisEvent; // Assume we are not consuming event
  	
  	#ifdef DEBUG_WALL_PUSHING
  	static unsigned char WallAngleAtImpact = 0;
  	static unsigned char WallBumpCounter = 0;
  	#endif
  
  	EventPrinter(ThisEvent);
  	switch (CurrentState)
  	{
      case FullSpeedAhead:
         printf("\r\nIn the FullSpeedAhead state.");
      	// Execute during function for FSA. Entry and exit are processed here
      	ThisEvent = DuringFSA(ThisEvent); 
      	
      	// Process events
      	// Check if there is an event to respond to
      	if (ThisEvent.EventType != ES_NO_EVENT)
      	{
      		// Switch on event types
      		switch (ThisEvent.EventType)
      		{
      		   
      			case ES_LEFT_TAPE_DETECTED: // detected tape on left sensor
      				NextState = HalfSpeedAhead; // determine what the next state will be
      				MakeTransition = True; // mark that we are making a transition
      				ReturnEvent.EventType = ES_NO_EVENT; // consume event
      			break;
      			
      			case ES_RIGHT_TAPE_DETECTED: // detected tape on left sensor
      				NextState = HalfSpeedAhead; // determine what the next state will be
      				MakeTransition = True; // mark that we are making a transition
      				ReturnEvent.EventType = ES_NO_EVENT; // consume event
      			break;
      			      			
      			case ES_FRONT_BUMPED: // front bumper triggered
      			   #ifdef DEBUG_WALL_PUSHING
      				WallAngleAtImpact = Get_WallAngle(); // store the wall angle at impact
      				#endif
      				NextState = FullReverse; // Set next state to full reverse
      				MakeTransition = True; // mark that we are making a transition
      				ReturnEvent.EventType = ES_NO_EVENT; // consume event
      			break;
      		}
      	}
      break; // End FullSpeedAhead state

      case HalfSpeedAhead:
         printf("\r\nIn the HalfSpeedAhead state.");
      	// Execute during function for HSA. Entry and exit are processed here
      	ThisEvent = DuringHSA(ThisEvent);
      	
      	// Process events
      	// Check if there is an event to respond to
      	if (ThisEvent.EventType != ES_NO_EVENT)
      	{
      		// Switch on event types
      		switch (ThisEvent.EventType)
      		{
      			case ES_TIMEOUT: // timer expired
      			// Verify that the motion timer expired
      			if (ThisEvent.EventParam == MOTION_TIMER)
      			{
      				NextState = FullSpeedAhead; // Set next state to 100% speed
      				MakeTransition = True; // mark that we are making a transition
      				ReturnEvent.EventType = ES_NO_EVENT; // consume event
      			}
      			break;
      			
      			case ES_FRONT_BUMPED: // front bumper triggered
      				NextState = FullReverse; // Set next state to full reverse
      				MakeTransition = True; // mark that we are making a transition
      				ReturnEvent.EventType = ES_NO_EVENT; // consume event
      			break;
      		} // End switch on event types	
      	} // End guard against no event
   	break; // End HalfSpeedAhead state
   	
   	case TurningLeft:
   	   printf("\r\nIn the TurningLeft state.");
   		// Execute during function for TurningLeft. Entry and exit are processed here
      	ThisEvent = DuringTurningLeft(ThisEvent);
      	
      	// Process events
      	// Check if there is an event to respond to
      	if (ThisEvent.EventType != ES_NO_EVENT)
      	{
      		switch (ThisEvent.EventType)
      		{
      			case ES_FRONT_BUMPED: // front bumper triggered
      				NextState = HalfReverse; // Set next state to half reverse
      				MakeTransition = True; // mark that we are making a transition
      				ReturnEvent.EventType = ES_NO_EVENT; // consume event
      			break;
      			
      			case ES_TIMEOUT: // timer expired
      				// Verify that the motion timer expired
      				if (ThisEvent.EventParam == MOTION_TIMER)
      				{
      					NextState = FullSpeedAhead; // set next state to 100% forward
      					MakeTransition = True; // mark that we are making a transition
      					ReturnEvent.EventType = ES_NO_EVENT; // consume event
      				}
      			break;
      		} // End event type switch
      	} // End guard on no event
   	break; // End TurningLeft state
   	
   	case TurningRight:
   	   printf("\r\nIn the TurningRight state.");
   		// Execute during function for TurningRight. Entry and exit are processed here
      	ThisEvent = DuringTurningRight(ThisEvent);
      	
      	// Process events
      	// Check if there is an event to respond to
      	if (ThisEvent.EventType != ES_NO_EVENT)
      	{
      		switch (ThisEvent.EventType)
      		{
      			case ES_FRONT_BUMPED: // front bumper triggered
      				NextState = HalfReverse; // Set next state to half reverse
      				MakeTransition = True; // mark that we are making a transition
      				ReturnEvent.EventType = ES_NO_EVENT; // consume event
      			break;
      			
      			case ES_TIMEOUT: // timer expired
      				// Verify that the motion timer expired
      				if (ThisEvent.EventParam == MOTION_TIMER)
      				{
      					NextState = FullSpeedAhead; // set next state to 100% forward
      					MakeTransition = True; // mark that we are making a transition
      					ReturnEvent.EventType = ES_NO_EVENT; // consume event
      				}
      			break;
      		} // End event type switch
      	} // End guard against no event
   	break; // End TurningRight state
   	
   	case FullReverse:
   		printf("\r\nIn the FullReverse state.");
   		// Execute during function for FullReverse. Entry and exit are processed here
      	ThisEvent = DuringFullReverse(ThisEvent);
      	
      	// Process events
      	// Check if there is an event to respond to
      	if (ThisEvent.EventType != ES_NO_EVENT)
      	{
      		// Switch on event types
      		switch (ThisEvent.EventType)
      		{
      			case ES_TIMEOUT: // timer expired
      				// Verify that the motion timer expired
      				if (ThisEvent.EventParam == MOTION_TIMER)
      				{
      					// Execute a turn to get a new heading
      					// Turn in alternating directions using the TurnDirection variable
      					if (TurnDirection == Right)
      					{
   					   	NextState = TurningRight; // Set next state to turn right
   					   	TurnDirection = QueryNextTurnDirection(); // update turn direction
      					}
      					else
      					{
      					   NextState = TurningLeft; // Set next state to turn left
      					   TurnDirection = QueryNextTurnDirection(); // updated turn direction
      					}		
      					
      					MakeTransition = True; // mark that we are making a transition
      					ReturnEvent.EventType = ES_NO_EVENT; // consume event
      				}
      				
      				#ifdef DEBUG_WALL_PUSHING
      				else if (ThisEvent.EventParam == WALL_BUMP_TIMER)
      				{
      				   // RESERVED FOR WALL PUSHING ALGORITHM
      				   unsigned char CurrentWallAngle = Get_WallAngle(); // store current wall angle
      				   int DeltaWallAngle = (int)CurrentWallAngle - (int)WallAngleAtImpact;
      				   DeltaWallAngle = abs(DeltaWallAngle);
      				   
      				   if ((DeltaWallAngle > WALL_BUMP_TOLERANCE) && (WallBumpCounter < MAX_BUMPS))
      				   {
      				      // We likely hit the wall
      				      printf("\r\nWe likely hit the wall. The wall changed %i degrees.", DeltaWallAngle);
      				      printf("\r\nThis is our %u bump this iteration.", WallBumpCounter);
      				      WallBumpCounter = WallBumpCounter++; // increment the counter
      				      if (WallBumpCounter >= MAX_BUMPS) // if we hit max number of bumps
      				      {
      				         WallBumpCounter = 0; // reset the counter
      				      }
      				      // Go forward again
      				      NextState = FullSpeedAhead; // move to going forward state
      				      MakeTransition = True; // mark that we are making a transition    				      
      				      ReturnEvent.EventType = ES_NO_EVENT; // consume the event
      				   }      				   
      				}
      				#endif
      			break; // End ES_TIMEOUT event
      		} // End event type switch
      	} // End guard on no event
   	break; // End FullReverse state
   	
   	case HalfReverse:
   		printf("\r\nIn the HalfReverse state.");
   		// Execute during function for HalfReverse. Entry and exit are processed here
      	ThisEvent = DuringHalfReverse(ThisEvent);
      	
      	// Process events
      	// Check if there is an event to respond to
      	if (ThisEvent.EventType != ES_NO_EVENT)
      	{
      		// Switch on event types
      		switch (ThisEvent.EventType)
      		{
      			case ES_TIMEOUT: // timer expired
      				// Verify that the motion timer expired
      				if (ThisEvent.EventParam == MOTION_TIMER)
      				{
      					// Execute a turn to get a new heading
      					// Turn in alternating directions using the TurnDirection variable
      					if (TurnDirection == Right)
      					{
      					   NextState = TurningRight; // Set next state to turn right
      					   TurnDirection = QueryNextTurnDirection();
      					}
      					else
      					{
      					   NextState = TurningLeft; // Set next state to turn left
      					   TurnDirection = QueryNextTurnDirection();
      					}		
      					
      					MakeTransition = True; // mark that we are making a transition
      					ReturnEvent.EventType = ES_NO_EVENT; // consume event
      				}
      				
      			break; // End ES_TIMEOUT event
      		} // End event type switch
      	} // End guard on no event
   	break; // End HalfReverse state
      
  	}// end switch on Current State
  	
  	// If we are making a state transition
  	if (MakeTransition == True)
  	{
  		// Execute exit function for current state
  		ThisEvent.EventType = ES_EXIT;
  		RunGatheringSM(ThisEvent);
  		
  		CurrentState = NextState; // Update state variable
  		
  		// Execute entry function for the new state
  		ThisEvent.EventType = ES_ENTRY;
  		RunGatheringSM(ThisEvent);
  	}
  	return ReturnEvent;
}

/****************************************************************************
 Function
     QueryGatheringSM

 Parameters
     None

 Returns
     GatheringState_t The current state of the Gathering state machine

 Description
     returns the current state of the Gathering state machine
 Notes

 Author
     J. Edward Carryer, 10/23/11, 19:21
****************************************************************************/
GatheringState_t QueryGatheringSM ( void )
{
   return(CurrentState);
}

/****************************************************************************
Function
     	StartGatheringSM

Parameters
     	CurrentEvent

Returns
		None
		
Description
     	returns the current state of the Gathering state machine
Notes

Author
     J. Edward Carryer, 10/23/11, 19:21
****************************************************************************/
void StartGatheringSM ( ES_Event CurrentEvent )
{
	// Create a local variable to allow debugger to display CurrentEvent
	ES_Event LocalEvent = CurrentEvent;
	
	CurrentState = FullSpeedAhead;
		
	printf("\r\nStarting Gathering SM.");
	RunGatheringSM(LocalEvent);
}
/***************************************************************************
private functions
***************************************************************************/
static ES_Event DuringFSA(ES_Event ThisEvent)
{
	// Process ES_ENTRY event
	if (ThisEvent.EventType == ES_ENTRY)
	{
		printf("\r\nENTERING the FullSpeedAhead state.");
		// Drive forward at 100% speed
		GoForward(100);
	}
	else if (ThisEvent.EventType == ES_EXIT)
	{
		printf("\r\nEXITING the FullSpeedAhead state.");
		// Stop the robot
		FullStop();
	}
	else 
	{
    	// No during function
	}
	return ThisEvent; // do not remap event
}

static ES_Event DuringHSA(ES_Event ThisEvent)
{
	// Process ES_ENTRY event
	if (ThisEvent.EventType == ES_ENTRY)
	{
		printf("\r\nENTERING the HalfSpeedAhead state.");
		// Drive forward caution speed
		GoForward(CAUTION_SPEED);
		// Set MOTION_TIMER to count slow "caution" speed time
		ES_Timer_StopTimer(MOTION_TIMER);
		ES_Timer_SetTimer(MOTION_TIMER, CAUTIONSPEED_INTERVAL);
		ES_Timer_StartTimer(MOTION_TIMER);
	}
	else if (ThisEvent.EventType == ES_EXIT)
	{
		printf("\r\nEXITING the HalfSpeedAhead state.");
		// Stop the robot
		FullStop();
	}
	else 
	{
    	// No during function
	}
	return ThisEvent; // do not remap event
}

static ES_Event DuringTurningLeft(ES_Event ThisEvent)
{
	// Process ES_ENTRY event
	if (ThisEvent.EventType == ES_ENTRY)
	{
		printf("\r\nENTERING the TurningLeft state.");
		// Begin a 90 degree turn to the left
		TurnLeft();
		// Set MOTION_TIMER to count turn time
		ES_Timer_StopTimer(MOTION_TIMER);
		ES_Timer_SetTimer(MOTION_TIMER, TURN_EVADE_INTERVAL);
		ES_Timer_StartTimer(MOTION_TIMER);
	}
	else if (ThisEvent.EventType == ES_EXIT)
	{
		printf("\r\nEXITING the TurningLeft state.");
		// Stop the robot
		FullStop();
	}	
	else 
	{
    	// No during function
	}
	return ThisEvent; // do not remap event
}

static ES_Event DuringTurningRight(ES_Event ThisEvent)
{
	// Process ES_ENTRY event
	if (ThisEvent.EventType == ES_ENTRY)
	{
		printf("\r\nENTERING the TurningRight state.");
		// Begin a 90 degree turn to the right
		TurnRight();
		// Set MOTION_TIMER to count turn time
		ES_Timer_StopTimer(MOTION_TIMER);
		ES_Timer_SetTimer(MOTION_TIMER, TURN_EVADE_INTERVAL);
		ES_Timer_StartTimer(MOTION_TIMER);
	}
	else if (ThisEvent.EventType == ES_EXIT)
	{
		printf("\r\nEXITINGthe TurningRight state.");
		// Stop the robot
		FullStop();
	}	
	else 
	{
    	// No during function
	}
	return ThisEvent; // do not remap event
}

static ES_Event DuringFullReverse(ES_Event ThisEvent)
{
	printf("\r\nENTERING the FullReverse state.");
	// Process ES_ENTRY event
	if (ThisEvent.EventType == ES_ENTRY)
	{
		// Drive backward at 100% speed
		GoBackward(100);
		// Set MOTION_TIMER to count how long to back up
		ES_Timer_StopTimer(MOTION_TIMER);
		ES_Timer_SetTimer(MOTION_TIMER, BACKUP_INTERVAL);
		ES_Timer_StartTimer(MOTION_TIMER);
		
		#ifdef DEBUG_WALL_PUSHING
		// Set WALL_BUMP_TIMER to count how long to back up
		ES_Timer_StopTimer(WALL_BUMP_TIMER);
		ES_Timer_SetTimer(WALL_BUMP_TIMER, WALL_BUMP_INTERVAL);
		ES_Timer_StartTimer(WALL_BUMP_TIMER);
		#endif
	}
	else if (ThisEvent.EventType == ES_EXIT)
	{
		printf("\r\nEXITING the FullReverse state.");
		// Stop the robot
		FullStop();
	}	
	else 
	{
    	// No during function
	}
	return ThisEvent; // do not remap event
}

static ES_Event DuringHalfReverse(ES_Event ThisEvent)
{
	printf("\r\nENTERING the HalfReverse state.");
	// Process ES_ENTRY event
	if (ThisEvent.EventType == ES_ENTRY)
	{
		// Drive backward at caution speed
		GoBackward(CAUTION_SPEED);
		// Set MOTION_TIMER to count how long to back up
		ES_Timer_StopTimer(MOTION_TIMER);
		ES_Timer_SetTimer(MOTION_TIMER, BACKUP_INTERVAL);
		ES_Timer_StartTimer(MOTION_TIMER);
	}
	else if (ThisEvent.EventType == ES_EXIT)
	{
		printf("\r\nEXITING the HalfReverse state.");
		// Stop the robot
		FullStop();
	}	
	else 
	{
    	// No during function
	}
	return ThisEvent; // do not remap event
}

static TurnDirection_t QueryNextTurnDirection(void)
{
   // This function tells you the next turn direction
   const unsigned char MaxTurnTableSize = 4;
   const TurnDirection_t TurnDirectionTable[4] = {Right, Right, Right, Right};
   static unsigned char TurnDirectionCounter = 0;
   TurnDirection_t TurnDirection;
   
   TurnDirection = TurnDirectionTable[TurnDirectionCounter]; // assign next turn direction
   if((TurnDirectionCounter == 0) || (TurnDirectionCounter == 1))
   {
      printf("\r\nCurrent turn direction is LEFT.");
   }
   else
   {
      printf("\r\nCurrent turn direction is RIGHT.");
   }
  
   
   TurnDirectionCounter++; // increment table counter
   
   // Check for index overflow
   TurnDirectionCounter = TurnDirectionCounter%MaxTurnTableSize;
   
   return TurnDirection;
}