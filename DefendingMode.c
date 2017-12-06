#include "ES_Configure.h"
#include "ES_General.h"
#include "ES_Events.h"
#include "ES_PostList.h"
#include "ES_Timers.h"
#include "MasterMachine.h"
#include <stdio.h>
#include "FSR.h"
#include "BeaconDetection.h"
#include "ScoringSM.h" // so we can query the scoring bin
#include "DefendingSM.h"
#include "SideID.h"

/* Note: This module assumes that the robot alerady went forward from the bin 
  and is ready to turn*/

#define SAFE_ANGLE 40
#define DANGER_ANGLE 35
#define ANGLES {315, 45, 135, 225}
#define ZONE_DANGER 0
#define ZONE_SAFE 1

// Static variables
static unsigned char MyBin;
static unsigned int LastWallAngle;
static unsigned int DangerAngle_Right;
static unsigned int DangerAngle_Left;
static unsigned int SafeAngle_Right;
static unsigned int SafeAngle_Left;

/****************************************************************************
 Function
   DefendingMode_Init
 Parameters
   None
 Returns
   None
 Description
   Initialize the defending mode, by identifying the bin defended
 Notes

 Author
   Hannah Droesbeke
****************************************************************************/
 void DefendingMode_Init(void)
 {
   static unsigned char Side;
   static unsigned int Angles_Array[4] = ANGLES;

   // Query the bin we are defending
   MyBin = QueryTargetBin();
   
   // Initialize the angle of the wall
   LastWallAngle = Get_WallAngle();
   
   // Query the side we are on
   Side = ID_QuerySide();
   // Depending on the side, set the limit angles for the defending mode
   if (Side == RED_TEAM)
   {
      DangerAngle_Right = (Angles_Array[MyBin-1] - DANGER_ANGLE + 180)%360;
      DangerAngle_Left = (Angles_Array[MyBin-1] + DANGER_ANGLE)%360;
      SafeAngle_Right = (Angles_Array[MyBin-1] - SAFE_ANGLE + 180)%360;
      SafeAngle_Left = (Angles_Array[MyBin-1] + SAFE_ANGLE)%360;      
   }
   else
   {
      DangerAngle_Right = (Angles_Array[MyBin-1] - DANGER_ANGLE)%360;
      DangerAngle_Left = (Angles_Array[MyBin-1] + DANGER_ANGLE + 180)%360;
      SafeAngle_Right = (Angles_Array[MyBin-1] - SAFE_ANGLE )%360;
      SafeAngle_Left = (Angles_Array[MyBin-1] + SAFE_ANGLE + 180)%360;
   }
   
   // Prevent from zero-360 crossing
   if (DangerAngle_Right > DangerAngle_Left)
   {
      DangerAngle_Left += 360; 
   }
 }

/****************************************************************************
 Function
   Wall_CheckEvents
 Parameters
   None
 Returns
   Boolean True if event detected 
 Description
   Sends an event DANGER_WALL or NODANGER_WALL when the rotatable wall enters
   or exits the zones of interest.
 Notes

 Author
   Hannah Droesbeke
****************************************************************************/
boolean Wall_CheckEvents( void )
{
   // Static variables to this function: WallAngle, CurrentZone, Angles_Array, 
   // LastTime, CurrentTime, ReturnVal
   unsigned int WallAngle = 1000;
   static unsigned char CurrentZone = 2;
   static unsigned int Angles_Array[4] = ANGLES;
   ES_Event ThisEvent;
   static uint16_t LastTime = 0;
   uint16_t CurrentTime = 0;
   boolean ReturnVal = False;
   
   // Set CurrentTime to the current time
   CurrentTime = ES_Timer_GetTime();
   
   // If we are in Defending mode and Waiting/Reseting/PushingForward/PushingBackward, and enough time has elapsed since we last queried the wall position
   if ((QueryMasterMachine() == Defending) && (QueryDefendingSM() == Waiting || QueryDefendingSM() == Reseting || QueryDefendingSM() == PushingForward || QueryDefendingSM() == PushingBackward) && ((CurrentTime-LastTime) > 50))
   {
      // Query the angle of the wall
      WallAngle = Get_WallAngle();

      if (WallAngle == 999)
      {
         // We got a bad command from SPI
         return False; // immediately quit the event checker
      }
      // Else
      else
      {
         // If the wall entered the danger zone from the right
         if (WallAngle > DangerAngle_Right && WallAngle <= Angles_Array[MyBin - 1])
         {
            // If the CurrentZone was not ZONE_DANGER already
            if (CurrentZone != ZONE_DANGER)
            {
                // Post Event DANGERWALL_RIGHT
               ThisEvent.EventType = ES_DANGERWALL_RIGHT; // adl: added ES_
               PostMasterMachine(ThisEvent);
               // Set CurrentZone to ZONE_DANGER
               CurrentZone = ZONE_DANGER;
               // Set ReturnVal to True
               ReturnVal = True;
            }
         }
         // Else if the wall entered the danger zone from the left
         else if (WallAngle < DangerAngle_Left && WallAngle >= Angles_Array[MyBin - 1])
         {
            // If the CurrentZone was not ZONE_DANGER already
            if (CurrentZone != ZONE_DANGER)
            {
                // PostEvent DANGERWALL_LEFT
               ThisEvent.EventType = ES_DANGERWALL_LEFT; // adl: added ES_
               PostMasterMachine(ThisEvent);
               // Set CurrentZone to ZONE_DANGER
               CurrentZone = ZONE_DANGER;
               // Set ReturnVal to True
               ReturnVal = True;
            }
         }
         // Else if the Wall entered in the safe zone and the CurrentZone was not ZONE_SAFE already
         else if ((WallAngle < SafeAngle_Right || WallAngle > SafeAngle_Left) && CurrentZone != ZONE_SAFE)
         {
            // Post Event NODANGERWALL
            ThisEvent.EventType = ES_NO_DANGERWALL; // adl: added ES_
            PostMasterMachine(ThisEvent);
            // Set CurrentZone to ZONE_SAFE
            CurrentZone = ZONE_SAFE;
            // Set ReturnVal to True
            ReturnVal = True; 
         }
      }
      LastTime = CurrentTime; // update the last time
   }
   // Return ReturnVal
   return ReturnVal;
}