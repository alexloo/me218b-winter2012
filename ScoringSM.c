/****************************************************************************
 Module
   ScoringSM.c

 Revision
   1.0.1

 Description
   This is a template file for implementing flat state machines under the 
   Gen2 Events and Services Framework.

 Notes

 History
 When           Who     What/Why
 -------------- ---     --------
 02/26/12 12:58 adl     Tailoring to be a scoring mode FSM
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
#include "ScoringSM.h"

// Module Headers
#include "MotorDriver.h"
#include "TimingConstants.h"
#include "FSR.h"
#include "SideID.h"
#include "BinControl.h"
#include "ScoringMode.h" // helper functions for use during scoring mode
#include "BeaconDetection.h"

#include <stdio.h>
#include "EventPrinter.h"

/*----------------------------- Module Defines ----------------------------*/
#define MAX_APPROACH_PASSES 2

#define BEACON_NOD
#define BACKUP_SEARCH

/*---------------------------- Module Functions ---------------------------*/
/* prototypes for private functions for this machine.They should be functions
   relevant to the behavior of this state machine
*/
static ES_Event DuringAligningRearBeacon(ES_Event);
static ES_Event DuringAligningFrontBeacon(ES_Event);
static ES_Event DuringDrivingForward_Clearance(ES_Event);
static ES_Event DuringDrivingForward_Alignment(ES_Event);
static ES_Event DuringBackingUp(ES_Event);
static ES_Event DuringUnloading(ES_Event);
static ES_Event DuringShuffling(ES_Event);
static ES_Event DuringFindingLeftBeacon(ES_Event ThisEvent);
static ES_Event DuringFindingRightBeacon(ES_Event ThisEvent);
static ES_Event DuringBisectingAngle(ES_Event ThisEvent);

/*---------------------------- Module Variables ---------------------------*/
// everybody needs a state variable, you may need others as well.
// type of state variable should match htat of enum in header file
static ScoringState_t CurrentState;

// Define variables which determine which bin is the target
static unsigned char TargetBin = 2;
static unsigned char OppositeBin = 3;
static unsigned char RightBin = 0;
static unsigned char LeftBin = 0;

// Variables for storing time of acquisition for the two side beacons
static uint16_t TOSA_Left = 0;
static uint16_t TOSA_Right = 0;

static unsigned char ApproachPass = 0;
static TurnDirection_t ShuffleDirection = Left;

/*------------------------------ Module Code ------------------------------*/
/****************************************************************************
 Function
     QueryScoringSM

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
ScoringState_t QueryScoringSM ( void )
{
   return(CurrentState);
}

/****************************************************************************
Function
     	StartScoringSM

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
void StartScoringSM ( ES_Event CurrentEvent )
{
	// Create a local variable to allow debugger to display CurrentEvent
	ES_Event LocalEvent = CurrentEvent;
	
	printf("\r\nRunning StartScoringSM.");
	CurrentState = AligningRearBeacon;
	// Run entry function for scoring state machine
	// Determine which bin to score on
	printf("\r\nAbout to determine target bin.");
   TargetBin = PickScoringBin();
   printf("\r\nTarget bin determined.");
   
   
   /*printf("\r\nBalls in Bin 1: %u", Get_BallsInBin(1));
   printf("\r\nBalls in Bin 2: %u", Get_BallsInBin(2));
   printf("\r\nBalls in Bin 3: %u", Get_BallsInBin(3));
   printf("\r\nBalls in Bin 4: %u", Get_BallsInBin(4));*/

   printf("\r\nTarget Bin is: %u", TargetBin);
	
	
	// Determine opposite bin from target bin
	switch(TargetBin)
	{
	   case 1:
	      OppositeBin = 3;
	      RightBin = 4;
	      LeftBin = 2;
	   break;
	   
	   case 2:
	      OppositeBin = 4;
	      RightBin = 1;
	      LeftBin = 3;
	   break;
	   
	   case 3:
	      OppositeBin = 1;
	      RightBin = 2;
	      LeftBin = 4;
	   break;
	   
	   case 4:
	      OppositeBin = 2;
	      RightBin = 3;
	      LeftBin = 1;
	   break;
	}
	printf("\r\nStarting Scoring SM.");
	RunScoringSM(LocalEvent);	
}

