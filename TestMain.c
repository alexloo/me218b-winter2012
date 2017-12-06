/****************************************************************************
   Description
         TestMain.c is the source file with the main function
 History
 When           Who	What/Why
 -------------- ---	--------
 02/06/12 19:13 kfn  Changed for Lab8 use
****************************************************************************/
// Includes ****************************************************************/
#include <stdio.h>
#include "ES_Configure.h"
#include "ES_Framework.h"
#include "ES_Timers.h"
// Module Defines **********************************************************/
// Module Private Functions ************************************************/
// Module Variables ********************************************************/
// Module Code *************************************************************/
void main( void)
{
    //Local variable: ES_Return_t ErrorType
    ES_Return_t ErrorType;

    puts("\n\rStarting Team 16 HIPPOS State Machine.");
    puts("\n\rUsing E128 as well as ES framework logic.");

    // Initialize the Events and Services Framework and timer rate
    ErrorType = ES_Initialize(ES_Timer_RATE_2MS);
    // If there is no error
    if ( ErrorType == Success )
        {
            //Run ES State Machine
            ErrorType = ES_Run();
        }  //End if
    //If we got to here, there was an error
    switch (ErrorType)
        {
        case FailedPointer:
            puts("Failed on NULL pointer");
            break;
        case FailedInit:
            puts("Failed Initialization");
            break;
        default:
            puts("Other Failure");
            break;
        }
    for (;;)
        ;
};
