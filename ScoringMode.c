/****************************************************************************
  Description
         ScoringSMHelperFunctions.c is the source file for the custom sensor control 
         module. All sensors contained in this module are interrupt based.
         The sensors included are:

 History
 When           Who	What/Why
 -------------- ---	--------
 02/21/12 14:53 adl  First pass
****************************************************************************/

// Includes ******************************************************
#include "ES_Configure.h"  /* get the typedefs for the components of an event*/
#include "ES_Framework.h"
#include <hidef.h>         /* common defines and macros */
#include <mc9s12e128.h>     /* derivative information */
#include <S12e128bits.h>    /* bit definitions  */
#include <Bin_Const.h>
#include <stdio.h>			/* standard i/o for print screens */
#include "ES_PostList.h"	/* prototypes for posting events */
#include "TimingConstants.h"
#include "ScoringMode.h"
#include "FSR.h"
#include "SideID.h"

// Module Defines ************************************************
//#define DEBUG

// Module Private Functions **************************************

// Module Variables **********************************************
// Define variables which determine if a bin is available
static Bin Bins[4] = { {1,0,Blocked,False},{2,0,Blocked,False},{3,0,Blocked,False},{4,0,Blocked,False} };

// Module Code ***************************************************

/****************************************************************************
Function
     	BinsAvailable

Parameters
     	None

Returns
		None
		
Description
     	Determines which bins are available based on the wall angle
Notes

Author
     Alex Loo, 2/26/2012, hours no longer hold meaning
****************************************************************************/
void BinsAvailable (void)
{
   // Variables local to the function: WallAngle, TeamColor
   unsigned int WallAngle = Get_WallAngle();
   unsigned char TeamColor = ID_QuerySide();
   
   // Go through different regions of wall angle
   //If region between A and H
   if ((WallAngle > ANGLE_H) || (WallAngle < ANGLE_A))
   {
      // If Blue Team
      if (TeamColor == BLUE_TEAM)
      {
         // Bins #2 and #3 are available and unobstructed
         Bins[1].BinAvailable = Full;
         Bins[2].BinAvailable = Full;  
      }
      //Else red team
      else
      {
         // Bins #1 and #4 are available and unobstructed
         Bins[0].BinAvailable = Full;
         Bins[3].BinAvailable = Full;   
      }  //Endif 
   }
   //Else if region between A and B
   else if ((WallAngle > ANGLE_A) && (WallAngle < ANGLE_B))
   {
      //If blue team
      if (TeamColor == BLUE_TEAM)
      {  
         // Bin 3 is available and unobstructed
         Bins[2].BinAvailable = Full;
      }
      //Else red team
      else
      {
         // Bin #1 is available and unobstructed
         Bins[0].BinAvailable = Full;   
      }  //Endif  \
      
      // Bins #2 and #4 are available but obstructed to both sides
      Bins[1].BinAvailable = Partial;
      Bins[3].BinAvailable = Partial;
   } 
   //Else if region between B and C
   else if ((WallAngle > ANGLE_B) && (WallAngle < ANGLE_C))
   {
      // If blue team
      if (TeamColor == BLUE_TEAM)
      {
         // Bins #3 and #4 are available and unobstructed
         Bins[2].BinAvailable = Full;
         Bins[3].BinAvailable = Full;
      }
      //Else red team
      else
      {
         // Bins #1 and #2 are available and unobstructed
         Bins[0].BinAvailable = Full;
         Bins[1].BinAvailable = Full;
      }  //Endif     
   } 
   //Else if region between C and D
   else if ((WallAngle > ANGLE_C) && (WallAngle < ANGLE_D))
   {
      // If blue team
      if (TeamColor == BLUE_TEAM)
      {  
         // Bin 4 is available and unobstructed
         Bins[3].BinAvailable = Full;
      }
      //Else red team
      else
      {
         // Bin #2 is available and unobstructed
         Bins[1].BinAvailable = Full;
      }  //Endif
      
      // Bins #1 and #3 are available but obstructed to both sides
     Bins[0].BinAvailable = Partial;
     Bins[2].BinAvailable = Partial;
   }
   //Else if region between D and E
   else if ((WallAngle > ANGLE_D) && (WallAngle < ANGLE_E))
   {
      // If red team
      if (TeamColor == RED_TEAM)
      {
         // Bins #2 and #3 are available and unobstructed
         Bins[1].BinAvailable = Full;
         Bins[2].BinAvailable = Full; 
      }
      //Else blue team
      else
      {
         // Bins #1 and #4 are available and unobstructed
         Bins[0].BinAvailable = Full;
         Bins[3].BinAvailable = Full;   
      }  //Endif    
   }
   //Else if region between E and F
   else if ((WallAngle > ANGLE_E) && (WallAngle < ANGLE_F))
   {
      //If red team
      if (TeamColor == RED_TEAM)
      {  
         // Bin 3 is available and unobstructed
         Bins[2].BinAvailable = Full;
      }
      //Else blue team
      else
      {
         // Bin #1 is available and unobstructed
         Bins[0].BinAvailable = Full; 
      }  //Endif
      // Bins #2 and #4 are available but obstructed to both sides
      Bins[1].BinAvailable = Partial;
      Bins[3].BinAvailable = Partial;   
   }
   //Else if region between F and G
   else if ((WallAngle > ANGLE_F) && (WallAngle < ANGLE_G))
   {
      // If team is red
      if (TeamColor == RED_TEAM)
      {
         // Bins #3 and #4 are available and unobstructed
         Bins[2].BinAvailable = Full;
         Bins[3].BinAvailable = Full;
      }
      //Else blue team
      else
      {
         //bins 1 and 2 are abailable and unobstructed
         Bins[0].BinAvailable = Full;
         Bins[1].BinAvailable = Full;            
      }  //Endif 
   }
   // Else region between G and H
   else
   {
      // If red team
      if (TeamColor == RED_TEAM)
      {  
         // Bin 4 is available and unobstructed
         Bins[3].BinAvailable = Full;
      }
      //Else blue team
      else
      {
         // Bin #2 is available and unobstructed
         Bins[1].BinAvailable = Full;  
      }  //Endif
      // Bins #1 and #3 are available but obstructed to both sides
      Bins[0].BinAvailable = Partial;
      Bins[2].BinAvailable = Partial;  
   } //Endif
}  //End of BinsAvailable