/****************************************************************************
 Function
    RunScoringSM

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
ES_Event RunScoringSM(ES_Event ThisEvent)
{
   boolean MakeTransition = False; // are we making a state transition?
  	ScoringState_t NextState = CurrentState;
  	ES_Event ReturnEvent = ThisEvent; // Assume we are not consuming event
  	  	
  	EventPrinter(ThisEvent);
  	switch (CurrentState)
  	{
  	   case AligningRearBeacon:
     	   //printf("\r\nIn the AligningRearBeacon state.");
     	   // Execute the during function for aligning rear beacon
     	   // Entry and exit functions are processed here
     	   ThisEvent = DuringAligningRearBeacon(ThisEvent);
     	   
     	   // Process events
     	   // Check if there is an event to respond to
     	   if (ThisEvent.EventType != ES_NO_EVENT)
     	   {
     	      // Switch on event types
     	      switch(ThisEvent.EventType)
     	      {
     	         case ES_BEACON_REAR:
        	         if (ThisEvent.EventParam == TargetBin)
        	         {
        	            //printf("\r\nFound the beacon on the rear. Going to BackingUp.");
        	            // We have the rear aligned with where we want to score
        	            NextState = BackingUp; // determine what the next state will be
        	            MakeTransition = True; // mark that we are making a transition
        	            ReturnEvent.EventType = ES_NO_EVENT; // consume the event 
        	         }
     	         break; // end rear beacon event
     	         
     	         case ES_TIMEOUT:
     	            if (ThisEvent.EventParam == BEACON_NOD_TIMER)
     	            {
     	               // Switch directions, this was to limit full circles while searching
     	               TurnLeftSpeedSelect(BEACON_SEARCH_TURN_SPEED);  
     	            }     	            
     	         break;
     	         
     	      } // End event type switch 
     	   } // End guard against no event
  	   break; // End AligningRearBeaconState
  	   
  	   case AligningFrontBeacon:
     	   printf("\r\nIn the AligningFrontBeacon state.");
     	   // Execute the during function for aligning rear beacon
     	   // Entry and exit functions are processed here
     	   ThisEvent = DuringAligningFrontBeacon(ThisEvent);
     	   
     	   // Process events
     	   // Check if there is an event to respond to
     	   if (ThisEvent.EventType != ES_NO_EVENT)
     	   {
     	      // Switch on event types
     	      switch(ThisEvent.EventType)
     	      {
     	         case ES_BEACON_FRONT:
     	            if (ThisEvent.EventParam == OppositeBin)
     	            {
     	               //printf("\r\nFound the beacon on the front. Going to DrivingForward_Alignment.");
     	               // We have the rear aligned with the bin across the field
     	               NextState = DrivingForward_Alignment;// determine what the next state will be
     	               MakeTransition = True; // mark that we are making a transition
     	               ReturnEvent.EventType = ES_NO_EVENT; // consume the event
     	               
     	               // Set the timer length based on which pass this is
     	               if (ApproachPass == (MAX_APPROACH_PASSES-2))
     	               {
     	                  // This is the first pass
     	                  ES_Timer_StopTimer(MOTION_TIMER);
     	                  ES_Timer_SetTimer(MOTION_TIMER, FIRST_FWD_ALIGN_INTERVAL);   
     	               }
     	               else if (ApproachPass == (MAX_APPROACH_PASSES-1))
     	               {
     	                  // This is the second pass
     	                  ES_Timer_StopTimer(MOTION_TIMER);
     	                  ES_Timer_SetTimer(MOTION_TIMER, SECOND_FWD_ALIGN_INTERVAL);
     	               }
     	               ES_Timer_StartTimer(MOTION_TIMER); // Start timer
     	            }
     	         break;
     	         
     	         case ES_TIMEOUT:
     	            if (ThisEvent.EventParam == BEACON_NOD_TIMER)
     	            {
     	               // Switch directions, this was to limit full circles while searching
     	               TurnLeftSpeedSelect(BEACON_SEARCH_TURN_SPEED);  
     	            }
     	            
     	            else if (ThisEvent.EventParam == GO_TO_BACKUP_SEARCH_TIMER)
     	            {
     	               // Could not find the beacon, go to backup alignment method
     	               printf("\r\nCould not find the opposite beacon. Go to bisecting.");
     	               NextState = FindingLeftBeacon; // go to the state where we're looking for left beacon
     	               MakeTransition = True; // mark that we are making a transition
     	               ReturnEvent.EventType = ES_NO_EVENT; // consume the event
     	            }     	            
     	         break;
     	         
     	      } // End event type switch 
     	   } // End guard against no event
  	   break; // End AligningFrontBeaconState
  	   
  	   case DrivingForward_Clearance:
     	   printf("\r\nIn the DrivingForward_Clearance state.");
     	   // Execute the during function for aligning rear beacon
     	   // Entry and exit functions are processed here
     	   ThisEvent = DuringDrivingForward_Clearance(ThisEvent);
     	   
     	   // Process events
     	   // Check if there is an event to respond to
     	   if (ThisEvent.EventType != ES_NO_EVENT)
     	   {
     	      // Switch on event types
     	      switch(ThisEvent.EventType)
     	      {
     	         case ES_TIMEOUT:
     	            if (ThisEvent.EventParam == MOTION_TIMER)
     	            {
     	               printf("\r\nEnough clearance has been achieved. Looking for front beacon.");
     	               // Sufficienct clearance has been achieved
     	               NextState = AligningFrontBeacon;// determine what the next state will be
     	               MakeTransition = True; // mark that we are making a transition
     	               ReturnEvent.EventType = ES_NO_EVENT; // consume the event 
     	            }
     	         break;
     	         
     	      } // End event type switch 
     	   } // End guard against no event
  	   break; // End DrivingForward100 State
  	   
  	   case DrivingForward_Alignment:
     	   printf("\r\nIn the DrivingForward_Alignment state.");
     	   // Execute the during function for aligning rear beacon
     	   // Entry and exit functions are processed here
     	   ThisEvent = DuringDrivingForward_Alignment(ThisEvent);
     	   
     	   // Process events
     	   // Check if there is an event to respond to
     	   if (ThisEvent.EventType != ES_NO_EVENT)
     	   {
     	      // Switch on event types
     	      switch(ThisEvent.EventType)
     	      {       	         
     	         case ES_TIMEOUT:
                  if (ThisEvent.EventParam == MOTION_TIMER)
                  {
                     printf("\r\nAlignment period ended. Look for rear beacon.");
                     // Alignment driving interval has been reached
                     NextState = AligningRearBeacon;// determine what the next state will be
  	                  MakeTransition = True; // mark that we are making a transition
  	                  ReturnEvent.EventType = ES_NO_EVENT; // consume the event  
                  } 	         
     	         break;
     	         
     	         case ES_FRONT_BUMPED:
     	            printf("\r\nHit something in front while aligning. Look for rear beacon.");
     	            // We hit something as we were driving forward
     	            NextState = AligningRearBeacon;// determine what the next state will be
  	               MakeTransition = True; // mark that we are making a transition
  	               ReturnEvent.EventType = ES_NO_EVENT; // consume the event
     	         break;
     	         
     	         /*
     	         case ES_BEACON_FRONT:
     	            if (ThisEvent.EventParam == 0)
     	            {
     	               printf("\r\nWe lost the front beacon.");
     	               // We lost the beacon, let's look for it again
     	               NextState = AligningFrontBeacon; // determine what the next state will be
     	               MakeTransition = True; // mark that we are making a transition
     	               ReturnEvent.EventType = ES_NO_EVENT; // consume event
     	               // decrement the alignment pass counter since we're going back into aligning state
     	               ApproachPass--; 
     	            }
     	         break;
     	         */
     	      } // End event type switch 
     	   } // End guard against no event
  	   break; // End DrivingForward_Alignment State
  	   
  	   case BackingUp:
     	   printf("\r\nIn the BackingUp state.");
     	   // Execute the during function for aligning rear beacon
     	   // Entry and exit functions are processed here
     	   ThisEvent = DuringBackingUp(ThisEvent);
     	   
     	   // Process events
     	   // Check if there is an event to respond to
     	   if (ThisEvent.EventType != ES_NO_EVENT)
     	   {
     	      // Switch on event types
     	      switch(ThisEvent.EventType)
     	      {
     	         case ES_LEFT_TAPE_DETECTED:
     	            printf("\r\nFound tape while heading to target bin, slow down.");
     	            // Tape detected, take caution
     	            GoBackward(CAUTION_SPEED); // set backward speed to caution speed
     	            ES_Timer_StopTimer(MOTION_TIMER);
     	            ES_Timer_SetTimer(MOTION_TIMER, CAUTION_INTERVAL);
     	            ES_Timer_StartTimer(MOTION_TIMER);
     	            ReturnEvent.EventType = ES_NO_EVENT; // consume the event
     	         break;
     	         
     	         case ES_RIGHT_TAPE_DETECTED:
     	            printf("\r\nFound tape while heading to target bin, slow down.");
     	            // Tape detected, take caution
     	            GoBackward(CAUTION_SPEED); // set backward speed caution speed
     	            ES_Timer_StopTimer(MOTION_TIMER);
     	            ES_Timer_SetTimer(MOTION_TIMER, CAUTION_INTERVAL);
     	            ES_Timer_StartTimer(MOTION_TIMER);
     	            ReturnEvent.EventType = ES_NO_EVENT; // consume the event
     	         break;
     	         
     	         case ES_TIMEOUT:
     	            if(ThisEvent.EventParam == MOTION_TIMER)
     	            {
     	               printf("\r\nCaution period expired. Go at 100");
     	               // The caution period expired without hitting a wall
     	               // Let's go ludicrous speed
     	               GoBackward(100);
     	               ReturnEvent.EventType = ES_NO_EVENT; // consume the event
     	            }
     	         break;
     	         
     	         case ES_REAR_BUMPED:
     	            // The rear bumper was hit, determine if making another pass or parking
     	            if (ApproachPass == MAX_APPROACH_PASSES)
     	            {
     	               printf("\r\nRear bumper hit on last approach. Dump balls.");
     	               // Approach sequence complete, go park
     	               NextState = Unloading;// determine what the next state will be
  	                  MakeTransition = True; // mark that we are making a transition
  	                  ReturnEvent.EventType = ES_NO_EVENT; // consume the event
  	                  FanControl(0); // turn off fan
     	            }
     	            else
     	            {
     	               // Make another pass if we don't see the opposite beacon on the front
     	               if (GetBeaconFront() != OppositeBin)
     	               {
        	               printf("\r\nRear bumper hit on preliminary approach. Make another pass.");
        	               // Approach sequence is not complete, make another pass
        	               NextState = DrivingForward_Clearance;// determine what the next state will be
     	                  MakeTransition = True; // mark that we are making a transition
     	                  ReturnEvent.EventType = ES_NO_EVENT; // consume the event
     	               }
     	               else
     	               {
     	                  // We see the opposite bin in our front beacon, we are aligned
     	                  printf("\r\nOpposite beacon seen in front on preliminary approach. Dump balls.");
        	               // Approach sequence complete, go park
        	               NextState = Unloading;// determine what the next state will be
     	                  MakeTransition = True; // mark that we are making a transition
     	                  ReturnEvent.EventType = ES_NO_EVENT; // consume the event
     	                  FanControl(0); // turn off fan
     	               }
     	            }
     	         break;
     	         
     	         /*
     	         case ES_BEACON_REAR:
     	            if (ThisEvent.EventParam == 0)
     	            {
     	               printf("\r\nWe lost the rear beacon.");
     	               // We lost the beacon, let's look for it again
     	               NextState = AligningRearBeacon; // set next state to looking for rear beacon
     	               MakeTransition = True; // mark that we are making a transition
     	               ReturnEvent.EventType = ES_NO_EVENT; // consume the event
     	            }
     	         break;
     	         */
     	         
     	      } // End event type switch 
     	   } // End guard against no event
  	   break; // End DrivingForward100 State
  	   
  	   case Unloading:
     	   printf("\r\nIn the Unloading state.");
     	   // Execute the during function for aligning rear beacon
     	   // Entry and exit functions are processed here
     	   ThisEvent = DuringUnloading(ThisEvent);
     	   
     	   // Process events
     	   // Check if there is an event to respond to
     	   if (ThisEvent.EventType != ES_NO_EVENT)
     	   {
     	      // Switch on event types
     	      switch(ThisEvent.EventType)
     	      {
     	         case ES_TIMEOUT:
     	            switch (ThisEvent.EventParam)
     	            {
     	               
     	               case MOTION_TIMER:
        	               printf("\r\nRobot has rammed the bin and door should be open. Start shuffling.");
           	            // Time to do the shuffle
           	            NextState = Shuffling;// determine what the next state will be
                        MakeTransition = True; // mark that we are making a transition
                        ReturnEvent.EventType = ES_NO_EVENT; // consume the event  
     	               break;
     	               
     	               case UNLOADING_DELAY_TIMER:
        	               printf("\r\nFans spun down. Start moving forward.");
        	               // Go forward then back up into the wall to help open the door
        	               GoForward(100); // move the bot forward
        	               // Set timer for length of forward pulse
        	               ES_Timer_StopTimer(UNLOADING_BUMP_TIMER);
        	               ES_Timer_SetTimer(UNLOADING_BUMP_TIMER, FORWARD_BUMP_INTERVAL);
        	               ES_Timer_StartTimer(UNLOADING_BUMP_TIMER);
     	               break;
     	               
     	               case UNLOADING_BUMP_TIMER:
     	                  // The bot went far enough forward, now go back.
     	                  printf("\r\nThe bot has completed forward path, now head back.");
     	                  GoBackward(100);     	                  
     	               break;
     	            } // end time out type switch     	            
     	         break;
     	         
     	         case ES_REAR_BUMPED:
     	            // The rear bumper has been pressed. Time to stop the bot
     	            printf("\r\nRear bumped. Stop the bot.");
     	            FullStop(); // stop the bot
     	            
     	            // Set timer for delay from rear ramming to shuffling
            	   ES_Timer_StopTimer(MOTION_TIMER);
            	   ES_Timer_SetTimer(MOTION_TIMER, UNLOAD_INTERVAL);
            	   ES_Timer_StartTimer(MOTION_TIMER);
     	         break;
     	         
     	      } // End event type switch 
     	   } // End guard against no event
  	   break; // End Unloading State
  	   
  	   case Shuffling:
     	   printf("\r\nIn the Shuffling state.");
     	   // Execute the during function for aligning rear beacon
     	   // Entry and exit functions are processed here
     	   ThisEvent = DuringShuffling(ThisEvent);
     	   
     	   // Process events
     	   // Check if there is an event to respond to
     	   if (ThisEvent.EventType != ES_NO_EVENT)
     	   {
     	      // Switch on event types
     	      switch(ThisEvent.EventType)
     	      {
     	         case ES_TIMEOUT:
     	            // Check if this is a change direction or end shuffle time out
     	            switch (ThisEvent.EventParam)
     	            {
     	               case SHUFFLE_TIMER:
     	                  printf("\r\nDone shuffling.");
     	                  // Time to stop shuffling
     	                  // Create an event to exit the scoring SM
     	                  ReturnEvent.EventType = ES_BALL_BIN_EMPTY; // signal to the NHSM that we are done               
     	               break;
     	               
     	               case SHUFFLE_STEP_TIMER:
     	                  printf("\r\nChange shuffle direction.");
     	                  // Change the direction of the shuffle
     	                  switch (ShuffleDirection)
     	                  {
     	                     case Left:
     	                        TurnLeft(); // turn left 
     	                        ShuffleDirection = Right; // update direction for next time
     	                        printf("\r\nShuffle left.");
     	                     break;
     	                     
     	                     case Right:
     	                        TurnRight(); // turn right
     	                        ShuffleDirection = Left; // update direction for next time
     	                        printf("\r\nShuffle right.");
     	                     break;
     	                  }
     	                  // Set shuffle step timer
  	                     ES_Timer_StopTimer(SHUFFLE_STEP_TIMER);
  	                     ES_Timer_SetTimer(SHUFFLE_STEP_TIMER, SHUFFLE_STEP_INTERVAL);
  	                     ES_Timer_StartTimer(SHUFFLE_STEP_TIMER);
     	               break;
     	            }
     	         break;     	              	         
     	      } // End event type switch 
     	   } // End guard against no event
  	   break; // End Shuffling State
  	   
  	   case FindingLeftBeacon:
         printf("\r\nIn the FindingLeftBeacon state.");
         
         // Execute the during function for FindingLeftBeacon state
         // Entry and exit functions are processed here
         ThisEvent = DuringFindingLeftBeacon(ThisEvent);
         
         // Process events
         // Check if there is an event to respond to     	            
         if (ThisEvent.EventType != ES_NO_EVENT)
         {
            // Switch on event types
            switch (ThisEvent.EventType)
            {
               case ES_BEACON_FRONT:
                  if (ThisEvent.EventParam == LeftBin)
                  {
                     // We have found the left beacon
                     NextState = FindingRightBeacon; // set next state to look for right beacon
                     MakeTransition = True; // indicate that we are making a transition
                     ReturnEvent.EventType = ES_NO_EVENT; // consume the event
                  }
               break;
            }
         } // End guard on no event
      break; // end finding left beacon state
     	         
      case FindingRightBeacon:
         printf("\r\nIn the FindingRightBeacon state.");
         
         // Execute the during function for FindingLeftBeacon state
         // Entry and exit functions are processed here
         ThisEvent = DuringFindingRightBeacon(ThisEvent);
         
         //Process events
         // Check if there is an event to respond to     	            
         if (ThisEvent.EventType != ES_NO_EVENT)
         {
            // Switch on event types
            switch (ThisEvent.EventType)
            {
               case ES_BEACON_FRONT:
                  if (ThisEvent.EventParam == RightBin)
                  {
                     // We have found the right beacon
                     NextState = BisectingAngle; // set next state to look for right beacon
                     MakeTransition = True; // indicate that we are making a transition
                     ReturnEvent.EventType = ES_NO_EVENT; // consume the event
                  }
               break;     	                  
            }
         } // End guard on no event
      break; // End Finding right beacon state
      
      case BisectingAngle:
         printf("\r\nIn the BisectingAngle state.");
         
         // Execute the during function for FindingLeftBeacon state
         // Entry and exit functions are processed here
         ThisEvent = DuringBisectingAngle(ThisEvent);
         
         //Process events
         // Check if there is an event to respond to     	            
         if (ThisEvent.EventType != ES_NO_EVENT)
         {
            // Switch on event types
            switch (ThisEvent.EventType)
            {
               case ES_TIMEOUT:
                  if(ThisEvent.EventParam == MOTION_TIMER)
                  {
                     // The bot has rotated back enough to bisect the angle
                     NextState = DrivingForward_Alignment; // set the next state to forward align
                     MakeTransition = True; // indicate that we are making a transition
                     ReturnEvent.EventType = ES_NO_EVENT; // consume the event
                    
                    
                    // March 4th, Hannah 
                     // Set the timer length based on which pass this is
     	               if (ApproachPass == (MAX_APPROACH_PASSES-1))
     	               {
     	                  // This is the first pass
     	                  ES_Timer_StopTimer(MOTION_TIMER);
     	                  ES_Timer_SetTimer(MOTION_TIMER, FIRST_FWD_ALIGN_INTERVAL);   
     	               }
     	               else if (ApproachPass == (MAX_APPROACH_PASSES))
     	               {
     	                  // This is the second pass
     	                  ES_Timer_StopTimer(MOTION_TIMER);
     	                  ES_Timer_SetTimer(MOTION_TIMER, SECOND_FWD_ALIGN_INTERVAL);
     	               }
     	               ES_Timer_StartTimer(MOTION_TIMER); // Start timer
                     
                  }
               break;
            }
         } // End guard on no event           
      break; // end bisecting angle state
     	         
  	    
  	} // End switch on current state
  	
  	// If we are making a state transition
  	if (MakeTransition == True)
  	{
  		// Execute exit function for current state
  		ThisEvent.EventType = ES_EXIT;
  		RunScoringSM(ThisEvent);
  		
  		CurrentState = NextState; // Update state variable
  		
  		// Execute entry function for the new state
  		ThisEvent.EventType = ES_ENTRY;
  		RunScoringSM(ThisEvent);
  	}
  	
  	return ReturnEvent;
}

