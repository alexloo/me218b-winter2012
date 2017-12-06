/****************************************************************************
Description
         TimingConstants.h is a header file that defines certain timing values
         that are used throughout the state machine.

 History
 When           Who	What/Why
 -------------- ---	--------
 02/26/12 01:49 adl	First pass
****************************************************************************/

#ifndef TIMINGCONSTANTS_H
#define TIMINGCONSTANTS_H

// Define clock pulses in a second when using a 2MS timer rate
#define _SECONDS_TIMER *488
#define _HALF_SECONDS_TIMER *244
#define _QUARTER_SECONDS_TIMER *122
#define _EIGTH_SECONDS_TIMER *61

#define _US_ *3 // 3 clock ticks per us
#define _MS_ *3000 // 3 clock ticks per us
#define _SEC_ *3000000 // 3 clock ticks per us

// Times for turns
#define DEGREE90_INTERVAL 300
#define DEGREE30_INTERVAL (unsigned int)(DEGREE90_INTERVAL/3)
#define DEGREE15_INTERVAL (unsigned int)(DEGREE90_INTERVAL/6)

// Specific speeds
#define CAUTION_SPEED 75
#define BEACON_SEARCH_TURN_SPEED 75

// Defines for the different timers
// End of game timer
#define END_GAME_TIMER 0
#define LENGTH_OF_GAME (120 _SECONDS_TIMER)

// Go to scoring timer
#define GO_TO_SCORING_TIMER 1
#define PROCEED_TO_SCORING (75 _SECONDS_TIMER)

// Timer for different motions
#define MOTION_TIMER 2
// Gathering SM
#define CAUTIONSPEED_INTERVAL (2 _SECONDS_TIMER) // how long to slow down for when tape seen
#define BACKUP_INTERVAL (1 _QUARTER_SECONDS_TIMER) // how long to backup for when bumper is hit
#define TURN_EVADE_INTERVAL (DEGREE30_INTERVAL*2)

// Scoring SM
#define FIRST_FWD_ALIGN_INTERVAL 3 _HALF_SECONDS_TIMER // for aligning the first pass with the opposite beacon
#define SECOND_FWD_ALIGN_INTERVAL 3 _QUARTER_SECONDS_TIMER // for aligning the second pass with the opposite beacon
#define CLEARANCE_INTERVAL 3 _QUARTER_SECONDS_TIMER // for driving forward from the wall after bumping it with rear when looking for the target bin
#define UNLOAD_INTERVAL 1 _HALF_SECONDS_TIMER // delay after ramming but before shuffling
#define CAUTION_INTERVAL (2) _SECONDS_TIMER // how long to slow down for when tape is seen

//Defending SM
#define WALL_SEPARATION_INTERVAL 5 _QUARTER_SECONDS_TIMER // how long to depart the wall to get into defending position
#define RESET_INTERVAL 7 _QUARTER_SECONDS_TIMER

// Timer for how long to shuffle
#define SHUFFLE_TIMER 3
#define SHUFFLE_INTERVAL (5) _SECONDS_TIMER

// Timer for each shuffle step
#define SHUFFLE_STEP_TIMER 4
#define SHUFFLE_STEP_INTERVAL 1 _QUARTER_SECONDS_TIMER
// 122 ticks at 2.048mS is 0.25s

// Timer for going to backup search method 
#define GO_TO_BACKUP_SEARCH_TIMER 5
#define GO_TO_BACKUP_SEARCH_INTERVAL 6*DEGREE90_INTERVAL

// Timer for doing a "nod" while looking for the beacon
#define BEACON_NOD_TIMER 6
#define BEACON_NOD_INTERVAL 1 _QUARTER_SECONDS_TIMER // time to get off beacon before swinging the other way

// Timer for pushing the wall in Gathering mode
#define WALL_BUMP_TIMER 6
#define WALL_BUMP_INTERVAL 50 // 100 ms

// Timer for forward then back motion
#define UNLOADING_DELAY_TIMER 5
#define FAN_SPIN_DOWN_TIMER (1 _HALF_SECONDS_TIMER) // how long to wait after last approach and going forward for bumping

#define UNLOADING_BUMP_TIMER 6
#define FORWARD_BUMP_INTERVAL 4 _QUARTER_SECONDS_TIMER // how far to separate before ramming the wall backward

// Timer for preventing jamming when backing into our target bin
#define ANTIJAM_TIMER 7
#define ANTIJAM_INTERVAL 8 _SECONDS_TIMER // How long to back up for until we try to go for a secondary bin

// Turn direction for evasive maneuvers
typedef enum {Left, Right} TurnDirection_t;

#endif