/*
 * Logger Header File
 * Author: Basith Fahumy
 * 
 * Defines a log class. 
 * Instances of this class can be created as
 * different logs are needed. 
 * 
 * Ex. An all.log, a thruster.log, a gripper.log, etc.
 */

#ifndef __LOGGER_H__
#define __LOGGER_H__

#include <iostream>
#include <fstream>
#include <string>
#include <stdio.h>

// These may be needed...?
/*
 * #include <dirent.h>
 * #include <unistd.h>
 * #include <sys/stat.h>
 * #include <sys/types.h>
 */

class Log {
  
 protected:

  /*
   * The number of *.log files we will keep,
   * where *1.log is the most recent, *(NUM_LOGS).log is the oldest.
   */
  const int NUM_LOGS = 5;

  std::string type; // e.g., all, thruster
  std::string log_name; // name of the log file
  std::ofstream current_log; // The log file the instance will write to

 private:
  bool log_exists(std::string checkfile);
  void copy_log(std::string from, std::string to);
  void create_log(int curLogs);
 
 public:
  Log(std::string s);
  ~Log();
   
  void write_msg(std::string message);
}

#endif
