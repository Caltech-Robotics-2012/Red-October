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

// Runs the given state through a map call
SubState runState(SubState state) {
  return sub_state_functions[state]();
}

SubState init() {
  /* TODO:
   * Whatever init requires
   */
  return ValGate;
}

SubState validGate() {
  /* TODO:
   * Move forward
   */
  return ToTrafLight;
}

SubState toTrafLight() {
  /* TODO:
   * Find path and travel to Traffic Light station
   */
  return TrafLight;
}

SubState trafLight() {
  /* TODO:
   * See and hit buoys
   */
  return ToPark;
}

SubState toPark() {
  /* TODO:
   * Find path and travel to bins
   */
  return Park;
}

SubState park() {
  /* TODO:
   * Check when light is green, drive over (sideways) 
   */
  return ToSpeedTrap;
}

SubState toSpeedTrap() {
  /* TODO:
   * Find a way to get to the bins
   */
  return SpeedTrap;
}

SubState speedTrap() {
  /* TODO:
   * Recognize correct bins, drop markers
   */
  return ToTollBooth;
}

SubState toTollBooth() {
  /* TODO:
   * Locate cutout of torpedo targets and move toward it
   */
  return TollBooth;
}

SubState tollBooth() {
  /* TODO:
   * Identify correct targets, aim and fire torpedos
   */
  return ToDriving;
}

SubState toDriving() {
  /* TODO:
   * Find the appropriate station
   */
  return Driving;
}

SubState driving() {
  /* TODO:
   * Turn wheel, Identify lever state and move it
   */
  return ToDriveThru;
}

SubState toDriveThru() {
  /* TODO:
   * Detect pinger and move to object
   */
  return DriveThru;
}

SubState driveThru() {
  /* TODO:
   * Retrieve object and surface in octagon
   */
  return COMPLETE;
}

/*
 * Assign states to positions in map
 * Done here because this doesn't compile if 
 * the initialization is done in the header...
 */
void assignStates() {
  sub_state_functions[Init] = &initSub;
  sub_state_functions[ValGate] = &validGate;
  sub_state_functions[TrafLight] = &trafLight;
  sub_state_functions[Park] = &park;
  sub_state_functions[SpeedTrap] = &speedTrap;
  sub_state_functions[TollBooth] = &tollBooth;
  sub_state_functions[Driving] = &driving;
  sub_state_functions[DriveThru] = &driveThru;

  sub_state_functions[ToTrafLight] = &toTrafLight;
  sub_state_functions[ToPark] = &toPark;
  sub_state_functions[ToSpeedTrap] = &toSpeedTrap;
  sub_state_functions[ToTollBooth] = &toTollBooth;
  sub_state_functions[ToDriving] = &toDriving;
  sub_state_functions[ToDriveThru] = &toDriveThru;
}


int main() {

  assignStates();

  SubState current = 0;

  while(current != COMPLETE) {
    current = runState(current);
  }

  return 0;
}