/***************************************************************************
private functions
***************************************************************************/
static ES_Event DuringAligningRearBeacon(ES_Event ThisEvent)
{
	if (ThisEvent.EventType == ES_ENTRY)
	{
	   //printf("\r\n\nENTERING the AligningRearBeacon state.");
	   // Process ES_ENTRY event
	   // Turn right while looking for beacons
	   TurnRight();
	   
	   #ifdef BEACON_NOD
	   // Set timer for opposite swing to get off the current beacon
	   ES_Timer_StopTimer(BEACON_NOD_TIMER);
	   ES_Timer_SetTimer(BEACON_NOD_TIMER, BEACON_NOD_INTERVAL);
	   ES_Timer_StartTimer(BEACON_NOD_TIMER);
	   #endif

	}
	else if (ThisEvent.EventType == ES_EXIT)
	{
		//printf("\r\n\nEXITING the AligningRearBeacon state.");
		// Process exit event
		// Stop the robot
		FullStop();
	}
	else
	{
	   // Perform during functions  
	   // No during functions for this state
	}
	return ThisEvent; // do not remap event   
}

static ES_Event DuringAligningFrontBeacon(ES_Event ThisEvent)
{
	if (ThisEvent.EventType == ES_ENTRY)
	{
	   //printf("\r\n\nENTERING the AligningFrontBeacon state.");
	   // Process ES_ENTRY event
      // Turn right while looking for beacons
	   TurnRight();
	   
	   #ifdef BEACON_NOD
	   // Set timer for opposite swing to get off the current beacon
	   ES_Timer_StopTimer(BEACON_NOD_TIMER);
	   ES_Timer_SetTimer(BEACON_NOD_TIMER, BEACON_NOD_INTERVAL);
	   ES_Timer_StartTimer(BEACON_NOD_TIMER);
	   #endif
	   
	   #ifdef BACKUP_SEARCH
	   // Set timer for abandoning the search for the front beacon
	   ES_Timer_StopTimer(GO_TO_BACKUP_SEARCH_TIMER);
	   ES_Timer_SetTimer(GO_TO_BACKUP_SEARCH_TIMER, GO_TO_BACKUP_SEARCH_INTERVAL);
	   ES_Timer_StartTimer(GO_TO_BACKUP_SEARCH_TIMER);
	   #endif
	}
	else if (ThisEvent.EventType == ES_EXIT)
	{
		//printf("\r\n\nEXITING the AligningFrontBeacon state.");
		// Process exit event
		// Stop the robot
		FullStop();
		ApproachPass++; // increment approach counter
	}
	else
	{
	   // Perform during functions
	   // No during functions for this state  
	}
	return ThisEvent; // do not remap event   
}

