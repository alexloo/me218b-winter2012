/****************************************************************************
 
  Header file for template Flat Sate Machine 
  based on the Gen2 Events and Services Framework

 ****************************************************************************/

#ifndef GatheringSM_H
#define GatheringSM_H

// Event Definitions
#include "ES_Configure.h"
#include "ES_Types.h"

// typedefs for the states
// State definitions for use with the query function
typedef enum { FullSpeedAhead, HalfSpeedAhead, 
               TurningLeft, TurningRight,
               FullReverse, HalfReverse} GatheringState_t ;

// Public Function Prototypes

// boolean InitGatheringSM ( uint8_t);
//boolean PostGatheringSM( ES_Event);
void StartGatheringSM(ES_Event);
ES_Event RunGatheringSM( ES_Event);
GatheringState_t QueryGatheringSM ( void );


#endif /* GatheringSM_H */