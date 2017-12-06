/****************************************************************************
 Module
   MasterMachine.c

 Revision
   1.0.1

 Description
   This is a template file for implementing flat state machines under the 
   Gen2 Events and Services Framework.

 Notes

 History
 When           Who     What/Why
 -------------- ---     --------
 02/24/12 13:30 adl     Began tailoring of template to be our MasterMachine
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
#include "MasterMachine.h"
#include "GatheringSM.h"
#include "ScoringSM.h"
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

#include "EventPrinter.h"

/*----------------------------- Module Defines ----------------------------*/
//#define DEBUG

/*---------------------------- Module Functions ---------------------------*/
/* prototypes for private functions for this machine.They should be functions
   relevant to the behavior of this state machine
*/
static ES_Event DuringGatheringState(ES_Event ThisEvent);
static ES_Event DuringScoring (ES_Event ThisEvent);
static ES_Event DuringDefending (ES_Event ThisEvent);
static ES_Event DuringGameOver (ES_Event ThisEvent);

/*---------------------------- Module Variables ---------------------------*/
// everybody needs a state variable, you may need others as well.
// type of state variable should match htat of enum in header file
static MasterMachineState_t CurrentState;

// with the introduction of Gen2, we need a module level Priority var as well
static uint8_t MyPriority;


/*------------------------------ Module Code ------------------------------*/
/****************************************************************************
 Function
     InitMasterMachine

 Parameters
     uint8_t : the priorty of this service

 Returns
     boolean, False if error in initialization, True otherwise

 Description
     Saves away the priority, sets up the initial transition and does any 
     other required initialization for this state machine
 Notes

 Author
     J. Edward Carryer, 10/23/11, 18:55
****************************************************************************/
boolean InitMasterMachine ( uint8_t Priority )
{
  ES_Event ThisEvent;

  MyPriority = Priority;
  
  // Initialize the MotorDriver module
   MotorDriver_Init();
   // Initialize the interrupt based sensor module
   QS_Initialize();
   // Initialize the SPI-based FSR module
   SPI_Init();
   // Initialize the beacon detection module  - Must be called after QS_initialize
   //BeaconDetection_Init();
   // Initialize the Side Identification module
   ID_Initialize();
   // Initialize the bin control module
   InitFan();
   
   printf("\r\nModule initialization sequence complete.");
   
   ThisEvent.EventType = ES_ENTRY;
   // Start the MasterMachine state machine
   StartMasterMachine(ThisEvent);
   
   return True;
}

/****************************************************************************
 Function
     PostMasterMachine

 Parameters
     EF_Event ThisEvent , the event to post to the queue

 Returns
     boolean False if the Enqueue operation failed, True otherwise

 Description
     Posts an event to this state machine's queue
 Notes

 Author
     J. Edward Carryer, 10/23/11, 19:25
****************************************************************************/
boolean PostMasterMachine( ES_Event ThisEvent )
{
  return ES_PostToService( MyPriority, ThisEvent);
}

/****************************************************************************
 Function
    StartMasterMachine

 Parameters
   ES_Event CurrentEvent: the event to process

 Returns
   None

 Description
   This function starts the MasterMachine SM. 
 Notes

 Author
   Alex Loo, 02/24/12, 18:07
****************************************************************************/
void StartMasterMachine (ES_Event CurrentEvent)
{
   unsigned char BeaconSeen = 0;
   // Local variable to allow the debugger to see the value of CurrentEvent
   volatile ES_Event LocalEvent = CurrentEvent;
   
   // Turn the fans on
   FanControl(1);
   
   #ifndef DEBUG
   // MasterMachine SM always starts in PreGame mode
   CurrentState = PreGame;
   
   // Run entry function for state PreGame
   // Find a beacon on either front or back
   
   do 
   {
      BeaconSeen = GetBeaconFront();
      if (BeaconSeen == 0)
      {
         // Still couldn't find beacon, try rear
         BeaconSeen = GetBeaconRear();
      }
   } while (BeaconSeen == 0);
   printf("\r\nFound beacon %u on our side.", BeaconSeen);
   #endif
   
   #ifdef DEBUG
   CurrentState = PreGame;
   BeaconSeen = 1;
   #endif
   
   
   // Light the LED based on the beacon we see   
   ID_IdentifySide(BeaconSeen);

   printf("\r\nMasterMachine SM start sequence complete.");
      printf("\r\n\nENTERING the PreGame state.");
   // Run the MasterMachine SM
   RunMasterMachine(LocalEvent); // use LocalEvent to avoid unused variable warnings   
}

