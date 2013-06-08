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
 */

#include "logger.h"

Log::Log(std::string s) {
  this->type = s;
  this->log_name = this->type + std::string("1.log");

  this->create_file(NUM_LOGS);
}

Log::~Log() {
  this->current_log.close();
}

// Tests whether given file is in current directory
bool Log::file_exists(std::string testfile) {
  std::ifstream test(testfile);
  if(test.good()) {
    return true;
  }
  return false;
}

// Function to copy a file. Assumes new file does not already exist.
void Log::copy_files(std::string from, std::string to) {
  ifstream src(from);
  ofstream dest(to);
  
  dest << src.rdbuff();
}

// Copies (or deletes) the old log files and opens the new log
void Log::create_file(int curLogs) {
  
  int start = 1;

  // Find oldest log file
  for(int i = curLogs; i > 0; i--) {
    string test = this->type + itoa(i) + std::string(".log");
    if (this->file_exists(test)){
      start = i; 
      break;
    }
  }

  // Delete and copy as necessry
  for(int i = start; i > 1; i--) {
    std::string older = this->type + itoa(i) + std::string(".log");
    std::string old = this->type + itoa(i - 1) + std::string(".log");
    remove(older.c_str());
    this->copy_files(old, older);
  }

  // Open the new file
  this->current_log.open(log_name, ios::trunc);
  this->current_log << this->type << " Log\n" << "\n";
}

// Writes information to appropriate log files
void write_msg(std::string message) {
  this->current_log << message << "\n";
}


