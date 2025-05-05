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
	destroyJobList(globals->jobList);
	if ((globals->fgJob)) destroyJob(globals->fgJob);
	for (int i = 0; i < JOBS_NUM_MAX; i++){
		if (globals->pwd_pointers[i]) free(globals->pwd_pointers[i]);
		if (globals->file1[i]) fclose(globals->file1[i]);
		if (globals->file2[i]) fclose(globals->file2[i]);
	}
	if (globals) free(globals);
}

void init_globals() {
	globals = malloc(sizeof(struct globals));
	globals->jobList = initJobList();
	globals->last_path = NULL;
	globals->cur_path = NULL;
	globals->fgJob = NULL;
	for (int i = 0; i < JOBS_NUM_MAX; i++){
		globals->pwd_pointers[i] = NULL;
		globals->file1[i] = NULL;
		globals->file2[i] = NULL;
	}
}

/*=============================================================================
* main function
=============================================================================*/
int main(int argc, char* argv[])
{
	// Gloabls
	init_globals();
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
		ParsingError parse_status = parseCmd(_line, &command);
		if (parse_status == INVALID_COMMAND) {
			printf("smash error: invalid command\n");	// ASSUMPTION - basing on ext-command guidlins
			continue;
		}
		
		//check for status and execute (execv + args) / fork and add to jobList
		CommandResult end_status = SMASH_NULL;
		if (command->status == FOREGROUND){
			if (command->env == EXTERNAL){
				// create fgJob in case of SIGSTOP
				globals->fgJob = initJob(_line, FOREGROUND, getpid()); 
				end_status = run_cmd(command);
			} else { // INTERNAL
				end_status = run_cmd(command);
			}
		} else { // BACKGROUND
			int pid = fork();
			if (pid == 0) { // child process
				end_status = run_cmd(command);
			} else { // parent process
				addNewJob(_line, BACKGROUND, pid); 
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