/****************************************************************************
 Function
    RunMasterMachine

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
ES_Event RunMasterMachine( ES_Event ThisEvent )
{
   static uint16_t LastTime = 0;
   uint16_t CurrentTime;
   unsigned char BallsInBin = 0;
   boolean MakeTransition = False; // are we making a state transition?
   MasterMachineState_t NextState = CurrentState;
   // Top level state machine should always return no event in the absence of an error
   ES_Event ReturnEvent;
   ReturnEvent.EventType = ES_NO_EVENT;
   
   
   // Test section for printing the balls collected every few seconds
   CurrentTime = ES_Timer_GetTime();
   if(CurrentTime - LastTime > 2 _SECONDS_TIMER)
   {
      BallsInBin = QS_QueryBallCount();
      printf("\r\n\nBalls in bin = %u\n\n", BallsInBin);      
      LastTime = CurrentTime; // update last time
   }
   // End test section
   
   
   EventPrinter(ThisEvent);
   // Begin switch on states of the MasterMachine
   switch (CurrentState)
   {
      case PreGame:
         printf("\r\nIn the PreGame state.");
         // This is a flat state before the game starts. It is only waiting for a non-
         // zero response from the FSR for balls present
         if (ThisEvent.EventType == ES_GAME_START)
         {
            // Determine the beacon that we are currently staring at
            // unsigned char LastSeenBeacon = BD_QueryLastBeacon;
            // the above function will return a number 1 - 4
            
            // The next step is to identify our side
            NextState = Gathering; // move to gathering mode
            MakeTransition = True; // mark that we're making a transition
            
            // Perform exit function to the PreGame state
            // Doing it here because there's only one way to leave current state
            // ID_IdentifySide(LastBeaconSeen);
            
            // Set timers for game tracking
            // Set timer indicating that the game is over
            ES_Timer_SetTimer(END_GAME_TIMER, LENGTH_OF_GAME);
            ES_Timer_StartTimer(END_GAME_TIMER);
            // Set timer to advance to the scoring mode
            ES_Timer_SetTimer(GO_TO_SCORING_TIMER, PROCEED_TO_SCORING);
            ES_Timer_StartTimer(GO_TO_SCORING_TIMER); 
         }
         break;
      
      case Gathering:
         printf("\r\nIn the Gathering state.");
         // Run during function for the Gathering state. Entry and Exits are processed 
         // here
         ThisEvent = DuringGatheringState(ThisEvent);
         
         // Process any events 
         if (ThisEvent.EventType != ES_NO_EVENT)
         {
            switch (ThisEvent.EventType)
            {
               case ES_TIMEOUT:
                  if (ThisEvent.EventParam == GO_TO_SCORING_TIMER)
                  {
                     // Timer signaling to advance to Scoring Mode has expired
                     NextState = Scoring; // move to scoring mode
                     MakeTransition = True; // mark that we're making a transition                     
                  } // End timer check
                  
                  else if (ThisEvent.EventParam == END_GAME_TIMER)
      	         {
      	            // Timer signaling the end of the game has expired
      	            NextState = GameOver; // move to game over mode
                     MakeTransition = True; // mark that we're making a transition 
      	         } 
               break;
            } // End switch on event type
         } // End guard against no event
      break; // End Gathering state
      
      case Scoring:
         printf("\r\nIn the Scoring state.");
      	// Run during function for the Gathering state. Entry and Exits are 
      	// processed here
      	ThisEvent = DuringScoring(ThisEvent);
      	
      	// Process any events
      	if (ThisEvent.EventType != ES_NO_EVENT)
      	{
      	   switch (ThisEvent.EventType)
      	   {
      	      case ES_BALL_BIN_EMPTY:
                  // The bin has been emptied; need to defend our bin
                  NextState = Defending; // move to defending mode
                  MakeTransition = True; // mark that we're making a transition      	      
      	      break; // End response to ball pin empty event
      	      
      	      case ES_TIMEOUT:
      	         if (ThisEvent.EventParam == END_GAME_TIMER)
      	         {
      	            // Timer signaling the end of the game has expired
      	            NextState = GameOver; // move to game over mode
                     MakeTransition = True; // mark that we're making a transition 
      	         }      	      
      	      break; // End time out event
      	      
      	   } // End switch on event type
      	} // End guard against no event
      break;
      
      case Defending:
         printf("\r\nIn the Defending state.");
         // Run during function for the Gathering state. Entry and Exits are 
      	// processed here
      	ThisEvent = DuringDefending(ThisEvent);
      	
      	// Process any events
      	if (ThisEvent.EventType != ES_NO_EVENT)
      	{
      	   switch (ThisEvent.EventType)
      	   {
      	      case ES_TIMEOUT:
      	         if (ThisEvent.EventParam == END_GAME_TIMER)
      	         {
      	            printf("\r\nThe END_GAME_TIMER expired. Go to game over.");
      	            // Timer signaling the end of the game has expired
      	            NextState = GameOver; // move to game over mode
                     MakeTransition = True; // mark that we're making a transition 
      	         }      	      
      	      break; // End time out event
      	   } // End switch on event type
      	} // End guard against no event   
      break;
      
      case GameOver:
         printf("\r\nIn the GameOver state.");
         ThisEvent = DuringGameOver(ThisEvent);
         
         // No other actions in this state
      break;
      
   } // End switch on current state
   
   // If we are making a state transition
   if (MakeTransition == True)
   {
   	// Execute exit function for current state
   	ThisEvent.EventType = ES_EXIT;
   	RunMasterMachine(ThisEvent);
   	
   	CurrentState = NextState; // Update the state variable
   	
   	// Execute the entry function for the new state
   	ThisEvent.EventType = ES_ENTRY;
   	RunMasterMachine(ThisEvent);
   }
  	return ReturnEvent;
}

/****************************************************************************
 Function
     QueryMasterMachine

 Parameters
     None

 Returns
     MasterMachineState_t The current state of the MasterMachine SM

 Description
     returns the current state of the MasterMachine SM
 Notes

 Author
     J. Edward Carryer, 10/23/11, 19:21
****************************************************************************/
MasterMachineState_t QueryMasterMachine ( void )
{
   return(CurrentState);
}

