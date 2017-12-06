// Event Checking functions for sample

#include "ES_Configure.h"
#include "ES_General.h"
#include "ES_Events.h"
#include "MasterMachine.h"
#include "EventCheckers.h"
#include "FSR.h"
#include "MasterMachine.h"
#include <stdio.h>
#include "ES_Timers.h"


// This include will pull in all of the headers from the service modules
// providing the prototypes for all of the post functions
#include "ES_ServiceHeaders.h"

boolean Check4Start(void)
{
   static uint16_t LastTime = 0;
   uint16_t CurrentTime;
   boolean ReturnVal = False;
   static unsigned char LastBallsInPlay = 1;
   
   CurrentTime = ES_Timer_GetTime();
   
   // Only care how many balls are in play if we are in pregame state
   // and wait for debounce interval of ~ 60 ms
   //if ((CurrentTime - LastTime) > 30) 
   if ((QueryMasterMachine() == PreGame) && ((CurrentTime - LastTime) > 30)) 
   {      
      unsigned char BallsInPlay = 0;
   
      // Query the FSR to find how many balls are in play
      BallsInPlay = Get_BallsInPlay();
      
      // printf for debugging
      printf("\r\n\nLast BallsInPlay = %u", LastBallsInPlay);
      printf("\r\nCurrent BallsInPlay = %u", BallsInPlay);  
      
      if ((BallsInPlay != 0) && (LastBallsInPlay == 0))
      {
         // Check if FSR has transitioned from 0 to non-zero balls in play
         // Game has started since FSR is reporting that there are balls in play
         ES_Event ThisEvent;
         ThisEvent.EventType = ES_GAME_START;
         ThisEvent.EventParam = 1;
         PostMasterMachine(ThisEvent); // this could be any SM post function or EF_PostAll
         ReturnVal = True;
         
         printf("\r\nBalls are now in play. Start the game.")         ;
      }
      
      LastBallsInPlay = BallsInPlay; // update the last read value of balls in play
      LastTime = CurrentTime; // update the last time that the FSR was queried
   }
   return ReturnVal;
}