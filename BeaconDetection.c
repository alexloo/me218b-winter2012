/****************************************************************************
 Description
       BeaconDetection.c is the source file for the custom made communication firmware.

 History
 When           Who	What/Why
 -------------- ---	--------
****************************************************************************/

// Includes ****************************************************************/
#include "ES_Configure.h"
#include "ES_Framework.h"
#include "ES_PostList.h"

#include <stdio.h>
#include <mc9s12e128.h>     /* derivative information */
#include <S12e128bits.h>    /* bit definitions  */
#include "S12eVec.h"
#include "BeaconDetection.h"
#include "QuickSense.h"
#include "TimingConstants.h"
#include "ES_Timers.h"
#include "MasterMachine.h"
#include "ScoringSM.h"
#include "DefendingSM.h"

// Module Defines **********************************************************/
#define PERIOD_NOBEACON (22L * (unsigned long)(1 _MS_))
#define PERIOD_OC_CHECKBEACON (20 _MS_) 

// Module Private Functions ************************************************/
// Module Variables ********************************************************/
// Vaiables local to this module: BeaconSeen_Front, BeaconSeen_Rear, LastTime_Rear, LastTime_Front
static unsigned char BeaconSeen_Front = 0;
static unsigned char BeaconSeen_Rear = 0;
static unsigned long LastTime_Rear = 0;
static unsigned long LastTime_Front = 0;

// Module Code *************************************************************/
/****************************************************************************
 Function
     BeaconDetection_Init

 Parameters
     None

 Returns
     None

 Description
     Initialize the interrupts needed to identify the beacons
     and the absence of a beacon 
 Notes

 Author
     Hannah C. Droesbeke
****************************************************************************/
void BeaconDetection_Init( void )
{
	// Set IC5 to capture rising edge
	TIM1_TCTL3 |= _S12_EDG5A;
	// Clear IC5 Flag
	TIM1_TFLG1 = _S12_C5F;
	// Enable IC5 Interrupt
	TIM1_TIE |= _S12_C5I;
	
	// Set IC6 to capture rising edge
	TIM1_TCTL3 |= _S12_EDG6A;
	// Clear IC6 Flag
	TIM1_TFLG1 = _S12_C6F;
	// Enable IC6 Interrupt
	TIM1_TIE |= _S12_C6I;
	
	// Set cap/comp 7 to output compare
	TIM1_TIOS |= _S12_IOS7;
	TIM1_TCTL1 = TIM1_TCTL1 & ~(_S12_OL7 | _S12_OM7);
	// Schedule first event in time PeriodOC
	TIM1_TC7 = TIM1_TCNT + PERIOD_OC_CHECKBEACON;
	// Clear OC7 flag
	TIM1_TFLG1 = _S12_C7F;
	// Enable OC7 Interrupt
	TIM1_TIE |= _S12_C7I; 

	// No need to enable Interrupts, they will initialized in QS_Initialize()
}  //End of BeaconDetection_Init

