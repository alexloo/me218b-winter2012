/****************************************************************************
 Description
      BeaconDetection.h is the header file for the BeaconDetection.c module

 History
 When           Who	What/Why
 -------------- ---	--------
 02/07/12 19:13 adl  First pass
****************************************************************************/
#ifndef BEACONDETECTION_H
#define BEACONDETECTION_H

// defines
#define PERIOD_1 20
#define PERIOD_2 18
#define PERIOD_3 16
#define PERIOD_4 14

// Function prototypes
void BeaconDetection_Init( void );
unsigned char GetBeaconRear( void );
unsigned char GetBeaconFront ( void );

#endif
