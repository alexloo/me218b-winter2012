/****************************************************************************
   Description
         FSR.h is the header file for the FSR.c module

 History
 When           Who	What/Why
 -------------- ---	--------
 02/07/12 19:13 adl  First pass
****************************************************************************/

#ifndef FSR_H
#define FSR_H

// function prototypes
void SPI_Init(void);
unsigned char Get_BallsInPlay(void);   
unsigned char Get_BallsInBin(char BinNumber);   
unsigned int Get_WallAngle(void);   

#endif 