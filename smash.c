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
jmp_buf env_buf; // global variable to store the environment for longjmp


char _line[CMD_LENGTH_MAX];

globals_t globals;

void destroy_globals() {
	if (!(globals->last_path)) {
		free(globals->last_path);
	}
	if (!(globals->cur_path)) {
		free(globals->cur_path);
	}
	destroyJobList(globals->jobList);
	if (!(globals->fgJob)) free(globals->fgJob);
	for (int i = 0; i < JOBS_NUM_MAX; i++){
		if (globals->pwd_pointers[i]) free(globals->pwd_pointers[i]);
		if (globals->file1[i]) fclose(globals->file1[i]);
		if (globals->file2[i]) fclose(globals->file2[i]);
	}
}

/*=============================================================================
* main function
=============================================================================*/
int main(int argc, char* argv[])
{
	// Gloabls
	globals->jobList = initJobList();
	char _cmd[CMD_LENGTH_MAX];
	while(1) { 
		setjmp(env_buf); // set the environment for longjmp
		printf("smash > ");
		fgets(_line, CMD_LENGTH_MAX, stdin);
		strcpy(_cmd, _line);
		
		//check for finished jobs
		removeFinishedJobs();
		
		//parse cmd
		cmd *command = NULL;
		int status = parseCmd(_line, command);
		if (status == INVALID_COMMAND) {
			printf("smash error: invalid command\n");	// ASSUMPTION - basing on ext-command guidlins
			continue;
		}

		//check for status and execute (execv + args) / fork and add to jobList
		int end_status = SMASH_NULL;
		if (command->status == FOREGROUND){
			end_status = run_cmd(command);
			if (command->env == EXTERNAL){
				// create fgJob in case of SIGSTOP
				// YUVAL - make sure I did this init right
				// YUVAL - change initJob to use nextID and check if not in max (instead of in addNewJob) so we can use it here
				globals->fgJob = initJob(_line, FOREGROUND, commandPID(command)); 

				end_status = run_cmd(command);
			}
		} else { // BACKGROUND
			int pid = fork();
			if (pid == 0) { // child process
				end_status = run_cmd(command);
			} else { // parent process
				addNewJob(_line, BACKGROUND, commandPID(command)); 
				// ASSUMPTION - are we dropping jobs/commands if list is full?
				// YUVAL - We need an indication at least to destroy the job
			}
		}

		//initialize buffers for next command
		_line[0] = '\0';
		_cmd[0] = '\0';
		destroyCmd(command);
		if (end_status == SMASH_QUIT) {
			break; 
		}
	}
	destroy_globals();
	return 0;
}
