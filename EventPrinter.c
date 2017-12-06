#include "ES_Configure.h"  /* get the typedefs for the components of an event*/
#include "ES_Framework.h"
#include <hidef.h>         /* common defines and macros */
#include <mc9s12e128.h>     /* derivative information */
#include <S12e128bits.h>    /* bit definitions  */
#include <Bin_Const.h>
#include <stdio.h>			/* standard i/o for print screens */
#include "EventPrinter.h"

void EventPrinter (ES_Event CurrentEvent)
{
   switch (CurrentEvent.EventType)
   {
      case ES_NO_EVENT:
         printf("\r\nThe current event is ES_NO_EVENT");
      break;
      
      case ES_ERROR:
         printf("\r\nThe current event is ES_ERROR");
      break;
      
      case ES_INIT:
         printf("\r\nThe current event is ES_INIT");
      break;
      
      case ES_NEW_KEY:
         printf("\r\nThe current event is ES_NEW_KEY");
      break;
      
      case ES_TIMEOUT:
         printf("\r\nThe current event is ES_TIMEOUT");
         switch(CurrentEvent.EventParam)
         {
            case 0:
               printf("\r\nGAME OVER clock expired");
            break;
            
            case 1:
               printf("\r\nMOVE TO SCORING clock expired");
            break;
            
            case 2:
               printf("\r\nMOTION clock expired");
            break;
            
            case 3:
               printf("\r\nSHUFFLE clock expired");
            break;
            
            case 4:
               printf("\r\nSHUFFLE STEP clock expired");
            break;
         }
      break;
      
      case ES_ENTRY:
         printf("\r\nThe current event is ES_ENTRY");
      break;
      
      case ES_EXIT:
         printf("\r\nThe current event is ES_EXIT");
      break;
      
      case ES_LEFT_TAPE_DETECTED:
         printf("\r\nThe current event is ES_LEFT_TAPE_DETECTED");
      break;
      
      case ES_RIGHT_TAPE_DETECTED:
         printf("\r\nThe current event is ES_RIGHT_TAPE_DETECTED");
      break;
      
      case ES_FRONT_BUMPED:
         printf("\r\nThe current event is ES_FRONT_BUMPED");
      break;
      
      case ES_REAR_BUMPED:
         printf("\r\nThe current event is ES_REAR_BUMPED");
      break;
      
      case ES_GAME_START:
         printf("\r\nThe current event is ES_GAME_START");
      break;
      
      case ES_BEACON_FRONT:
         printf("\r\nThe current event is ES_BEACON_FRONT");
         switch(CurrentEvent.EventParam)
         {
            case 1:
               printf("\r\nBeacon 1 is seen.");
            break;
            
            case 2:
               printf("\r\nBeacon 2 is seen.");
            break;
            
            case 3:
               printf("\r\nBeacon 3 is seen.");
            break;
            
            case 4:
               printf("\r\nBeacon 4 is seen.");
            break;
            
            case 0:
               printf("\r\nFRONT beacon lost.");
            break;
         }
      break;
      
      case ES_BEACON_REAR:
         printf("\r\nThe current event is ES_BEACON_REAR");
         switch(CurrentEvent.EventParam)
         {
            case 1:
               printf("\r\nBeacon 1 is seen.");
            break;
            
            case 2:
               printf("\r\nBeacon 2 is seen.");
            break;
            
            case 3:
               printf("\r\nBeacon 3 is seen.");
            break;
            
            case 4:
               printf("\r\nBeacon 4 is seen.");
            break;
            
            case 0:
               printf("\r\nREAR beacon lost.");
            break;
         }
      break;
      
      case ES_BALL_BIN_EMPTY:
         printf("\r\nThe current event is ES_BALL_BIN_EMPTY");
      break;
      
      case ES_DANGERWALL_RIGHT:
         printf("\r\nThe current event is ES_DANGERWALL_RIGHT");
      break;
      
      case ES_DANGERWALL_LEFT:
         printf("\r\nThe current event is ES_DANGERWALL_LEFT");
      break;
      
      case ES_NO_DANGERWALL:
         printf("\r\nThe current event is ES_NO_DANGERWALL");
      break;
      
      
   } // end switch
}