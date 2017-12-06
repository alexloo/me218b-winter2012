/****************************************************************************
   Description
         FSR.c is the source file for the custom made communication firmware.

 History
 When           Who	What/Why
 -------------- ---	--------
 02/07/12 19:13 adl  First pass
 03/11/12 11:34 kfn  Removal of ifndef DEBUG_SYNC code (only final code)
****************************************************************************/

// Includes ****************************************************************/
#include <hidef.h>          /* common defines and macros */
#include <mc9s12e128.h>     /* derivative information */
#include <S12e128bits.h>    /* bit definitions  */
#include <Bin_Const.h>       
#include "FSR.h"
#include "ES_Timers.h"
#include <stdio.h>

// Module Defines **********************************************************/
//#define DEBUG   /* used for debugging */
#define QUERY_BALL 0xFD //Command to query balls on playing field initially
#define QUERY_BIN1 0xFC //Command to query balls in bin 1
#define QUERY_BIN2 0xFB //Command to query balls in bin 2
#define QUERY_BIN3 0xFA //Command to query balls in bin 3
#define QUERY_BIN4 0xF9 //Command to query balls in bin 4
#define QUERY_WALL 0xF8 //Command to query wall angle
#define SYNC_BYTE  0xFD //Syncing byte value
#define SEND_DATA  0x00 //Command to send data to FSR

#define _52MS 26  // Debounce for query commands
#define _4MS 2    // Debounce for between query commands

// Module Private Functions ************************************************/
static unsigned char PingSPI(unsigned char Command);
static void SyncFirstByte(unsigned char Command);  //Syncs first byte to read FSR correctly

// Module Variables ********************************************************/
// unsigned int counter = 0; /* used for debugging */
static uint16_t LastTime = 0; // uint16 LastTime for last query time
static uint16_t LastSecondByte = 0; //uint16_t LastSecondByte for last time of second byte command

// Module Code *************************************************************/
/***************************************************************************
Function
   SPI_Init
   
Parameters
   None
   
Returns
   None
   
Description
   This function initializes the spi communication module

****************************************************************************/
void SPI_Init(void)
{
   // Set the baud rate.
   SPIBR = _S12_SPPR2|_S12_SPPR1|_S12_SPPR0|_S12_SPR2|_S12_SPR1|_S12_SPR0;
   // Initialize the clock. Want mode 3 control:
   SPICR1 |= _S12_CPOL|_S12_CPHA;   // Active low and sample even edges
   // Enable as the master.
   SPICR1 |= _S12_MSTR;
   // SS Control:
   SPICR1 |= _S12_SSOE; // Enable slave select line
   SPICR2 |= _S12_MODFEN; // Enable mode fault
   // Enable SPI.
   SPICR1 |= _S12_SPE;
}

// Module Code ***************************************************
/****************************************************************************
Function
   Get_BallsInPlay 
     
Parameters
   none 
   
Returns
   unsigned char   
   
Description
   This function queries the FSR via SPI to return number of balls in play
   
****************************************************************************/
unsigned char Get_BallsInPlay(void)
{
   //Local Variables: Time, Response
   unsigned int Time;
   unsigned char Response;

   //Do
   do 
   {
   	//Call SyncFirstByte to keep querying until sync'd the command (first) byte
		SyncFirstByte(QUERY_BALL);
		//Set Time to current time
		Time = ES_Timer_GetTime();
		//Debounce 2 mS
		while(ES_Timer_GetTime()-Time < 2)
		{}	
		//Set Response to second command byte from PingSPI with parameter SEND_DATA
		Response = PingSPI(SEND_DATA);
		//Update LastTime to current time
		LastTime = ES_Timer_GetTime();
   } //While get a non-valid response 
   while ((Response < 0x80) || (Response > 0xF8));
    
   //Return Response - 0x80
	return (Response - 0x80);
}  //End of Get_BallsInPlay

