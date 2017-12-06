/****************************************************************************


 Description
         PWM.h is the header file for the custom made PWM functions.

 History
 When           Who	What/Why
 -------------- ---	--------
 02/04/12 20:42	adl	 Modified for Lab 8
 01/20/12 12:15 adl  First pass
****************************************************************************/

#ifndef PWM_H
#define PWM_H

// Defines
#define PERIOD 100
#define MAX_DUTY 100

#define PWM_4_kHz 1
#define PWM_1_kHz 2
#define PWM_5_kHz 3
#define PWM_750_Hz 4
#define PWM_30_kHz 5
#define PWM_60_Hz 6

#define _LEFT 1
#define _RIGHT 2
   
//function prototypes for the library
void PWM_Init(unsigned char);
void PWM_SetDuty(unsigned char, unsigned char);

#endif