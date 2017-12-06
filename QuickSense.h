/****************************************************************************
Description
         QuickSense.h is the header file for the custom sensor control 
         module. All sensors contained in this module are interrupt based.
         The sensors included are:
         -  Ball counter (PT0)
         -  Left tape sensor (PT1)
         -  Right tape sensor (PT2)
         -  Front bumper limit switch (PT3)
         -  Rear bumper limit switch (PT4)

 History
 When           Who	What/Why
 -------------- ---	--------
 02/21/12 14:53 adl  First pass
****************************************************************************/

#ifndef QUICKSENSE_H
#define QUICKSENSE_H

// Defines

// Function prototypes
void QS_Initialize(void);
unsigned char QS_QueryBallCount(void);
unsigned int QS_QueryTIM0Overflow(void);
unsigned int QS_QueryTIM1Overflow(void);

#endif