/****************************************************************************
Function
   Get_BallsInBin 
     
Parameters
   char BinNumber 
   
Returns
   unsigned char  
   
Description
   This function queries the FSR via SPI to return number of balls in a bin
   
****************************************************************************/
unsigned char Get_BallsInBin(char BinNumber)
{
   //Local Variables: Time, BinCommand = QUERY_BIN1, Response
   unsigned int Time;
   unsigned char BinCommand = QUERY_BIN1;
   unsigned char Response;

   //Choose one of the following cases based on BinNumber
   switch (BinNumber)
   {
      // 1
      case 1:
         //Set BinCommand to QUERY_BIN1
         BinCommand = QUERY_BIN1;
      break;
      // 2
      case 2:
         //Set BinCommand to QUERY_BIN2
         BinCommand = QUERY_BIN2;
      break;
      // 3
      case 3:
         //Set BinCommand to QUERY_BIN3
         BinCommand = QUERY_BIN3;
      break;
      // 4
      case 4:
         //Set BinCommand to QUERY_BIN4
         BinCommand = QUERY_BIN4;
      break;   
   } //End of case structure on BinNumber
   
   //Do
   do 
   {
   	//Call SyncFirstByte to keep querying until sync'd the command (first) byte
		SyncFirstByte(BinCommand);
		//Set Time to current time
		Time = ES_Timer_GetTime();
		//Debounce 2 mS
		while(ES_Timer_GetTime()-Time < 2)
		{}	
		//Set Response to second command byte from PingSPI with parameter SEND_DATA
		Response = PingSPI(SEND_DATA);
		//Update LastTime to current time
		LastTime = ES_Timer_GetTime();
   } //While get a non-valid response
   while ((Response < 0x01) || (Response > 0xF1));
   //Return Response - 1, the actual number of balls in bin
   return (Response - 1);
}  //End of Get_BallsInBin

/****************************************************************************
Function
   Get_WallAngle  
    
Parameters
   None
   
Returns
   unsigned int  
    
Description
   This function queries the FSR via SPI to return the current wall angle
   
****************************************************************************/
unsigned int Get_WallAngle(void)
{
   //Local Variables: Time, Response
   unsigned int Time;
   unsigned char Response;
   
   //Do
   do 
   {
   	//Call SyncFirstByte to keep querying until sync'd the command (first) byte
		SyncFirstByte(QUERY_WALL);
		//Set Time to current time
		Time = ES_Timer_GetTime();
		//Debounce 2 mS
		while(ES_Timer_GetTime()-Time < 2)
		{}	
		//Set Response to second command byte from PingSPI with parameter SEND_DATA
		Response = PingSPI(SEND_DATA);
		//Update LastTime to current time
		LastTime = ES_Timer_GetTime();
   } //While get a non-valid response
   while ((Response < 0x01) || (Response > 0xB4));

   //Return wall angle conversion
   return 2*((unsigned int)Response-1);
}  //End of Get_WallAngle

/****************************************************************************
Function
   PingSPI
      
Parameters
   unsigned char Command  
   
Returns
   unsigned char Response  
    
Description
   This function queries the FSR via SPI
   
****************************************************************************/
static unsigned char PingSPI(unsigned char Command)
{
   //Local variable: Response, Time
   unsigned char Response = 0xFE;
   unsigned int Time;  
     
   //If Slave has an empty transmit register
   if ((SPISR & _S12_SPTEF) != 1)
   {
      // Write Query_Command to the SPIDR to transmit new data
      SPIDR = Command;
   }  //Endif           
   
   //Set Time to current time
   Time = ES_Timer_GetTime();
   //Debounce for 2 mS
   while(ES_Timer_GetTime()-Time<2)
   {}  
   
   //If there is currently a command waiting to be read
   if ((SPISR & _S12_SPIF) != 0)
   {
      //Store what is in the SPIDR register to Response
      Response = SPIDR;
   } //Endif         
   
   // Return Response
   return Response;
}  //End of PingSPI

/****************************************************************************
Function
   SyncFirstByte 
     
Parameters
   unsigned char Command 
    
Returns
   unsigned char Response 
    
Description
   This sends the first command query byte to the FSR until it is sync'd. 
                                                                               
****************************************************************************/
static void SyncFirstByte(unsigned char Command)
{
   //Local Variables: CurrentTime, LastTimeSync, Response
   uint16_t CurrentTime = 0; 
   uint16_t LastTimeSync = 0;
   unsigned char Response = 0xFE;
   
   //Do
   do
   {
      //Set CurrentTime to current time
      CurrentTime = ES_Timer_GetTime();
      //If at least 50 ms have elapsed since the last time the FSR has been queried
      if((CurrentTime - LastTimeSync) > 26)
      {
         //Set Response to value of function PingSPI with parameter Command
         Response = PingSPI(Command);
         //Update LastTimeSync to the current time
         LastTimeSync = ES_Timer_GetTime();  
      } //Endif
   } //While not synced
   while (Response != SYNC_BYTE);
}  //End of SyncFirstByte