/***************************************************************************
 private functions
 ***************************************************************************/

/****************************************************************************
 Function
     DuringGatheringState

 Parameters
     ThisEvent

 Returns
      ThisEvent
      
 Description
     The during function of the Gathering state
     
 Notes

 Author
     Alex Loo, 2/25/2012/ 18:00
****************************************************************************/
static ES_Event DuringGatheringState(ES_Event ThisEvent)
{
   // Local variable for debugger
   ES_Event NewEvent = ThisEvent;
   
   // Process ES_ENTRY and ES_EXIT events
   if (ThisEvent.EventType == ES_ENTRY)
   {
      printf("\r\n\nENTERING the Gathering state.");
      // Run any start functions required for the statemachine
      StartGatheringSM(ThisEvent);
   }
   else if (ThisEvent.EventType == ES_EXIT)
   {
      printf("\r\n\nEXITING the Gathering state.");
      // Have lower level machines clean up on exit
      NewEvent = RunGatheringSM(ThisEvent);
   }
   else
   {
      // Run the during function for this state
      // Run the lower level state machine
      NewEvent = RunGatheringSM(ThisEvent);
   }
   return NewEvent;  
}

static ES_Event DuringScoring (ES_Event ThisEvent)
{
   // Local variable for debugger
   ES_Event NewEvent = ThisEvent;
   
   // Process ES_ENTRY and ES_EXIT events
   if (ThisEvent.EventType == ES_ENTRY)
   {
      printf("\r\n\nENTERING the Scoring state.");
      // Run any start functions required for this state machine
      StartScoringSM(ThisEvent);
   }
   else if (ThisEvent.EventType == ES_EXIT)
   {
      printf("\r\n\nEXITING the Scoring state.");
      // Have lower level machines clean up on exit
      NewEvent = RunScoringSM(ThisEvent);
      // No exit functions for Scoring state
   }
   else 
   {
      // No during function for this state
      // Run the lower level state machine
      NewEvent = RunScoringSM(ThisEvent);
   }
   return NewEvent;	
}

static ES_Event DuringDefending (ES_Event ThisEvent)
{
   // Local variable for debugger
   ES_Event NewEvent = ThisEvent;
   
   // Process ES_ENTRY and ES_EXIT events
   if (ThisEvent.EventType == ES_ENTRY)
   {
      printf("\r\n\nENTERING the Defending state.");
      // Run any start fucntions required for this state machine
      StartDefendingSM(ThisEvent);      
   }
   else if (ThisEvent.EventType == ES_EXIT)
   {
      printf("\r\n\nEXITING the Defending state.");
      // Have lower level machines clean up on exit
      NewEvent = RunDefendingSM(ThisEvent);
      
      // Run exit functions for Defending state
      FullStop(); // shut down wheels
      //FanControl(0);// shut down fans
   }
   else
   {
      // No during function for this state
      // Run the lower level state machine
      //printf("\r\nRunning RundefendingSM as part of the state's during function.");
      NewEvent = RunDefendingSM(ThisEvent);  
   }
   return NewEvent; 	
}

static ES_Event DuringGameOver (ES_Event ThisEvent)
{
   // Local variable for debugger
   ES_Event NewEvent = ThisEvent;
   
   // Process ES_ENTRY and ES_EXIT events
   if (ThisEvent.EventType == ES_ENTRY)
   {
      printf("\r\n\nENTERING the GameOver state.");
      // Shut down all motors
      FullStop(); // shut down wheels
      //FanControl(0);// shut down fans
      
      // Turn on both lights to have a visual indication of game over
      ID_AllLights();
        
   }
   else if (ThisEvent.EventType == ES_EXIT)
   {
      printf("\r\n\nEXITING the GameOver state.");
   }
   else
   {
      // No during function for this state 
   }
   return NewEvent; 	
}