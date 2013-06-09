/*
 * Logger Program
 * Author: Basith Fahumy
 *
 * Logs messages from the dispatcher
 * 
 * Defines necessary functions.
 * Once class is initialized, only the write_msg()
 * function needs to be called.
 * 
 * log files are of the form "all1.log"
 */

#include "logger.h"

Log::Log(std::string s) {
  this->type = s;
  this->log_name = this->type + std::string("1.log");

  this->create_log(this->NUM_LOGS);
}

Log::~Log() {
  this->current_log.close();
}

// Tests whether given file is in current directory
bool Log::log_exists(std::string testfile) {
  std::ifstream test(testfile);
  return test.good();
}

// Function to copy a file. Assumes new file does not already exist.
void Log::copy_log(std::string from, std::string to) {
  std::ifstream src(from);
  std::ofstream dest(to);
  
  dest << src.rdbuf();
}

// Copies (or deletes) the old log files and opens the new log
void Log::create_log(int curLogs) {
  
  int start = 1;

  /* 
   * Find the oldest log in existence.
   * This is done because the copy function assumes
   * that the new file doesn't exist.
   * This assumes that the oldest possible
   * log will be *(curLogs).log 
   */
  for(int i = curLogs; i > 0; i--) {
    // File should have name (type)(number).log
    std::string test = this->type + itoa(i) + std::string(".log"); 
    if (this->log_exists(test)){
      start = i; 
      break; // only want to get out of this loop
    }
  }

  // Delete and copy as necessary
  for(int i = start; i > 1; i--) {
    // The "old" log becomes "older"
    std::string older = this->type + itoa(i) + std::string(".log");
    std::string old = this->type + itoa(i - 1) + std::string(".log");
    remove(older.c_str());
    this->copy_log(old, older);
  }

  // Open the new log
  this->current_log.open(log_name, ios::trunc);
  // Just a header for the log
  this->current_log << this->type << " Log" << endl << endl;
}

// Writes information to appropriate log files
void write_msg(std::string message) {
  this->current_log << message << endl;
}


