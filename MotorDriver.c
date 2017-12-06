/****************************************************************************


 Description
         MotorDriver.c is the source file for the custom made motor driving
         firmware.

 History
 When           Who	What/Why
 -------------- ---	--------
 02/04/12 23:50 adl  First pass
****************************************************************************/

// Includes ******************************************************
#include <hidef.h>         /* common defines and macros */
#include <mc9s12e128.h>     /* derivative information */
#include <S12e128bits.h>    /* bit definitions  */
#include <Bin_Const.h>
#include "PWM.h"
#include "MotorDriver.h"
#include "TimingConstants.h"
#include <stdio.h>

// Module Defines ************************************************
//#define DEBUG
#define MAX_RPM  86 // Empirically measured maximum RPM
#define LEFT_ENCODER_PIN BIT5HI
#define RIGHT_ENCODER_PIN BIT6HI
#define CONTROLLER_TIMER_PIN BIT0HI
#define PERCENT_DIFFERENCE 4

// Module Private Functions **************************************
static unsigned char Speed2Duty(unsigned char);
static unsigned int PulsePeriodToShaftRPM(unsigned int);

// Module Variables **********************************************
// unsigned int counter = 0;  //used for debugging
#ifdef FEEDBACK //if we used encoders, variables local to this module: Desired Speed, MeasuredSpeed_Left, MeasuredSpeed_Right, uPeriod_Left, uPeriod_Right
unsigned char DesiredSpeed = 0; // Desired speed as a percentage of max shaft RPM
unsigned char MeasuredSpeed_Left = 0; // Measured speed as a percentage of max shaft RPM
unsigned char MeasuredSpeed_Right = 0; // Measured speed as a percentage of max shaft RPM

static unsigned int uPeriod_Left = 0;
static unsigned int uPeriod_Right = 0;
#endif

// Module Code ***************************************************
/****************************************************************************
Function
   GoForward
   
Parameters
   unsigned char Speed: A percentage of maximum speed, form 0 to 100
   
Returns
   None
   
Description
   This function drives both motors forward at the desired speed.

****************************************************************************/
void GoForward(unsigned char Speed)
{
	//Local Variables: DutyCyle_Right, DutyCycle_Left
	unsigned char DutyCycle_Right;
	unsigned char DutyCycle_Left;
	//Call function FullStop
	FullStop();
	// Set Ports U6 and U7 LOW (forward)
	PTU &= ~(BIT6HI|BIT7HI);
	// Convert the desired speed to a duty cycle based on motor mapping and set to DutyCycle_Left
	DutyCycle_Left = Speed2Duty(Speed);
	// Set the DutyCycle_Right to slightly scaled
	DutyCycle_Right = DutyCycle_Left - (unsigned char)((PERCENT_DIFFERENCE*(unsigned int)DutyCycle_Left)/100);	
	// Set converted duty cycle on motors
	PWM_SetDuty(DutyCycle_Left, _LEFT);
	PWM_SetDuty(DutyCycle_Right, _RIGHT);
}  //End of GoForward

/****************************************************************************
Function
   GoBackward
   
Parameters
   unsigned char Speed: A percentage of maximum speed, form 0 to 100
   
Returns
   None
   
Description
   This function drives both motors backward at the desired speed.

****************************************************************************/
void GoBackward(unsigned char Speed)
{
	//Local Variables: DutyCyle_Right, DutyCycle_Left
	unsigned char DutyCycle_Right;
	unsigned char DutyCycle_Left;
	//Call function FullStop
	FullStop();
	// Set Ports U6 and U7 HI (backward)
	PTU |= BIT6HI|BIT7HI;
	// Convert the desired speed to a duty cycle based on motor mapping and set to DutyCycle_Left
	DutyCycle_Left = Speed2Duty(Speed);
	// Set the DutyCycle_Right to slightly scaled
	DutyCycle_Right = DutyCycle_Left - (unsigned char)((PERCENT_DIFFERENCE*(unsigned int)DutyCycle_Left)/100);	
	// Set converted duty cycle on motors
	PWM_SetDuty(DutyCycle_Left, _LEFT);
	PWM_SetDuty(DutyCycle_Right, _RIGHT);
}  //End of GoBackward

