/****************************************************************************
 Description
         MotorDriver.h is the header file for the custom made MotorDriver.c file.

 History
 When           Who	What/Why
 -------------- ---	--------
 02/04/12 20:42 adl	Modified for Lab 8
 01/20/12 12:15 adl  First pass
****************************************************************************/

#ifndef MOTORDRIVER_H
#define MOTORDRIVER_H
   
// defines
#define PULSES_PER_REVOLUTION 512
#define ENCODER_REVS_PER_SHAFT_REV 141

//function prototypes
void GoForward(unsigned char);
void GoBackward(unsigned char);
void TurnLeft(void);
void TurnLeftSpeedSelect(unsigned char);
void TurnRight(void);
void TurnRightSpeedSelect(unsigned char);
void FullStop(void);
void MotorDriver_Init(void);

#endif