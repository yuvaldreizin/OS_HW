//commands.c
#include "commands.h"

//example function for printing errors from internal commands
void perrorSmash(const char* cmd, const char* msg){
    fprintf(stderr, "smash error:%s%s%s\n",
        cmd ? cmd : "",
        cmd ? ": " : "",
        msg);
}

/**
 * recieve a string and an empty cmd pointer
 * return a filled cmd struct, including memory allocation
 */
int parseCmd(char* line, cmd *command){
	char* delimiters = " \t\n"; //parsing should be done by spaces, tabs or newlines
	char* recieved_cmd = strtok(line, delimiters); //read strtok documentation - parses string by delimiters
	if(!recieved_cmd)
		return INVALID_COMMAND; //this means no tokens were found, most like since command is invalid
	
	char* args[ARGS_NUM_MAX];
	int nargs = 0;
	args[0] = recieved_cmd; //first token before spaces/tabs/newlines should be command name
	for(int i = 1; i < ARGS_NUM_MAX; i++)
	{
		args[i] = strtok(NULL, delimiters); //first arg NULL -> keep tokenizing from previous call
		if(!args[i])
			break;
		nargs++;
	}

	command = MALLOC_VALIDATED(struct cmd, sizeof(struct cmd));
	command->command = MALLOC_VALIDATED(char, strlen(recieved_cmd) + 1);
	int args_byte_size = 0;	
	for (int i = 0; i < nargs; i++){
		args_byte_size += strlen(args[i]) + 1;
	}
	command->args = MALLOC_VALIDATED(char*, args_byte_size);
	command->nargs = nargs;
	command->status = strcmp(args[nargs],"&") == 0 ? BACKGROUND : FOREGROUND ;
	command->env = (isInternalCommand(command) >= 0) ? INTERNAL : EXTERNAL;

	return VALID_COMMAND; 
}

void destroyCmd(cmd* cmd){	// add to smash
	if (cmd){
		if(cmd->args)
			free(cmd->args);
		if(cmd->command)
			free(cmd->command);
	}
}

/**
 * @brief Checks if a given command is an internal command.
 * 
 * @return The index of the internal command if found, or -1 if the command is external.
 */

int isInternalCommand(cmd *cmd){
	//check if command is internal or external
	for (int i = 0; i < NUM_INTERNAL_COMMANDS; i++)
	{
		if (strcmp(cmd->command, internal_command_t[i].command) == 0)
			return i; 
	}
	return -1; // External command
}

int run_cmd(cmd *cmd){
	if (!cmd) return 0;
	int index = isInternalCommand(cmd);
	if (index != -1){
		internal_command_t[index].func(cmd); // Call the command function
		return INTERNAL;
	} else {	// Add External command handle
		// exec + wait
		return EXTERNAL;
	}
}

int args_num_error(cmd *cmd, int expected_num){
    if (cmd->nargs != expected_num + 1){
		// handle error
        printf("smash error: %s: expected %d arguments\n", cmd->command, expected_num);
		return INVALID_COMMAND;
	}
    return VALID_COMMAND;
}

int showpid(cmd *cmd){
	if (args_num_error(cmd, 0) == INVALID_COMMAND){
		return INVALID_COMMAND;
	} else {
		if (cmd->status == FOREGROUND && cmd->env == INTERNAL){	// running in smash process - TODO - only FOREGROUND needed?
			printf("smash pid is %d\n", getpid());
		} else {												// running in child-process
			printf("smash pid is %d\n", getppid());				// TODO - compiling error, not sure why yet
		}
		return VALID_COMMAND;
	}
}

int pwd(cmd *cmd){ // TODO - change to global
	if (args_num_error(cmd, 0) == INVALID_COMMAND){
		return INVALID_COMMAND;
	}
	char *pwd = getcwd(NULL, 0);
	if (pwd != NULL){
		printf("%s\n", pwd);
		free(pwd);
		return VALID_COMMAND;
	} 
	// handle error
	free(pwd);
	return INVALID_COMMAND;
}

int cd(cmd *cmd){
	if (args_num_error(cmd, 1) == INVALID_COMMAND){
		return INVALID_COMMAND;
	}
	if (strcmp(cmd->args[1], "-") == 0){ // case: "-" (go to last path)
		if(!globals->last_path){
			printf("smash error: cd: expected %d arguments\n", 1);
			return INVALID_COMMAND;
		} else {
			pwd = getcwd(NULL, 0);
			chdir(globals->last_path);
			pwd(cmd);
			// switch last path
			free(globals->last_path);
			globals->last_path = pwd;
			return VALID_COMMAND;
		}
	} else { // case: general cd
		int res = chdir(cmd->args[1]);
		if (res == -1){
			if (errno = ENOENT){
				printf("smash error: cd: target directory does not exist\n");
				return INVALID_COMMAND;
			} else if (errno = ENOTDIR){
				printf("smash error:%s%s%s: not a directory\n",
					cmd ? cmd : "",
					cmd ? ": " : "",
					cmd->args[1]);
				return INVALID_COMMAND;
			} else 
				return VALID_COMMAND;
		}
	}
	
}