/****************************************************************************
Function
   TurnLeft
   
Parameters
   None
   
Returns
   None
   
Description
   This function turns the robot left by driving the left motor backward and 
   the right motor forward at 100% DC.

****************************************************************************/
void TurnLeft(void)
{
	//Call function FullStop
	FullStop();
	// Set U6 to be high (backward)
	PTU |= BIT6HI;
	// Set U7 to be low (forward)
	PTU &= ~BIT7HI;
	// Set 100% duty cycle on both motors
	PWM_SetDuty(MAX_DUTY, _LEFT);
	PWM_SetDuty(MAX_DUTY, _RIGHT);
}  //End of TurnLeft

/****************************************************************************
Function
   TurnLeftSpeedSelect
   
Parameters
   Speed
   
Returns
   None
   
Description
   This function turns the robot left by driving the left motor backward and 
   the right motor forward at a selected speed.

****************************************************************************/
void TurnLeftSpeedSelect(unsigned char Speed)
{
   //Local Variable: DutyCycle
   unsigned char DutyCycle;
   //Call function full stop
   FullStop();
   // Set U6 to be high (backward)
	PTU |= BIT6HI;
	// Set U7 to be low (forward)
	PTU &= ~BIT7HI;
	// Convert the desired speed to a duty cycle based on motor mapping and set to DutyCycle
	DutyCycle = Speed2Duty(Speed);      
	// Set converted duty cycle on motors
	PWM_SetDuty(DutyCycle, _LEFT);
	PWM_SetDuty(DutyCycle, _RIGHT);
}  //End of TurnLeftSpeedSelect

/****************************************************************************
Function
   TurnRight
   
Parameters
   None
   
Returns
   None
   
Description
   This function turns the robot right by driving the left motor foward and 
   the right motor backward at 100 %.

****************************************************************************/
void TurnRight(void)
{
	//Call function FullStop
	FullStop();
	// Set U6 to be low (forward)
	PTU |= BIT6HI;
	// Set U7 to be high (backward)
	PTU &= ~BIT7HI;
	// Set 100% duty cycle on both motors
	PWM_SetDuty(MAX_DUTY, _LEFT);
	PWM_SetDuty(MAX_DUTY, _RIGHT);
}  //End of TurnRight

/****************************************************************************
Function
   TurnRightSpeedSelect
   
Parameters
   Speed
   
Returns
   None
   
Description
   This function turns the robot right by driving the left motor foward and 
   the right motor backward at a selected speed.

****************************************************************************/
void TurnRightSpeedSelect(unsigned char Speed)
{
   //Local Variable: DutyCycle
   unsigned char DutyCycle;
   //Call function full stop
   FullStop();
   // Set U6 to be low (forward)
	PTU |= BIT6HI;
	// Set U7 to be high (backward)
	PTU &= ~BIT7HI;
	// Convert the desired speed to a duty cycle based on motor mapping and set to DutyCycle
	DutyCycle = Speed2Duty(Speed);      
	// Set converted duty cycle on motors
	PWM_SetDuty(DutyCycle, _LEFT);
	PWM_SetDuty(DutyCycle, _RIGHT);
}  //End of TurnRightSpeedSelect

/****************************************************************************
Function
   FullStop
   
Parameters
   None
   
Returns
   None
   
Description
   This function stops the robot.

****************************************************************************/
void FullStop(void)
{	
	// Set 0% duty cycle on both motors
	PWM_SetDuty(0, _LEFT);
	PWM_SetDuty(0, _RIGHT);
}  //End of FullStop


