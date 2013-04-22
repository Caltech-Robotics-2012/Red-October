/*
 * Header for the Mission Commander States and stuff
 * AUthor: Basith Fahumy
 */

#ifndef __MISSIONCOMMANDER_H__
#define __MISSIONCOMMANDER_H__

#include <cstdlib>
#include <unordered_map>

// enum the states
enum SubState { Init = 00, ValGate = 10, TrafLite = 20, Park = 30,
		SpeedTrap = 40, TollBooth = 50, Driving = 60, DriveThru = 70,
		ToTrafLite = 120, ToPark = 130, ToSpeedTrap = 140, 
		ToTollBooth = 150, ToDriving = 160, ToDriveThru = 170 };

/* 
 * A map of the different functions...supposedly
 * Calls to these functions should be done through 
 * the map.
 */
typedef int (*state_function)();
std::unordered_map<int, state_functiion> sub_state_functions;

// Function to call different states:
int runState(SubState state); 

// Initialization
int initSub();

// Validation Gate
int validGate();

// Move to Traffic Light
int toTrafLight();
// Traffic Light
int trafLight();

// Move to Parking
int toPark();
// Parking
int park();

// Move to Speed Trap
int toSpeedTrap();
// Speed Trap
int speedTrap();

// Move to Toll Booth
int toTollBooth();
// Toll Booth
int tollBooth();

// Move to Driving
int toDriving();
// Driving
int driving();

// Move to Drive-thru
int toDriveThru();
// Drive-thru
int driveThru();

#endif
