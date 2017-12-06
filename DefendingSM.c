/****************************************************************************
 Module
   DefendingSM.c

 Revision
   1.0.1

 Description
   This is a template file for implementing flat state machines under the 
   Gen2 Events and Services Framework.

 Notes

 History
 When           Who     What/Why
 -------------- ---     --------
 02/28/12 19:37 adl     Began tailoring of template to be our Defending SM
 01/15/12 11:12 jec      revisions for Gen2 framework
 11/07/11 11:26 jec      made the queue static
 10/30/11 17:59 jec      fixed references to CurrentEvent in RunTemplateSM()
 10/23/11 18:20 jec      began conversion from SMTemplate.c (02/20/07 rev)
****************************************************************************/
/*----------------------------- Include Files -----------------------------*/
/* include header files for this state machine as well as any machines at the
   next lower level in the hierarchy that are sub-machines to this machine
*/
#include <stdio.h>
#include <mc9s12e128.h>     /* derivative information */
#include <S12e128bits.h>    /* bit definitions  */

#include "ES_Configure.h"
#include "ES_Framework.h"
#include "ES_PostList.h"

// Includes for statemachines
#include "DefendingSM.h"

// Includes for modular code
#include "TimingConstants.h"
#include "SideID.h"
#include "MotorDriver.h"
#include "QuickSense.h"
#include "FSR.h"
#include "BeaconDetection.h"
#include "BinControl.h"
#include "DefendingMode.h"
#include "ScoringSM.h" // for querying the target bin

#include "EventPrinter.h"

/*----------------------------- Module Defines ----------------------------*/


/*---------------------------- Module Functions ---------------------------*/
/* prototypes for private functions for this machine.They should be functions
   relevant to the behavior of this state machine
*/
static ES_Event DuringDrivingAwayFromWall(ES_Event);
static ES_Event DuringAligningPerpendicular(ES_Event);
static ES_Event DuringWaiting(ES_Event);
static ES_Event DuringReseting(ES_Event);
static ES_Event DuringPushingForward(ES_Event);
static ES_Event DuringPushingBackward(ES_Event);
static ES_Event DuringRealigning(ES_Event);

 
/*---------------------------- Module Variables ---------------------------*/
// everybody needs a state variable, you may need others as well.
// type of state variable should match htat of enum in header file
static DefendingState_t CurrentState;
static unsigned char TargetBin;
static TurnDirection_t TurnDirection = Left; // the current turn direction

/*------------------------------ Module Code ------------------------------*/
/****************************************************************************
 Function
     QueryDefendingSM

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
DefendingState_t QueryDefendingSM ( void )
{
   return(CurrentState);
}

/****************************************************************************
Function
     	StartDefendingSM

Parameters
     	CurrentEvent

Returns
		None
		
Description
     	Starts the scoring state machine
Notes

Author
     J. Edward Carryer, 10/23/11, 19:21
****************************************************************************/
void StartDefendingSM ( ES_Event CurrentEvent )
{
   // Create a local variable to allow the debugger to display CurrentEvent
   ES_Event LocalEvent = CurrentEvent;
   
   CurrentState = DrivingAwayFromWall;
   TargetBin = QueryTargetBin(); // determine the bin that we scored on
   
   // Initialize the wall helper module
   DefendingMode_Init();   
   
   printf("\r\nStarting Defending SM.");
   RunDefendingSM(LocalEvent);
}