/****************************************************************************
Function
   MotorDriver_Init
   
Parameters
   None
   
Returns
   None
   
Description
   This function initializes the motor driver module. It also initializes 
   the PWM sub-module.

****************************************************************************/
void MotorDriver_Init(void)
{
   // Initialize the PWM module
   PWM_Init(PWM_30_kHz);
   // Set PWMPOL such that output is initially high for U0 and U1
	PWMPOL |= _S12_PPOL0 | _S12_PPOL1;
   // Set Ports U6 and U7 to write
   DDRU |= BIT6HI|BIT7HI;
   // Write 0 to Ports U6 and U7 
   PTU &= ~(BIT6HI|BIT7HI);
   
   #ifdef FEEDBACK
   // Now initialize the timer modules necesary for measuring the encoder 
   // speed and running the speed controller.
   // Turn the timer system on for Timer 0 and Timer 1
   TIM0_TSCR1 = _S12_TEN;
   TIM1_TSCR1 = _S12_TEN;
   // Set the pre-scale to divide by 8 = 3 MHz, 3 scaled clock pulses = 1 us
   TIM0_TSCR2 = _S12_PR1|_S12_PR0;
   TIM1_TSCR2 = _S12_PR1|_S12_PR0;
   
   // Set output compare for the motor controller (Timer 0, channel 4 --> PT0)
   TIM0_TIOS = _S12_IOS4;
   // Set no pin connected to output compare on channel 4
   TIM0_TCTL1 &= ~(_S12_OL4 | _S12_OM4); 
   // Schedule the firt interrupt 20 ms from now for channel 4
   TIM0_TC4 = TCNT + 20 _MS_;
   // Clear the OC4 flag
   TIM0_TFLG1 = _S12_C4F;
   // Enable OC4 interrupt
   TIM0_TIE |= _S12_C4I;
   // End output compare for Timer 0, channel 4 set up
   
   // Set up to capture rising edges for the motor encoders on PT5 and PT6
   // These map to Timer 1, Channels 5 and 6, respectively
   // Set to capture rising edges only on channels 5 and 6
   TIM1_TCTL3 |= _S12_EDG5A|_S12_EDG6A;
   // Clear flags
   TIM1_TFLG1 = _S12_C5F; // clear flag for channel 5
   TIM1_TFLG1 = _S12_C6F; // clear flag for channel 6
   // Enable interrupts for channels 5 and 6 on Timer 1
   TIM1_TIE |= _S12_C5I|_S12_C6I;
   // Enable global interrupts
   EnableInterrupts;
   // End input interrupts for Timer 1, channels 5 and 6 set up
   #endif
   
   printf("\r\nMotor control module initialization complete.\n"); 
    
} //End of MotorDriver_Init

/****************************************************************************
Function
   Speed2Duty
   
Parameters
   DesiredSpeed (0 - 100. A percent of max speed)
   
Returns
   DutyCycle
   
Description
   This function calculates the duty cycle setting based on the DesiredSpeed.
   The conversion is made per the data gathered from motor mapping. Optional:
   may need to have different mapping functions for the two motors.

****************************************************************************/
static unsigned char Speed2Duty(unsigned char DesiredSpeed)
{
	//Local Variable: DutyCycle
	unsigned char DutyCycle;
	
	// If Desired Speed is greater than max (100)
	if (DesiredSpeed > 100)
	   //Set Desired Speed to max (100)
	   DesiredSpeed = 100;
	//Endif
	
	// Map the requested speed to DutyCycle
	DutyCycle = (unsigned char) ((70 * (unsigned int)DesiredSpeed + 3000)/100);
	//Return DutyCycle
	return DutyCycle;
}  //End of Speed2Duty

#ifdef FEEDBACK //if we used encoders
/****************************************************************************
Function
   PulsePeriodToShaftRPM
   
Parameters
   PulsePeriod
   
Returns
   ShaftRPM
   
Description
   This function calculates the shaft speed (in RPM) based on the encoder's 
   pulse period.

****************************************************************************/
static unsigned int PulsePeriodToShaftRPM(unsigned int PulsePeriod)
{
   //Local Variables: ShaftRPM, denominator = 0;
   //Local Constant: numerator
   unsigned int ShaftRPM;
   const unsigned long numerator = (60 * 10 _SEC_);
   unsigned long denominator = 0;
   
   //Set denominator to PulsePeriod*Pulses Per Rev * Encoder Revs per shaft rev (lab 8)
   denominator = (unsigned long) PulsePeriod * PULSES_PER_REVOLUTION * ENCODER_REVS_PER_SHAFT_REV;
   //Set ShaftRPM to numerator/denominator
   ShaftRPM = (unsigned int) (numerator/denominator);
   //Return ShaftRPM
   return ShaftRPM  
}  //End of PulsePeriodToShaftRPM

