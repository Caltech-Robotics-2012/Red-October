/*
 * Mission Commander...commands
 * Author: Basith Fahumy
 * 
 * Code Skeleton. Bare Bones. No Meat.
 * (Doesn't do anything yet)
 * Order is tentative, pending final mission objectives
 * 
 */

#include "MissionCommander.h"

// Runs the given state somehow
int runState(SubState state) {
  return sub_state_functions[state]();
}

// Initialization state
int init() {
  /* TODO:
   * Whatever init requires
   */
  return ValGate; // Next step
}

// Validation Gate state
int validGate() {
  /* TODO:
   * Move forward
   */
  return ToTrafLite;
}

int toTrafLight() {
  /* TODO:
   * Find path and travel to Traffic Light station
   */
  return TrafLite;
}

// Traffic Light state
int trafLight() {
  /* TODO:
   * See and hit buoys
   */
  return ToPark;
}

int toPark() {
  /* TODO:
   * Find path and travel to bins
   */
  return Park;
}

// Parking state
int park() {
  /* TODO:
   * Check when light is green, drive over (sideways) 
   */
  return ToSpeedTrap;
}

int toSpeedTrap() {
  /* TODO:
   * Find a way to get to the bins
   */
  return SpeedTrap;
}

// Speed Trap state
int speedTrap() {
  /* TODO:
   * Recognize correct bins, drop markers
   */
  return ToTollBooth;
}

int toTollBooth() {
  /* TODO:
   * Locate cutout of torpedo targets and move toward it
   */
  return TollBooth;
}

// Toll Booth state
int tollBooth() {
  /* TODO:
   * Identify correct targets, aim and fire torpedos
   */
  return ToDriving;
}

int toDriving() {
  /* TODO:
   * Find the appropriate station
   */
  return Driving;
}

// Driving state
int driving() {
  /* TODO:
   * Turn wheel, Identify lever state and move it
   */
  return ToDriveThru;
}

int toDriveThru() {
  /* TODO:
   * Detect pinger and move to object
   */
  return DriveThru;
}

// Drive-thru state
int driveThru() {
  /* TODO:
   * Retrieve object and surface in octagon
   */
  return 0;
}


// run everything!
int main() {
  // Assign states in map
  sub_state_functions[Init] = &initSub;
  sub_state_functions[ValGate] = &validGate;
  sub_state_functions[TrafLite] = &trafLight;
  sub_state_functions[Park] = &park;
  sub_state_functions[SpeedTrap] = &speedTrap;
  sub_state_functions[TollBooth] = &tollBooth;
  sub_state_functions[Driving] = &driving;
  sub_state_functions[DriveThru] = &driveThru;

  // Transit States
  sub_state_functions[ToTrafLite] = &toTrafLight;
  sub_state_functions[ToPark] = &toPark;
  sub_state_functions[ToSpeedTrap] = &toSpeedTrap;
  sub_state_functions[ToTollBooth] = &toTollBooth;
  sub_state_functions[ToDriving] = &toDriving;
  sub_state_functions[ToDriveThru] = &toDriveThru;



  int i = 0; // Numerically call states
  int cur = 0

  while(i < 200) { // Last fn should return 200 (or higher)
    cur = runState(i);

    if(cur == 100) {
      i = runstate(cur);
    } else {
      i = cur;
    }

  }

  return 0;
}
