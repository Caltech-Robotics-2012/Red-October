/*
 * Mission Commander...commands
 * Author: Basith Fahumy
 * 
 * Doesn't do anything yet
 */

#include "MissionCommander.h"

// Runs the given state somehow
int runState(SubState state) {
  sub_state_functions[state]();

  return 0;
}

// Initialization state
int init() {
  return 0;
}

// Validation Gate state
int validGate() {
  return 0;
}

// Traffic Light state
int trafLight() {
  return 0;
}

// Speed Trap state
int speedTrap() {
  return 0;
}

// Toll Booth state
int tollBooth() {
  return 0;
}

// Driving state
int driving() {
  return 0;
}

// Drive-thru state
int driveThru() {
  return 0;
}


// run everything!
int main() {
  return 0;
}
