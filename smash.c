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
struct globals {
	jobList_t jobList;
	char* last_path;
	char* cur_path;
	job_t fgJob;
	char *pwd_pointers[JOBS_NUM_MAX]; 
};
typedef struct globals* globals_t;
globals_t globals;

void destroy_globals() {
	if !(globals->last_path) {
		free(globals->last_path);
	}
	if !(globals->cur_path) {
		free(globals->cur_path);
	}
	destroyJobList(globals->jobList);
	if !(globals->fgjob) free(globals->fgJob);
	for (int i = 0; i < JOBS_NUM_MAX; i++){
		if (globals->pwd_pointers[i]) free(globals->pwd_pointers[i]);
	}
}

/*=============================================================================
* main function
=============================================================================*/
int main(int argc, char* argv[])
{
	// Gloabls
	globals->joblist = initJobList();
	char _cmd[CMD_LENGTH_MAX];
	while(1) { 
		setjmp(env); // set the environment for longjmp
		printf("smash > ");
		fgets(_line, CMD_LENGTH_MAX, stdin);
		strcpy(_cmd, _line);
		
		//check for finished jobs
		removeFinishedJobs();
		
		//parse cmd
		cmd *command;
		int status = parse_cmd(_line, command);
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
				globals->fgJob = initJob(_line, FOREGROUND, command->pid); 
				end_status = run_cmd(command);
			}
		} else { // BACKGROUND
			int pid = fork();
			if (pid == 0) { // child process
				end_status = run_cmd(command);
			} else { // parent process
				addNewJob(_line, BACKGROUND, command->pid); 
				// ASSUMPTION - are we dropping jobs/commands if list is full?
				// YUVAL - We need an indication at least to destroy the job
			}
		}
`
		//initialize buffers for next command
		_line[0] = '\0';
		_cmd[0] = '\0';
		destroyCmd(command);
		if (end_status == SMASH_QUIT) {
			break; 
		}
	}
	destroy_globals()
	return 0;
}
