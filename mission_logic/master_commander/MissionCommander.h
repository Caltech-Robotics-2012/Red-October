/*
 * Header for the Mission Commander 
 * AUthor: Basith Fahumy
 */

#ifndef __MISSIONCOMMANDER_H__
#define __MISSIONCOMMANDER_H__

#include <cstdlib>
#include <unordered_map>

// enum the states as type SubState
enum SubState { Init, ValGate, TrafLight, Park, SpeedTrap, TollBooth, 
		Driving, DriveThru, ToTrafLight, ToPark, ToSpeedTrap, 
		ToTollBooth, ToDriving, ToDriveThru, COMPLETE };

/* 
 * A map of the different functions
 * Calls to these functions should be done through 
 * the map.
 */
typedef SubState (*state_function)();
std::unordered_map<SubState, state_functiion> sub_state_functions;

// Function to call different states:
SubState runState(SubState state); 

// States
SubState initSub();

SubState validGate();

SubState toTrafLight();
SubState trafLight();

SubState toPark();
SubState park();

SubState toSpeedTrap();
SubState speedTrap();

SubState toTollBooth();
SubState tollBooth();

SubState toDriving();
SubState driving();

SubState toDriveThru();
SubState driveThru();

// Function that initializes map, first call in main
void assignStates();

#endif
