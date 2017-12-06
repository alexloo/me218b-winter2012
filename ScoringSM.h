/****************************************************************************
 
  Header file for template Flat Sate Machine 
  based on the Gen2 Events and Services Framework

 ****************************************************************************/

#ifndef ScoringSM_H
#define ScoringSM_H

// Event Definitions
#include "ES_Configure.h"
#include "ES_Types.h"

// typedefs for the states
// State definitions for use with the query function
typedef enum { AligningFrontBeacon, AligningRearBeacon,
               BackingUp, DrivingForward_Clearance, 
               DrivingForward_Alignment, Unloading,
               Shuffling, BisectingAngle,
               FindingLeftBeacon, FindingRightBeacon} ScoringState_t ;
              
// Public Function Prototypes
// boolean InitGatheringSM ( uint8_t);
//boolean PostScoringSM( ES_Event);
void StartScoringSM(ES_Event);
ES_Event RunScoringSM( ES_Event);
ScoringState_t QueryScoringSM ( void );
unsigned char QueryTargetBin(void);

#endif /* ScoringSM_H */