/***************************************************************************
 private interrupt functions
 ***************************************************************************/
 /****************************************************************************
Interrupt
   LeftMotorSpeed (PT5 --> Timer 1, Channel 5)
   
Parameters
   None
   
Returns
   None
   
Description
   This interrupt service saves the period from the left encoder.

****************************************************************************/  
void interrupt _Vec_tim1ch5 LeftMotorSpeed(void)
{
   //Function Static Variable: uLastEdge
   static unsigned int uLastEdge;
   
   //Disable interrupts
   DisableInterrupts;
   //Calculate encoder period, in clock pulses and set to uPeriod_Left  
   uPeriod_Left = TIM1_TC5 - uLastEdge;
   // Update uLastEdge
   uLastEdge = TIM1_TC5; 
   //Clear flag for channel 5
   TIM1_TFLG1 = _S12_C5F;
   // Calculate the shaft revolutions from encoder period and set to MeasuredSpeed_Left
   MeasuredSpeed_Left = PulsePeriodToShaftRPM(uPeriod_Left);
   //Enable interrupts
   EnableInterrupts;
}  //End of LeftMotorSpeed
 /****************************************************************************
Interrupt
   RightMotorSpeed (PT6 --> Timer 1, Channel 6)
   
Parameters
   None
   
Returns
   None
   
Description
   This interrupt service saves the period from the right encoder.

****************************************************************************/  
void interrupt _Vec_tim1ch6 RightMotorSpeed(void)
{
   //Function Static Variable: uLastEdge
   static unsigned int uLastEdge;
   
   //Disable interrupts
   DisableInterrupts;
   //Calculate encoder period, in clock pulses and set to uPeriod_Right
   uPeriod_Right = TIM1_TC6 - uLastEdge;
   // Update uLastEdge
   uLastEdge = TIM1_TC6; 
   //Clear flag for channel 6
   TIM1_TFLG1 = _S12_C6F;
   // Calculate the shaft revolutions from encoder period and set to MeasuredSpeed_Right
   MeasuredSpeed_Right = PulsePeriodToShaftRPM(uPeriod_Right);
   //Endable interrupts
   EnableInterrupts;
}  //End of RightMotorSpeed

 /****************************************************************************
Interrupt
   PI_Controller 
   
Parameters
   None
   
Returns
   None
   
Description
   This interrupt service saves the period from the right encoder.

****************************************************************************/ 
void interrupt _Vec_tim0ch4 PI_Controller(void)
{
   // BEGIN CONTROLLER VARIABLE DEFINITIONS
   static float iState = 0; // Integrator state
   // Maximum and minimum allowable integrator state
   const float iMax = 100; 
   const float iMin = -100; 
   
   const float iGain = 0.4; // Integral gain
   const float pGain = 2.4; // Proportional gain
   
   static float pTerm, iTerm;
   static float error; 
   static float ControllerRPM;
   static float ReferenceDuty;
   // END CONTROLLER VARIABLE DEFINITIONS
   
   //Disable interrupts
   DisableInterrupts;
   // Clear OC4 flag
   TIM0_TFLG1 = _S12_C4F; 
   // Program next compare
   TIM0_TC4 += 20_MS_; 
   //Enable interrupts
   EnableInterrupts;
   
   // BEGIN CONTROLLER MODULE
   //Calculate the error
   error = (float)DesiredRPM - (float)MeasuredRPM;
   // Calculate the proportional term
   pTerm = pGain * error;
   // Calculate the integral state
   iState += error; 
   //Check the integral term if it exceeds the bounds
   if (iState > iMax)
   {
     iState = iMax;
     iState -= error; 
   } else if (iState < iMin)
   {
      iState = iMin;
      iState -= error;
   }  //Endif
      
   // Calculate the integral term and set to iTerm
   iTerm = iGain * iState; 
}  //End of PI_Controller
#endif