/****************************************************************************
 Function
     ResponseToIC5

 Parameters
     None

 Returns
     None

 Description
     Interrupt response routine to the Input Capture in channel 5 of
     timer 1. Posts an event in case a new beacon has been detected in
     the front. 
 Notes

 Author
     Hannah C. Droesbeke
****************************************************************************/
void interrupt _Vec_tim1ch5 ResponseToIC5( void )
{
    // Local variables in the function : CurrentTime, LastFrontPeriod, FrontPeriod, uOverFlows, CurrentRegister
    // Initialize LastFrontPeriod to 0
    static unsigned long CurrentTime;
    static unsigned long LastFrontPeriod = 0;
    unsigned long FrontPeriod;
    unsigned int uOverFlows;
    unsigned int CurrentRegister;
    ES_Event ThisEvent;
    
   // Clear flag
   TIM1_TFLG1 = _S12_C5F;
   
   // Get the overflow count in timer 1
   uOverFlows = QS_QueryTIM1Overflow();
   // Get the current time in channel 5
   CurrentRegister = TIM1_TC5;
   // Calculate CurrentTime
   CurrentTime = ((unsigned long) uOverFlows << 16) + CurrentRegister;

   // Set FrontPeriod (ms) to CurrentTime - LastTime_Front
   FrontPeriod = (CurrentTime - LastTime_Front) /((unsigned long)(1 _MS_));
   // Set LastTime_Front to CurrentTime
   LastTime_Front = CurrentTime;

   // If the FrontPeriod is different than LastFrontPeriod and if we are in one of the following modes:
   // AligningFrontBeacon, FindingLeftBeacon, FindingRightBeacon, PreGame   
   if ((FrontPeriod != LastFrontPeriod) && ((QueryScoringSM() == AligningFrontBeacon) || (QueryScoringSM() == FindingLeftBeacon) || (QueryScoringSM() == FindingRightBeacon) || (QueryMasterMachine() == PreGame)))
   {  
      // If the Period is 20 or 19 (adjusted from tests), and the last beacon seen was not Beacon 1
      if (BeaconSeen_Front != 1 && (FrontPeriod == PERIOD_1 || FrontPeriod == PERIOD_1 - 1))
      {
            // Post Event ES_BEACON_FRONT with parameter 1
            ThisEvent.EventType = ES_BEACON_FRONT;
            ThisEvent.EventParam = 1;
            PostMasterMachine(ThisEvent);
	    // Update BeaconSeen_Front to 1
            BeaconSeen_Front = 1;
      } 
      // Else if the Period is 18 or 17 (adjusted from tests), and the last beacon seen was not Beacon 2
      else if (BeaconSeen_Front != 2 && (FrontPeriod == PERIOD_2 || FrontPeriod == PERIOD_2 - 1))
      {
            // Post Event ES_BEACON_FRONT with parameter 2
            ThisEvent.EventType = ES_BEACON_FRONT;
            ThisEvent.EventParam = 2;
            PostMasterMachine(ThisEvent);
	    // Update BeaconSeen_Front to 2
            BeaconSeen_Front = 2;
     } 
      // Else if the Period is 16 or 15 (adjusted from tests), and the last beacon seen was not Beacon 3
      else if (BeaconSeen_Front != 3 && (FrontPeriod == PERIOD_3 || FrontPeriod == PERIOD_3 - 1))
      {
            // Post Event ES_BEACON_FRONT with parameter 3
	    ThisEvent.EventType = ES_BEACON_FRONT;
            ThisEvent.EventParam = 3;
            PostMasterMachine(ThisEvent);
	    // Update BeaconSeen_Front to 3
            BeaconSeen_Front = 3;
      } 	
      // Else if the Period is 14 or 13 (adjusted from tests), and the last beacon seen was not Beacon 4
      else if (BeaconSeen_Front != 4 && (FrontPeriod == PERIOD_4 || FrontPeriod == PERIOD_4 - 1))
      {
            // Post Event ES_BEACON_FRONT with parameter 4
            ThisEvent.EventType = ES_BEACON_FRONT;
            ThisEvent.EventParam = 4;
            PostMasterMachine(ThisEvent);
	    // Update BeaconSeen_Front to 4
           BeaconSeen_Front = 4;
      }  //Endif
      // Set LastFrontPeriod to FrontPeriod
      LastFrontPeriod = FrontPeriod;
   }  //Endif

}  //End of ResponseToIC5

/****************************************************************************
 Function
     ResponseToIC6

 Parameters
     None

 Returns
     None

 Description
     Interrupt response routine to the Input Capture in channel 6 of
     timer 1. Posts an event in case a new beacon has been detected in
     the rear.
 Notes

 Author
     Hannah C. Droesbeke
****************************************************************************/
void interrupt _Vec_tim1ch6 ResponseToIC6( void )
{
    // Local variables in the function : CurrentTime, LastRearPeriod, RearPeriod, uOverFlows, CurrentRegister
    // Initialize LastRearPeriod to 0
    static unsigned long CurrentTime;
    static unsigned long LastRearPeriod = 0;
    unsigned long RearPeriod;
    unsigned int uOverFlows;
    unsigned int CurrentRegister;
    ES_Event ThisEvent;
    
   // Clear flag
   TIM1_TFLG1 = _S12_C6F;
   
   // Get the overflow count in timer 1
   uOverFlows = QS_QueryTIM1Overflow();
   // Get the current time in channel 6
   CurrentRegister = TIM1_TC6;
  // Calculate CurrentTime
   CurrentTime = ((unsigned long) uOverFlows << 16) + CurrentRegister;

   // Set FrontPeriod (ms) to CurrentTime - LastTime_Front
   RearPeriod = (CurrentTime - LastTime_Front) /((unsigned long)(1 _MS_));
   // Set LastTime_Rear to CurrentTime
   LastTime_Rear = CurrentTime;

   // If the FrontPeriod is different than LastFrontPeriod and if we are in one of the following modes:
   // AligningRearBeacon, PreGame, Realigning   
   if ((RearPeriod != LastRearPeriod) && ((QueryScoringSM() == AligningRearBeacon) || (QueryMasterMachine() == PreGame) || (QueryDefendingSM() == Realigning))) 
   {  
      // If the Period is 20 or 19 (adjusted from tests), and the last beacon seen was not Beacon 1
      if (BeaconSeen_Rear != 1 && (RearPeriod == PERIOD_1 || RearPeriod == PERIOD_1 - 1))
      {
            // Post Event ES_BEACON_REAR with parameter 1
            ThisEvent.EventType = ES_BEACON_REAR;
            ThisEvent.EventParam = 1;
            PostMasterMachine(ThisEvent);
	    // Update BeaconSeen_Rear to 1
            BeaconSeen_Rear = 1;
      } 
      // Else if the Period is 18 or 17 (adjusted from tests), and the last beacon seen was not Beacon 2
      else if (BeaconSeen_Rear != 2 && (RearPeriod == PERIOD_2 || RearPeriod == PERIOD_2 - 1))
      {
            // Post Event ES_BEACON_REAR with parameter 2
            ThisEvent.EventType = ES_BEACON_REAR;
            ThisEvent.EventParam = 2;
            PostMasterMachine(ThisEvent);
	    // Update BeaconSeen_Rear to 2
            BeaconSeen_Rear = 2;
     } 
      // Else if the Period is 16 or 15 (adjusted from tests), and the last beacon seen was not Beacon 3
      else if (BeaconSeen_Rear != 3 && (RearPeriod == PERIOD_3 || RearPeriod == PERIOD_3 - 1))
      {
            // Post Event ES_BEACON_REAR with parameter 3
	    ThisEvent.EventType = ES_BEACON_REAR;
            ThisEvent.EventParam = 3;
            PostMasterMachine(ThisEvent);
	    // Update BeaconSeen_Rear to 3
            BeaconSeen_Rear = 3;
      } 	
      // Else if the Period is 14 or 13 (adjusted from tests), and the last beacon seen was not Beacon 4
      else if (BeaconSeen_Rear != 4 && (RearPeriod == PERIOD_4 || RearPeriod == PERIOD_4 - 1))
      {
            // Post Event ES_BEACON_REAR with parameter 4
            ThisEvent.EventType = ES_BEACON_REAR;
            ThisEvent.EventParam = 4;
            PostMasterMachine(ThisEvent);
	    // Update BeaconSeen_Rear to 4
           BeaconSeen_Rear = 4;
      }  //Endif
      // Set LastRearPeriod to RearPeriod
      LastRearPeriod = RearPeriod;
   }  //Endif 

}  //End of ResponseToIC6