/****************************************************************************
Function
     	PickScoringBin

Parameters
     	None

Returns
	ID of the bin we want to score in
		
Description
     	Determines which bins to score in
Notes

Author
     Alex Loo, 2/26/2012, hours no longer hold meaning
****************************************************************************/
unsigned char PickScoringBin(void)
{
   //Local Variables: i,j,k and reference
   unsigned char i;
   unsigned char j = 0;
   unsigned char k;
   Bin reference;
   
   // Run BinsAvailable to get the availabilities of the bins
   BinsAvailable();
   
   // First assign the numbers of balls to the bins
   for (i = 0; i <4; i++)
   {
      Bins[i].BallsInBin = Get_BallsInBin(i+1);
   }

   // Then order depending on the status of the bin        
   for (i = 0; i <4; i++)
   {
      if (Bins[i].BinAvailable == Full)
      {
         reference = Bins[j];
         Bins[j] = Bins[i];
         Bins[i] = reference;
         j++;
      } 
   }
   
   // Then order depending on the number of balls on the two subgroups, fully available and partially available
   
   // Fully available bins
   for (i = 0; i <j; i++)
   {
      reference = Bins[i];
      for (k = i; k <j; k++)
      {
         if (Bins[k].BallsInBin > Bins[i].BallsInBin)
         {
            Bins[i] = Bins[k];
            Bins[k] = reference;
            continue;
         }
      } 
   }
   
   // Partially available bins
   for (i = j; i <4; i++)
   {
      reference = Bins[i];
      for (k = i; k <4; k++)
      {
         if (Bins[k].BallsInBin > Bins[i].BallsInBin)
         {
            Bins[i] = Bins[k];
            Bins[k] = reference;
            continue;
         }
      } 
   }
   
   // Return the ID of the first Bin in the list (best scoring chance)
	return Bins[0].ID;
}  //End of PickScoringBin