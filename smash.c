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

/*=============================================================================
* main function
=============================================================================*/
int main(int argc, char* argv[])
{
	char _cmd[CMD_LENGTH_MAX];
	while(1) {
		printf("smash > ");
		fgets(_line, CMD_LENGTH_MAX, stdin);
		strcpy(_cmd, _line);

		//AMIR thats what i think needs to be the flow -
		//check for finished jobs
		//parse cmd
		//check for status and execute / fork and add to jobList

		//initialize buffers for next command
		_line[0] = '\0';
		_cmd[0] = '\0';

	}

	return 0;
}