/****************************************************************************
 Function
    RunDefendingSM

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
ES_Event RunDefendingSM(ES_Event ThisEvent)
{
   boolean MakeTransition = False; // are we making a state transition
   DefendingState_t NextState = CurrentState;
   ES_Event ReturnEvent = ThisEvent; // assume we are not consuming the event
   
   EventPrinter(ThisEvent);
   // Switch on the states of the Defending SM
   switch (CurrentState)
   {
      case DrivingAwayFromWall:
         printf("\r\nIn the DrivingAwayFromWall state.");
         // Execute the during function for this state
         // Entry and exit functions are processed here
         ThisEvent = DuringDrivingAwayFromWall(ThisEvent);
         
         //printf("\r\nDone running DuringDrivingAwayFromWall.");
         // Process events
         // Check if there is an event to respond to
         if (ThisEvent.EventType != ES_NO_EVENT)
         {
            switch(ThisEvent.EventType)
            {
               case ES_TIMEOUT:
                  if (ThisEvent.EventParam == MOTION_TIMER)
                  {
                     printf("\r\nMOTION_TIMER expired, move to Waiting state.");
                     NextState = Waiting; // set next state
                     MakeTransition = True; // mark that we are making a transition
                     ReturnEvent.EventType = ES_NO_EVENT; // consume event
                  }
               break;
            } // End event type switch
         } // End guard against no event
      break; // End during DrivingAwayFromWall
      
      case AligningPerpendicular:
         printf("\r\nIn the AligningPerpendicular state.");
         // Execute the during function for this state
         // Entry and exit functions are processed here
         ThisEvent = DuringAligningPerpendicular(ThisEvent);
         
         // Process events
         // Check if there is an event to respond to
         if (ThisEvent.EventType != ES_NO_EVENT)
         {
            switch(ThisEvent.EventType)
            {
               case ES_TIMEOUT:
                  if (ThisEvent.EventParam == MOTION_TIMER)
                  {
                     printf("\r\nMOTION_TIMER expired while aligning perpendicular, go to PushingForward.");
                     NextState = PushingForward; // set next state
                     MakeTransition = True; // mark that we are making a transition
                     ReturnEvent.EventType = ES_NO_EVENT; // consume event
                  }
               break;
            } // End event type switch
         } // End guard against no event
      break;
      
         
      case Waiting:
         printf("\r\nIn the Waiting state.");
         // Execute the during function for this state
         // Entry and exit functions are processed here
         ThisEvent = DuringWaiting(ThisEvent);
         
         // Process events
         // Check if there is an event to respond to
         if (ThisEvent.EventType != ES_NO_EVENT)
         {
            switch(ThisEvent.EventType)
            {
               case ES_DANGERWALL_RIGHT:
                  printf("\r\nRight wall encroaching, go to AligningPerpendicular.");
                  NextState = AligningPerpendicular; // set next state
                  MakeTransition = True; // mark that we are making a transition
                  ReturnEvent.EventType = ES_NO_EVENT; // consume event
                  TurnDirection = Right; // set the turn direction
               break;
               
               case ES_DANGERWALL_LEFT:
                  printf("\r\nLeft wall encroaching, go to AligningPerpendicular");
                  NextState = AligningPerpendicular; // set next state
                  MakeTransition = True; // mark that we are making a transition
                  ReturnEvent.EventType = ES_NO_EVENT; // consume event
                  TurnDirection = Left; // set the turn direction     
               break;
            } // End event type switch
         } // End guard against no event
      break;
      
      case Reseting:
         printf("\r\nIn the Reseting state.");
         // Execute the during function for this state
         // Entry and exit functions are processed here
         ThisEvent = DuringReseting(ThisEvent);
         
         // Process events
         // Check if there is an event to respond to
         if (ThisEvent.EventType != ES_NO_EVENT)
         {
            switch(ThisEvent.EventType)
            {
               case ES_DANGERWALL_RIGHT:
                  if (TurnDirection == Right)
                  {
                     printf("\r\nWall moving from right again while reseting. Go to PushingForward.");
                     NextState = PushingForward; // set next state
                     MakeTransition = True; // mark that we are making a transition
                     ReturnEvent.EventType = ES_NO_EVENT; // consume event   
                  }
                  else
                  {
                     printf("\r\nWall moving from left while reseting. Go to PushingBackward.");
                     NextState = PushingBackward; // set next state
                     MakeTransition = True; // mark that we are making a transition
                     ReturnEvent.EventType = ES_NO_EVENT; // consume event 
                  }
                   break;
                   
               case ES_DANGERWALL_LEFT:
                  if (TurnDirection == Left)
                  {
                     printf("\r\nWall moving from left again while reseting. Go to PushingForward.");
                     NextState = PushingForward; // set next state
                     MakeTransition = True; // mark that we are making a transition
                     ReturnEvent.EventType = ES_NO_EVENT; // consume event 
                  }
                  else
                  {
                     printf("\r\nWall moving from right while reseting. Go to PushingBackward.");
                     NextState = PushingBackward; // set next state
                     MakeTransition = True; // mark that we are making a transition
                     ReturnEvent.EventType = ES_NO_EVENT; // consume event 
                  }
                  break;
               
               case ES_LEFT_TAPE_DETECTED:
                  printf("\r\nTape detected while reseting. Go to Realigning.");
                  NextState = Realigning; // set next state
                  MakeTransition = True; // mark that we are making a transition
                  ReturnEvent.EventType = ES_NO_EVENT; // consume event      
               break;
               
               case ES_RIGHT_TAPE_DETECTED:
                  printf("\r\nTape detected while reseting. Go to Realigning.");
                  NextState = Realigning; // set next state
                  MakeTransition = True; // mark that we are making a transition
                  ReturnEvent.EventType = ES_NO_EVENT; // consume event      
               break;
               
               case ES_TIMEOUT:
                  if (ThisEvent.EventParam == MOTION_TIMER)
                  {
                     printf("\r\nTimer expired while reseting. Go to Realigning.");
                     NextState = Realigning; // set next state
                     MakeTransition = True; // mark that we are making a transition
                     ReturnEvent.EventType = ES_NO_EVENT; // consume event    
                  }
               break;
               
            } // End event type switch
         } // End guard against no event
      break;
      
      case PushingForward:
         printf("\r\nIn the PushingForward state.");
         // Execute the during function for this state
         // Entry and exit functions are processed here
         ThisEvent = DuringPushingForward(ThisEvent);
         
         // Process events
         // Check if there is an event to respond to
         if (ThisEvent.EventType != ES_NO_EVENT)
         {
            switch(ThisEvent.EventType)
            {
               
               case ES_NO_DANGERWALL:
                  printf("\r\nThe wall has moved into the safe zone. Reset position.");
                  NextState = Reseting; // set next state
                  MakeTransition = True; // mark that we are making a transition
                  ReturnEvent.EventType = ES_NO_EVENT; // consume event      
               break;
            } // End event type switch
         } // End guard against no event   
      break;
      
      case PushingBackward:
         printf("\r\nIn the PushingBackward state.");
         // Execute the during function for this state
         // Entry and exit functions are processed here
         ThisEvent = DuringPushingBackward(ThisEvent);
         
         // Process events
         // Check if there is an event to respond to
         if (ThisEvent.EventType != ES_NO_EVENT)
         {
            switch(ThisEvent.EventType)
            {
               case ES_NO_DANGERWALL:
                  printf("\r\nThe wall has moved into the safe zone. Reset position.");
                  NextState = Reseting; // set next state
                  MakeTransition = True; // mark that we are making a transition
                  ReturnEvent.EventType = ES_NO_EVENT; // consume event      
               break;
            } // End event type switch
         } // End guard against no event
      break;
      
      case Realigning:
         printf("\r\nIn the Realigning state.");
         // Execute the during function for this state
         // Entry and exit functions are processed here
         ThisEvent = DuringRealigning(ThisEvent);
         
         // Process events
         // Check if there is an event to respond to
         if (ThisEvent.EventType != ES_NO_EVENT)
         {
            switch(ThisEvent.EventType)
            {
               case ES_BEACON_REAR:
                  if(ThisEvent.EventParam == TargetBin)
                  {
                     // We have found our target bin and are aligned
                     printf("\r\nRear beacon is aligned with the target bin.");
                     NextState = Waiting; // set next state
                     MakeTransition = True; // mark that we are making a transition
                     ReturnEvent.EventType = ES_NO_EVENT; // consume event
                  }                        
               break;
            } // End event type switch
         } // End guard against no event
      break;
      
   } // End switch on current state
   
   
   // Check if we are making a state transition
   if (MakeTransition == True)
   {
      // Execute the exit function for the current state
      ThisEvent.EventType = ES_EXIT;
      RunDefendingSM(ThisEvent);
      
      CurrentState = NextState; // Update state variable
      
      // Execute the entry function for the new state
      ThisEvent.EventType = ES_ENTRY;
      RunDefendingSM(ThisEvent);
   }
   //printf("\r\nReturning from RunDefendingSM.");
   return ReturnEvent;
}

/***************************************************************************
private functions
***************************************************************************/
static ES_Event DuringDrivingAwayFromWall(ES_Event ThisEvent)
{
   if(ThisEvent.EventType == ES_ENTRY)
   {
      // Process ES_ENTRY
      printf("\r\nENTERING the DrivingAwayFromWall state.");
      GoForward(100);
      
      // Set timer for wall separation
      ES_Timer_StopTimer(MOTION_TIMER);
	   ES_Timer_SetTimer(MOTION_TIMER, WALL_SEPARATION_INTERVAL);
	   ES_Timer_StartTimer(MOTION_TIMER);
	   //printf("\r\nThe wall separation timer has been set.");
   }
   else if (ThisEvent.EventType == ES_EXIT)
   {
      // Process ES_EXIT
      printf("\r\nEXITING the DrivingAwayFromWall state.");
      FullStop(); // stop the bot
   }
   else
   {
      // Perform during functions
      // No during functions for this state
   }
   //printf("\r\nReturning from DuringDrivingAwayFromWall.");
   return ThisEvent;
}

