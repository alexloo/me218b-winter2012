/****************************************************************************
 
  Header file for template service 
  based on the Gen 2 Events and Services Framework

 ****************************************************************************/

#ifndef MasterMachine_H
#define MasterMachine_H

#include "ES_Types.h"

// Public Function Prototypes

boolean InitMasterMachine ( uint8_t Priority );
boolean PostMasterMachine( ES_Event ThisEvent );
ES_Event RunMasterMachine( ES_Event ThisEvent );


#endif /* MasterMachine_H */

