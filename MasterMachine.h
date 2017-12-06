/****************************************************************************
 
  Header file for template Flat Sate Machine 
  based on the Gen2 Events and Services Framework

 ****************************************************************************/

#ifndef MasterMachine_H
#define MasterMachine_H

// Event Definitions
#include "ES_Configure.h"
#include "ES_Types.h"

// typedefs for the states
// State definitions for use with the query function
typedef enum { InitPState, UnlockWaiting, _1UnlockPress, 
               _2UnlockPresses, Locked, 
               PreGame,
               SideIdentification,
               Gathering,
               Scoring, 
               Defending,
               GameOver} MasterMachineState_t ;


// Public Function Prototypes

boolean InitMasterMachine ( uint8_t);
boolean PostMasterMachine( ES_Event);
void StartMasterMachine(ES_Event);
ES_Event RunMasterMachine( ES_Event);
MasterMachineState_t QueryMasterMachine ( void );


#endif /* MasterMachine_H */