static ES_Event DuringAligningPerpendicular(ES_Event ThisEvent)
{
   if(ThisEvent.EventType == ES_ENTRY)
   {
      // Process ES_ENTRY
      printf("\r\nENTERING the AligningPerpendicular state.");
      
      // Turn to face the oncoming wall
      switch (TurnDirection)
      {
         case Left:
            TurnLeft();
         break;        
         
         case Right:
            TurnRight();
         break;
      }
      
      // Set timer for turning 90 degrees
      ES_Timer_StopTimer(MOTION_TIMER);
      ES_Timer_SetTimer(MOTION_TIMER, DEGREE90_INTERVAL);
      ES_Timer_StartTimer(MOTION_TIMER);
   }
   else if (ThisEvent.EventType == ES_EXIT)
   {
      // Process ES_EXIT
      printf("\r\nEXITING the AligningPerpendicular state.");
      FullStop(); // stop the bot
   }
   else
   {
      // Perform during functions
      // No during functions for this state
   }
   return ThisEvent;   
}

static ES_Event DuringWaiting(ES_Event ThisEvent)
{
   if(ThisEvent.EventType == ES_ENTRY)
   {
      // Process ES_ENTRY
      printf("\r\nENTERING the Waiting state.");
      FullStop(); // make sure that the bot is stopped
   }
   else if (ThisEvent.EventType == ES_EXIT)
   {
      // Process ES_EXIT
      printf("\r\nEXITING the Waiting state.");
   }
   else
   {
      // Perform during functions
      // No during functions for this state
   }
   return ThisEvent;   
}