static ES_Event DuringDrivingForward_Clearance(ES_Event ThisEvent)
{
	if (ThisEvent.EventType == ES_ENTRY)
	{
	   printf("\r\n\nENTERING the DrivingForward_Clearance state.");
	   // Process ES_ENTRY event
	   GoForward(100);
	   
	   // Set timer for clearance space
	   ES_Timer_StopTimer(MOTION_TIMER);
	   ES_Timer_SetTimer(MOTION_TIMER, CLEARANCE_INTERVAL);
	   ES_Timer_StartTimer(MOTION_TIMER);
	   printf("\r\nTimer SET for driving forward clearance.");

	}
	else if (ThisEvent.EventType == ES_EXIT)
	{
		printf("\r\n\nEXITING the DrivingForward_Clearance state.");
		// Process exit event
		// Stop the robot
		FullStop();
	}
	else
	{
	   // Perform during functions
	   // No during functions for this state  
	}
	return ThisEvent; // do not remap event   
}

static ES_Event DuringDrivingForward_Alignment(ES_Event ThisEvent)
{
	if (ThisEvent.EventType == ES_ENTRY)
	{
	   printf("\r\n\nENTERING the DrivingForward_Alignment state.");
	   // Process ES_ENTRY event
	   GoForward(100);
	}
	else if (ThisEvent.EventType == ES_EXIT)
	{
		printf("\r\n\nEXITING the DrivingForward_Alignment state.");
		// Process exit event
		// Stop the robot
		FullStop();
	}
	else
	{
	   // Perform during functions
	   // No during functions for this state  
	}
	return ThisEvent; // do not remap event   
}