int jobs(cmd *cmd){
	if (args_num_error(cmd, 0) == INVALID_COMMAND){
		return INVALID_COMMAND;
	}
	printJobList(globals->jobList);
	return VALID_COMMAND;
}

int smashKill(cmd *cmd){
	if (cmd->nargs != 3 || /* ||*/ ((int)(cmd->args[1]) > 0 && (int)(cmd->args[1]) < _NSIG)){ // TODO - fix casting
		printf("smash error: cd: invalid arguments\n");
		return INVALID_COMMAND;
	} 
	int signum = cmd->args[1];
	int jobID = cmd->args[2];

	if (!lookup(jobID))
		printf("smash error: kill: job id %d does not exist\n",jobID);
	// send signal
	return VALID_COMMAND;
}

int fg(cmd  *cmd){
	int jobID;
	if (cmd->nargs > 2){
		printf("smash error: fg: invalid arguments\n");
		return INVALID_COMMAND;
	} else if (cmd->args[1] == NULL){
		if (globals->jobList->count == 0){
			return INVALID_COMMAND;
		} 
		jobID = 0; // TODO - replace with max available jobID
	} else {
		jobID = cmd->args[1];
		// check validity of jobid 
		// check if job exist
		
	}
	struct job *job = globals->jobList->jobs[jobID];
	if (getStatus(job) == STOPPED){
		bg(jobID);
	}
	printf(job->cmd);
	waitpid(job->pid);
	removeJob(globals->jobList, jobID);
	return VALID_COMMAND;
}

int bg(cmd *cmd){
	int jobID;
	if (cmd->nargs > 2){
		printf(stdout, "smash error: bg: invalid arguments\n");
		return INVALID_COMMAND;
	} else if (cmd->args[1] == NULL){
		if (/*are there any jobs*/ && /*are there any stopped jobs - for with lookup*/){
			return INVALID_COMMAND;
		} 
		jobID = 0; // TODO - replace with max available jobID
	} else {
		jobID = cmd->args[1];
		// check validity of jobid 
		if (/*doesn't exist*/){
			return INVALID_COMMAND;
		} else if (getStatus(globals->jobList->jobs[jobID];) != STOPPED) {
			printf("smash error: bg: job id %d is already in background\n",jobID);
			return INVALID_COMMAND;
		}
	}
	struct job *job = globals->jobList->jobs[jobID];
	printf(job->cmd);
	// handle job transmition with signal SIGCONT
	return VALID_COMMAND;
}

int quit(cmd *cmd){
	if (cmd->nargs > 2){
		printf("smash error: quit: expected 0 or 1 arguments\n");
		return INVALID_COMMAND;
	} else if (cmd->nargs == 1){
		exit(0);
	} else { // kill given
		if (strcmp()(cmd->args[1], "kill") != 0){
			printf("smash error: quit: unexpected argument\n");
			return INVALID_COMMAND;
		}
		for (int i = 0; i < JOBS_NUM_MAX; i++){
			if (globals->jobList->jobs[i]){
				printf("[%d] %s - ", i, globals->jobList->jobs[i]->cmd);
				kill(globals->jobList->jobs[i]->pid, SIGTERM);
				printf("sending SIGTERM... ");
				time_t start = time(NULL);
				int done = 0;
				sleep(5);
				// while(difftime(time(NULL), start) < 5){	// use sleep(5)
				// 	int res = waitpid(obList_t->jobs[i]->pid, WNOHANG);
				// 	if (res == 0){
				// 		sleep(1);
				// 	} else if (res == -1){ // needed?
				// 		return INVALID_COMMAND;
				// 	} else {
				// 		printf("done\n");
				// 		done = 1;
				// 		break;
				// 	}
				// }
				if (!done){
					kill(globals->jobList->jobs[i]->pid, SIGKILL);
					printf("sending SIGKILL... done\n");
				}
			}
		}
	}
	return VALID_COMMAND;
}

int diff(cmd *cmd){
	if (args_num_error(cmd, 1) == INVALID_COMMAND){
		return INVALID_COMMAND;
	}
	// check if paths are valid and files
	struct stat st;
	stat(cmd->args[1], &st);
	if(stat(cmd->args[1], &st) == -1 || stat(cmd->args[2], &st) == -1){
		printf(stdout, "smash error: diff: expected valid paths for files\n");
		return INVALID_COMMAND;
	}
	if(S_ISREG(cmd->args[1]) && S_ISREG(cmd->args[2])){
		printf("smash error: diff: paths are not files\n");
		return INVALID_COMMAND;
	}
	// diff
	return system("diff %s %s", cmd->args[1], cmd->args[2]);
}

int run_ext_cmd(cmd *cmd){
	int is_file = access(cmd->command, F_OK);
	if (is_file == -1){ // file not found
		printf("smash error: external: cannot find program\n");
		return INVALID_COMMAND;
	}
	int id = fork();
	if (id==0){
		int ret = execv(cmd->command, cmd->args);
		if (ret ==-1) printf("smash error: external: invalid command\n");
	} else {
		waitpid(id, NULL, 0);
	}
	return VALID_COMMAND;
}