static ES_Event DuringReseting(ES_Event ThisEvent)
{
   if(ThisEvent.EventType == ES_ENTRY)
   {
      // Process ES_ENTRY
      printf("\r\nENTERING the Reseting state.");
      // Set the reset clock
      ES_Timer_StopTimer(MOTION_TIMER);
      ES_Timer_SetTimer(MOTION_TIMER, RESET_INTERVAL);
      ES_Timer_StartTimer(MOTION_TIMER);
   }
   else if (ThisEvent.EventType == ES_EXIT)
   {
      // Process ES_EXIT
      printf("\r\nEXITING the Reseting state.");
      FullStop(); // stop the bot
   }
   else
   {
      // Perform during functions
      // No during functions for this state
   }
   return ThisEvent;   
}

static ES_Event DuringPushingForward(ES_Event ThisEvent)
{
   if(ThisEvent.EventType == ES_ENTRY)
   {
      // Process ES_ENTRY
      printf("\r\nENTERING the PushingForward state.");
      GoForward(100); // drive the bot forward to push the wall
   }
   else if (ThisEvent.EventType == ES_EXIT)
   {
      // Process ES_EXIT
      printf("\r\nEXITING the PushingForward state.");
      GoBackward(100); // go back toward the cetner line of the bin
   }
   else
   {
      // Perform during functions
      // No during functions for this state
   }
   return ThisEvent;   
}

static ES_Event DuringPushingBackward(ES_Event ThisEvent)
{
   if(ThisEvent.EventType == ES_ENTRY)
   {
      // Process ES_ENTRY
      printf("\r\nENTERING the PushingBackward state.");
      GoBackward(100); // drive the bot backward to push the wall
   }
   else if (ThisEvent.EventType == ES_EXIT)
   {
      // Process ES_EXIT
      printf("\r\nEXITING the PushingBackward state.");
      GoForward(100); // go back toward the cetner line of the bin
   }
   else
   {
      // Perform during functions
      // No during functions for this state
   }
   return ThisEvent;   
} 


static ES_Event DuringRealigning(ES_Event ThisEvent)
{
   if(ThisEvent.EventType == ES_ENTRY)
   {
      // Process ES_ENTRY
      printf("\r\nENTERING the Realigning state.");
      
      // Turn in the opposite direction of the original turn while looking for bin
      switch (TurnDirection)
      {
         case Left:
            TurnRightSpeedSelect(BEACON_SEARCH_TURN_SPEED); // start turning the bot to find the beacon
         break;
         
         case Right:
            TurnLeftSpeedSelect(BEACON_SEARCH_TURN_SPEED); // start turning the bot to find the beacon
         break;
      }
   }
   else if (ThisEvent.EventType == ES_EXIT)
   {
      // Process ES_EXIT
      printf("\r\nEXITING the Realigning state.");
      FullStop(); // stop the bot
   }
   else
   {
      // Perform during functions
      // No during functions for this state
   }
   return ThisEvent;   
}