static ES_Event DuringBackingUp(ES_Event ThisEvent)
{
	if (ThisEvent.EventType == ES_ENTRY)
	{
	   printf("\r\n\nENTERING the BackingUp state.");
	   // Process ES_ENTRY event
	   GoBackward(100);

	}
	else if (ThisEvent.EventType == ES_EXIT)
	{
		printf("\r\n\nEXITING the BackingUp state.");
		// Process exit event
		// Stop the robot
		FullStop();
	}
	else
	{
	   // Perform during functions
	   // No during functions for this state  
	}
	return ThisEvent; // do not remap event   
}

static ES_Event DuringUnloading(ES_Event ThisEvent)
{
	if (ThisEvent.EventType == ES_ENTRY)
	{
	   printf("\r\n\nENTERING the Unloading state.");
	   // Process ES_ENTRY event
	   // Turn off motors and fans
	   FullStop();
	   FanControl(0);
	   
	   // Set timer to allow for fan spin down 
	   ES_Timer_StopTimer(UNLOADING_DELAY_TIMER);
	   ES_Timer_SetTimer(UNLOADING_DELAY_TIMER, FAN_SPIN_DOWN_TIMER);
	   ES_Timer_StartTimer(UNLOADING_DELAY_TIMER);
	}
	else if (ThisEvent.EventType == ES_EXIT)
	{
		printf("\r\n\nEXITING the Unloading state.");
		// Process exit event
		// No exit functions for this state
	}
	else
	{
	   // Perform during functions
	   // No during functions for this state  
	}
	return ThisEvent; // do not remap event   
}

