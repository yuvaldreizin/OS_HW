//smash.c

/*=============================================================================
* includes, defines, usings
=============================================================================*/
#define _POSIX_C_SOURCE 200809L
#include <stdlib.h>

#include "commands.h"
#include "signals.h"
#include "jobs.h"
#include <setjmp.h>
#include <sys/types.h>

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
	destroyJobList();
	if ((globals->fgJob)) destroyJob(globals->fgJob);
	for (int i = 0; i < JOBS_NUM_MAX; i++){
		if (globals->pwd_pointers[i]) free(globals->pwd_pointers[i]);
		if (globals->file1[i]) fclose(globals->file1[i]);
		if (globals->file2[i]) fclose(globals->file2[i]);
	}
	if (globals) free(globals);
}

void init_globals() {
	globals = (globals_t)malloc(sizeof(struct global_params));
	if (!globals) ERROR_EXIT("malloc");
	globals->jobList = initJobList();
	globals->last_path = NULL;
	globals->cur_path = NULL;
	globals->fgJob = NULL;
	for (int i = 0; i < JOBS_NUM_MAX; i++){
		globals->pwd_pointers[i] = NULL;
		globals->file1[i] = NULL;
		globals->file2[i] = NULL;
	}
	globals->smash_pid = getpid();
}

/*=============================================================================
* main function
=============================================================================*/
int main(int argc, char* argv[])
{
	// Gloabls
	init_globals();
	char _cmd[CMD_LENGTH_MAX];
	setupSignalHandlers(); // setup signal handlers
	while(1) { 
		sigsetjmp(env_buf, 1); // set the environment for longjmp
		printf("smash > ");
		fgets(_line, CMD_LENGTH_MAX, stdin);
		strcpy(_cmd, _line);

		//parse cmd
		cmd_t *curr_cmd = NULL;
		ParsingError parse_status = parseCmd(_line, &curr_cmd);
		if (parse_status == INVALID_COMMAND) {
			// printf("smash error: invalid command\n");	// ASSUMPTION - basing on ext-command guidlins
			continue;
		}
		
		//check for status and execute (execv + args) / fork and add to jobList
		CommandResult end_status = SMASH_NULL;
		if (curr_cmd->cmdStatus == FOREGROUND){
			if (curr_cmd->env == EXTERNAL){
				// create fgJob in case of SIGSTOP
				globals->fgJob = initJob(_line, FOREGROUND, 0); 
				end_status = run_cmd(curr_cmd);
			} else { // INTERNAL
				end_status = run_cmd(curr_cmd);
			}
			destroyCmd(curr_cmd);
		} else { // BACKGROUND
			int new_pid = fork();
			if (new_pid == 0) { // child process
				setpgid(0, 0);
				run_cmd(curr_cmd);
				destroyCmd(curr_cmd); 
				exit(0); // child process exits
			} else if (new_pid > 0){ // parent process
				char *temp = _cmd;
				while (*temp != '\0' && *temp != '\n') temp++;
				*temp = '\0'; // remove the newline character
				addNewJob(_cmd, BACKGROUND, new_pid);
				// ASSUMPTION - are we dropping jobs/commands if list is full?
			}
		}
		//initialize buffers for next command
		_line[0] = '\0';
		_cmd[0] = '\0';
		// destroyCmd(curr_cmd); - not here so BACKGROUND parent won't kill child
		if (end_status == SMASH_QUIT) {
			break; 
		}
	}
	destroy_globals();
	return 0;
}
