/*
 * Header for the Mission Commander States and stuff
 * AUthor: Basith Fahumy
 */

#ifndef __MISSIONCOMMANDER_H__
#define __MISSIONCOMMANDER_H__

#include <stdlib>
#include <unordered_map>

// enum the states
enum SubState { Init = 00, ValGate = 10, TrafLite = 20, Park = 30,
		SpeedTrap = 40, TollBooth = 50, Driving = 60, DriveThru = 70,
		Commute = 100 };

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
sub_state_functions[Init] = &initSub;

// Validation Gate
int validGate();
sub_state_functions[ValGate] = &validGate;

// Move to Traffic Light
// Traffic Light
int trafLight();
sub_state_functions[TrafLite] = &trafLight;

// Move to Parking
// Parking
int park();
sub_state_functions[Park] = &park;

// Move to Speed Trap
// Speed Trap
int speedTrap();
sub_state_functions[SpeedTrap] = &speedTrap;

// Move to Toll Booth
// Toll Booth
int tollBooth();
sub_state_functions[TollBooth] = &tollBooth;

// Move to Driving
// Driving
int driving();
sub_state_functions[Driving] = &driving;

// Move to Drive-thru
// Drive-thru
int driveThru();
sub_state_functions[DriveThru] = &driveThru;

#endif
