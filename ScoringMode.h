/****************************************************************************
Description
         ScoringSMHelperFunctions.h is the source file for the custom sensor control 
         module. All sensors contained in this module are interrupt based.
         The sensors included are:

 History
 When           Who	What/Why
 -------------- ---	--------
 02/21/12 14:53 adl  First pass
****************************************************************************/

#ifndef SCORINGMODE_H
#define SCORINGMODE_H

// Event Definitions
#include "ES_Configure.h"
#include "ES_Types.h"

// Define the angles bounding each bin
#define ANGLE_A 32
#define ANGLE_B 58
#define ANGLE_C 122
#define ANGLE_D 148
#define ANGLE_E 212
#define ANGLE_F 238
#define ANGLE_G 302
#define ANGLE_H 328

#define MIN_ANGLE 0
#define MAX_ANGLE 360

typedef enum {Full, Partial, Blocked} BinAvailability_t;

typedef struct Bin_t{
   unsigned char ID;
   unsigned char BallsInBin;
   BinAvailability_t BinAvailable; // is bin on our side of the wall
   boolean PartialBin; // is wall partiall in bin zone   
} Bin;


// Public Function Prototypes
void BinsAvailable (void);
unsigned char PickScoringBin (void);


#endif /* SCORINGMODE_H */