static ES_Event DuringShuffling(ES_Event ThisEvent)
{
	if (ThisEvent.EventType == ES_ENTRY)
	{
	   printf("\r\n\nENTERING the Shuffling state.");
	   // Process ES_ENTRY event
      // Commence shuffling
      switch (ShuffleDirection)
      {
         case Left:
            TurnLeft(); // turn left 
            ShuffleDirection = Right; // update direction for next time
            printf("\r\nShuffle left.");
         break;
         
         case Right:
            TurnRight(); // turn right
            ShuffleDirection = Left; // update direction for next time
            printf("\r\nShuffle right.");
         break;
      }
      
      // Set shuffle step timer
      ES_Timer_StopTimer(SHUFFLE_STEP_TIMER);
      ES_Timer_SetTimer(SHUFFLE_STEP_TIMER, SHUFFLE_STEP_INTERVAL);
      ES_Timer_StartTimer(SHUFFLE_STEP_TIMER);
      // Set overall shuffle length timer
      ES_Timer_StopTimer(SHUFFLE_TIMER);
      ES_Timer_SetTimer(SHUFFLE_TIMER, SHUFFLE_INTERVAL);
      ES_Timer_StartTimer(SHUFFLE_TIMER);
      printf("\r\nShuffle timers have been set.");
	}
	else if (ThisEvent.EventType == ES_EXIT)
	{
		printf("\r\n\nEXITING the Shuffling state.");
		// Process exit event
		// Stop the robot
		FullStop();
	}
	else
	{
	   // Perform during functions  
	}
	return ThisEvent; // do not remap event   
}

