//smash.c

/*=============================================================================
* includes, defines, usings
=============================================================================*/
#include <stdlib.h>

#include "commands.h"
#include "signals.h"
#include "jobs.h"
/*=============================================================================
* classes/structs declarations
=============================================================================*/

/*=============================================================================
* global variables & data structures
=============================================================================*/
char _line[CMD_LENGTH_MAX];
typdef struct globals {
	jobList_t jobList;
	char* last_path;
} globals_t;
globals_t globals = {NULL, NULL};
jmp_buf env; // global variable to store the environment for longjmp

/*=============================================================================
* main function
=============================================================================*/
int main(int argc, char* argv[])
{
	globals->joblist = initJobList();
	char _cmd[CMD_LENGTH_MAX];
	while(1) {
		setjmp(env); // set the environment for longjmp
		printf("smash > ");
		fgets(_line, CMD_LENGTH_MAX, stdin);
		strcpy(_cmd, _line);

		//AMIR thats what i think needs to be the flow -
		//check for finished jobs
		//parse cmd - create a cmd struct with command, args and status (in/external + background)?
		//check for status and execute (execv + args) / fork and add to jobList

		//initialize buffers for next command
		_line[0] = '\0';
		_cmd[0] = '\0';
		//kill last path - not NULL
	}

	return 0;
}
