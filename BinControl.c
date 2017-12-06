/****************************************************************************
 Description
       BinControl.c is the source file for the custom made communication firmware.

 History
 When           Who	What/Why
 -------------- ---	--------
 02/07/12 19:13 adl  First pass
 03/11/12 11:12 kfn  removal of LockControl (no function)
****************************************************************************/

// Includes ****************************************************************/
#include <hidef.h>          /* common defines and macros */
#include <mc9s12e128.h>     /* derivative information */
#include <S12e128bits.h>    /* bit definitions  */
#include <Bin_Const.h>       
#include "BinControl.h"
#include <stdio.h>

// Module Defines **********************************************************/

// Module Private Functions ************************************************/

// Module Variables ********************************************************/
// unsigned int counter = 0; /* used for debugging */

// Module Code *************************************************************/
/***************************************************************************
Function
   InitFan 
    
Parameters
   None 
    
Returns
   None 
     
Description
   This function initialized PS2 and PS3 to be outputs in the 'E128
   
****************************************************************************/
void InitFan (void)
{
   //Set PS2 and PS3 to output
   DDRS |= (BIT2HI | BIT3HI);
} //End of InitFan

/***************************************************************************
Function
   FanControl 
    
Parameters
   char Control
   
Returns
   None   
   
Description
   This function sets the outputs on the fan to on if parameter is 1 and off
   if parameter is 0
   
****************************************************************************/
void FanControl (char Control)
{
   //If Control is set to 1
   if (Control == 1)
      //Set fan ports on 'E128 high (on)
      PTS |= (BIT2HI | BIT3HI);
   //Else
   else
      //Set fan ports on 'E128 low (off)
      PTS &= ~(BIT2HI | BIT3HI);
   //Endif
} //End of FanControl