static ES_Event DuringFindingLeftBeacon(ES_Event ThisEvent)
{
   if (ThisEvent.EventType == ES_ENTRY)
   {
      // Process the ES_ENTRY event
      TurnLeft(); // make the bot spin left, looking for the left beacon
   }
   else if (ThisEvent.EventType == ES_EXIT)
   {
      // Process the ES_EXIT event
      TOSA_Left = ES_Timer_GetTime(); // get the time when the left bin signal is acquired
      FullStop(); // stop the bot
   }
   else
   {
      // Process the during functions
      // No during functions
   }
   return ThisEvent; // do not remap event
}

static ES_Event DuringFindingRightBeacon(ES_Event ThisEvent)
{
   if (ThisEvent.EventType == ES_ENTRY)
   {
      // Process the ES_ENTRY event
      TurnRight(); // make the bot spin right, looking for the right beacon
   }
   else if (ThisEvent.EventType == ES_EXIT)
   {
      // Process the ES_EXIT event
      TOSA_Right = ES_Timer_GetTime(); // get the time when the right bin signal is acquired
      FullStop(); // stop the bot
   }
   else
   {
      // Process the during functions
      // No during functions
   }
   return ThisEvent; // do not remap event
}

static ES_Event DuringBisectingAngle(ES_Event ThisEvent)
{
   if (ThisEvent.EventType == ES_ENTRY)
   {
      // Process the ES_ENTRY event
      // calculate the time betwe en the left and right beacons during sweep
      uint16_t BisectTime = (TOSA_Right - TOSA_Left)/2;
      TurnLeft(); // Spin the bot back to the left
      
      // Set the timer to stop at the bisection of the angle
      ES_Timer_StopTimer(MOTION_TIMER);
      ES_Timer_SetTimer(MOTION_TIMER, BisectTime);
      ES_Timer_StartTimer(MOTION_TIMER);
       
   }
   else if (ThisEvent.EventType == ES_EXIT)
   {
      // Process the ES_EXIT event
      FullStop(); // stop the bot
   }
   else
   {
      // Process the during functions
      // No during functions
   }
   return ThisEvent; // do not remap event
}

unsigned char QueryTargetBin(void)
{
   return TargetBin;
}