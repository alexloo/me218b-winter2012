/****************************************************************************


 Description
         PWM.c is the source file for the custom made PWM functions.

 History
 When           Who	What/Why
 -------------- ---	--------
 01/20/12 12:15 adl  First pass
 03/11/12 12:29 kfn  Removal of BAD_MOTOR stuff
****************************************************************************/

// Includes ******************************************************
#include <hidef.h>         /* common defines and macros */
#include <mc9s12e128.h>     /* derivative information */
#include <S12e128bits.h>    /* bit definitions  */
#include <Bin_Const.h>
#include "PWM.h"
#include <stdio.h>

// Module Defines ************************************************
//#define DEBUG
// Module Variables **********************************************
// unsigned int counter = 0; //used in debugging

// Module Code ***************************************************
/****************************************************************************
Function
   PWM_Init
   
Parameters
   unsigned char DesiredFrequency: a predefined symbol for the frequency
   
Returns
   None
   
Description
   Initializes PWM on Ports U0 and U1. The PWM frequency is based on the 
   value passed to this function. This sets up a 100 clock tick PWM period.

****************************************************************************/
void PWM_Init(unsigned char DesiredFrequency)
{
    // Map Ports U0 and U1 to PWM
    MODRR |= BIT0HI|BIT1HI;    
    // Enable PWM on U0 and U1
    PWME |= _S12_PWME0|_S12_PWME1;
    // Set PWM Output Polarity so that it is initially high
    PWMPOL |= _S12_PPOL0 | _S12_PPOL1;
    // Set PWM Clock Select to use the scale clock A (CLK SA)
    PWMCLK |= _S12_PCLK0 | _S12_PCLK1;
    
    //Choose one of the following based on the value of DesiredRequency
    switch (DesiredFrequency)
    {      
      //PWM_1_kHz
      case PWM_1_kHz :
         // Set pre-scaler to divide by 8
         PWMPRCLK |= _S12_PCKA1|_S12_PCKA0;
         // Set scaler to divide by 30 (15*2)
         PWMSCLA = 15;
      break;
      
      //PWM_4_kHz
      case PWM_4_kHz :
         // Set pre-scaler to divide by 2
         PWMPRCLK |= _S12_PCKA0;
         // Set scaler to divide by 30 (15*2)
         PWMSCLA = 15;
      break;
      
       //PWM_5_kHz
      case PWM_5_kHz :
         // Set pre-scaler to divide by 8
         PWMPRCLK |= _S12_PCKA1|_S12_PCKA0;
         // Set scaler to divide by 6 (3*2)
         PWMSCLA = 3;
      break;
      
      //PWM_750_Hz
      case PWM_750_Hz :
         // Set pre-scaler to divide by 32
         PWMPRCLK |= _S12_PCKA2|_S12_PCKA0;
         // Set scaler to divide by 10 (5*2)
         PWMSCLA = 5;
      break;
      
      //PWM_30_kHz
      case PWM_30_kHz :
         // Set pre-scaler to divide by 4
         PWMPRCLK |= _S12_PCKA1;
         // Set scaler to divide by 2 (1*2)
         PWMSCLA = 1;
      break;
      
      //PWM_60_Hz
      case PWM_60_Hz:
      // Set pre-scaler to divide by 32
         PWMPRCLK |= _S12_PCKA2|_S12_PCKA0;
         // Set scaler to divide by 120 (60*2)
         PWMSCLA = 60;
      break;

      //Default
      default:
         // Set pre-scaler to divide by 2
         PWMPRCLK |= _S12_PCKA0;
         // Set scaler to divide by 30 (15*2)
         PWMSCLA = 15;
         
    }  //End of case structure for DesiredFrequency
    
    // Set the Period to 100 counts of CLKSA for U0 and U1
    PWMPER0 = PERIOD;
    PWMPER1 = PERIOD;
    // Set the Duty Cycle to 0 for U0 and U1
    PWMDTY0 = 0;
    PWMDTY1 = 0;
    
    printf("\r\nPWM initialization of Ports U0 and U1 complete.");
}   //End of PWMInit

/****************************************************************************
Function
   PWM_SetDuty
   
Parameters
   Duty, Side
   
Returns
   None
   
Description
   This function sets the PWM duty cycle for lines U0 or U1. The first argument
   passed is the requested duty cycle for the line, from 0 to 100. The second
   argument indicates the line to set the duty cycle on.
****************************************************************************/
void PWM_SetDuty(unsigned char Duty, unsigned char Side)
{
   //If the requested duty cycle exceeds the maximum duty
   if (Duty > MAX_DUTY)
   {
      //Set Duty to max
      Duty = MAX_DUTY;
   }
 
   //Choose one of the following based on the value of Side:
	switch (Side)
	{
		//_LEFT
		case _LEFT:
			// Set duty cycle for the "left" line (U0) based on the input
			PWMDTY0 = Duty;
		break;
		
		//_RIGHT
		case _RIGHT:
		// Set duty cycle for the "right" line (U1) based on the input
			PWMDTY1 = Duty;
		break;
	 
	}  //End of case structure for DesiredFrequency
   
   #ifdef DEBUG
   if (counter%10 == 0)
   {                   
      printf("\r\nThe duty cycle was set to %u", Duty);                       
   }
   counter++;
   #endif
}  //End of PWM_SetDuty
