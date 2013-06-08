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

/*
 * The number of *.log files we will keep,
 * where *1.log is the most recent, *n.log is the oldest.
 */
#define NUM_LOGS 5 

class Log {
  
 protected:
  std::string type; // e.g., all, thruster
  std::string log_name; // name of the log file
  ofstream current_log; 

 private:
  // Functions called internally
  bool file_exists(std::string checkfile);
  void copy_files(std::string from, std::string to);
  void create_file(int curLogs);
 
 public:
  // Constructor
  Log(std::string s);

  // Destructor
  ~Log();
  
  // Function called externally 
  void write_msg(std::string message);
}

#endif
