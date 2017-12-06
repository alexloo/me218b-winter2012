/****************************************************************************
 
  Header file for template Flat Sate Machine 
  based on the Gen2 Events and Services Framework

 ****************************************************************************/

#ifndef DEFENDINGSM_H
#define DEFENDINGSM_H

// Event Definitions
#include "ES_Configure.h"
#include "ES_Types.h"

// typedefs for the states
// State definitions for use with the query function
typedef enum { DrivingAwayFromWall, AligningPerpendicular, 
               Waiting, Reseting, Realigning,
               PushingForward, PushingBackward} DefendingState_t ;
              
// Public Function Prototypes
// boolean InitGatheringSM ( uint8_t);
//boolean PostScoringSM( ES_Event);
void StartDefendingSM(ES_Event);
ES_Event RunDefendingSM( ES_Event);
DefendingState_t QueryDefendingSM ( void );

#endif /* DEFENDINGSM_H */