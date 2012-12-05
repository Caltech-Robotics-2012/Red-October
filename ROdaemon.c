/*****************************************************************
* Red October Daemon
*
* Description: Responsible for starting the main loop of the sub, once power is
*	       supplied (killswitch closed).
* Operation:   Creates a child process, giving it a SID and PID
*	       Opens the log, and calls the mainloop
*
* Defects:     Could not be what is necessary
*	       doesn't initialize 
* Revision History:     12-5-12    Tim Holland   Initial Revision
*****************************************************************/

// If you question design decisions, ask 
// www.netzmafia.de/skripten/unix/linux-daemon-howto.html

#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <syslog.h>
#include <string.h>

int main(void) {

	// For autonomy, it needs to be a child process
	// fork: Creates a child process and returns a process id number 
	// (PID) if it was successful. Else returns -1.
	// See man fork for more

	pid_t pid, sid;

	/* Fork off the parent process */
	pid = fork();
	if (pid < 0) {
		exit(EXIT_FAILURE);
	}
	/* If we got a good PID, then we can exit the parent process */
	if (pid > 0) {
		exit(EXIT_SUCCESS);
	}

	//Need to change file mode to be able to write to files (eg logs)
	umask(0);


	// Open the log files
	// Taken from www.cprogramming.com/tutorial/cfileio.html
	// Better to handle in main loop?
	FILE *log;
	log = fopen("/var/logs/ROoperation", "a");

	// !!! This an innapropriate place to close !!!
	fclose(log);

	// Child process needs an S(ession)ID to operate. Why? I don't know, it 
	// becomes an "orphan" without one.
	sid = setsid();
	if (sid < 0) {
		fprintf(log, "Error: unable to assign SID to Daemon.\n");
		exit(EXIT_FAILURE);
	}

	// Ensure that directory is /
	if ((chdir("/")) < 0) {
		fprintf(log, "Error: unable to change to root directory.\n");
		exit(EXIT_FAILURE);
	}

	// Close standard file descriptors so it doesn't mess with the terminal
	// close: deletes thedescriptor from the per-process object reference table
	// see man close for more

	close(STDIN_FILENO);
	close(STDERR_FILENO);

	// Initialization?
	while (1) {
		printf("Hello World!");
		sleep(3);
	}
    exit(EXIT_SUCCESS);
}