/****************************************************************************
 Function
     CheckNoBeacon

 Parameters
     None

 Returns
     None

 Description
     Interrupt response routine to the Output Compare in channel 7 of
     timer 1. Posts an event in case we lost the beacon we were seing
     in the front or in the rear. 
 Notes

 Author
     Hannah C. Droesbeke
****************************************************************************/
void interrupt _Vec_tim1ch7 CheckNoBeacon(void)
{
    // Local variables in the function : CurrentTime, uOverFlows, CurrentRegister
   ES_Event ThisEvent;
   unsigned long CurrentTime;
   unsigned int uOverFlows;
   unsigned int CurrentRegister;
      
   // Get the overflow count in timer 1
   uOverFlows = QS_QueryTIM1Overflow();
   // Get the current time in channel 7
   CurrentRegister = TIM1_TC7;
  // Calculate CurrentTime
   CurrentTime = ((unsigned long) uOverFlows << 16) + CurrentRegister;
    
   // Clear OC7 flag
   TIM1_TFLG1 = _S12_C7F;
   
   // If BeaconSeen_Front is not 0 
   if (BeaconSeen_Front != 0)
   {
      // If the time elapsed since the last time we were seen a beacon in the front is more than PERIOD_NOBEACON
      if ((CurrentTime - LastTime_Front) > PERIOD_NOBEACON)
      {
         // Post Event ES_BEACON_FRONT with parameter 0 (No Beacon in front)
         ThisEvent.EventType = ES_BEACON_FRONT;
         ThisEvent.EventParam = 0;
         PostMasterMachine(ThisEvent);
         // Set BeaconSeen_Front to 0
         BeaconSeen_Front = 0;
      }  //Endif
   }  //Endif
   
   // If BeaconSeen_Rear is not 0
   if (BeaconSeen_Rear != 0)
   {
      // If the time elapsed since the last time we were seen a beacon in the rear is more than PERIOD_NOBEACON
      if ((CurrentTime - LastTime_Rear) > PERIOD_NOBEACON)
      {
         // Post Event ES_BEACON_REAR with parameter 0 (No Beacon in rear)
        ThisEvent.EventType = ES_BEACON_REAR;
        ThisEvent.EventParam = 0;
        PostMasterMachine(ThisEvent);
        // Set BeaconSeen_Rear to 0
        BeaconSeen_Rear = 0;
      } //Endif  
   }  //Endif

   // Schedule next output compare to check for no beacon
   TIM1_TC7 = TIM1_TCNT + PERIOD_OC_CHECKBEACON;
}  //End of CheckNoBeacon

/****************************************************************************
 Function
     GetBeaconRear

 Parameters
     None

 Returns
     char: 1 for seen, 0 for not seen

 Description
     Returns value of BeaconSeen_Rear
 Notes
 
****************************************************************************/
unsigned char GetBeaconRear( void )
{
   //Return BeaconSeen_Rear
   return BeaconSeen_Rear;
}  //End of GetBeaconRear

/****************************************************************************
 Function
     GetBeaconFront

 Parameters
     None

 Returns
     char: 1 for seen, 0 for not seen

 Description
     Returns value of BeaconSeen_Front
 Notes
 
****************************************************************************/
unsigned char GetBeaconFront( void )
{
   //Return BeaconSeen_Front
   return BeaconSeen_Front;
}  //End of GetBeaconFront