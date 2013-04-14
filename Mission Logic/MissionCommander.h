/*
 * Header for the Mission Commander States and stuff
 */

#ifndef __MISSIONCOMMANDER_H__
#define __MISSIONCOMMANDER_H__

// enum the states
enum SubState { Init = 00, ValGate = 10, TrafLite = 20, Park = 30,
		SpeedTrap = 40, TollBooth = 50, Driving = 60, DriveThru = 70,
		Commute = 100 };

// Function to call different states:
int runState(SubState); 


// Initialization
int initSub();

// Validation Gate
int validGate();

// Move to Traffic Light
// Traffic Light
int trafLight();

// Move to Parking
// Parking
int park();

// Move to Speed Trap
// Speed Trap
int SpeedTrap();

// Move to Toll Booth
// Toll Booth
int tollBooth();

// Move to Driving
// Driving
int driving();

// Move to Drive-thru
// Drive-thru
int driveThru();




#endif
