/****************************************************************************
Description
         SideID.h is the header file for the custom module that identifies
         which side that the robot is starting on.
         
 History
 When           Who	What/Why
 -------------- ---	--------
 02/22/12 16:29 adl  First pass
****************************************************************************/

#ifndef SIDEID_H
#define SIDEID_H

// Defines
#define BS_BEACON1 1
#define BS_BEACON2 2
#define BS_BEACON3 3
#define BS_BEACON4 4
// may want to move the above 4 defines to the beacon sensor module

#define BLUE_LED_PIN BIT2HI
#define RED_LED_PIN BIT0HI

#define BLUE_TEAM 2
#define RED_TEAM 0

// Function prototypes
void ID_Initialize(void);
void ID_IdentifySide(unsigned char);
unsigned char ID_QuerySide(void);
void ID_AllLights(void);

#endif