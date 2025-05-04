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

/**
 * @brief Frees the memory of a command structure.
 */
void destroyCmd(cmd* cmd){
	if (cmd){
		if(cmd->args)
			free(cmd->args);
		if(cmd->command)
			free(cmd->command);
		free(cmd);
	}
}

/**
 * @brief Checks if a given command is an internal command.
 * @return The index of the internal command if found, or -1 if the command is external.
 */
int isInternalCommand(cmd *cmd){
	//check if command is internal or external
	for (int i = 0; i < NUM_INTERNAL_COMMANDS; i++){
		if (strcmp(cmd->command, internal_command_t[i].command) == 0)
			return i; 
	}
	return -1; // External command
}

/**
 * @brief run the given command, handle both internal and external commands
 * @return End status - success or failure
 */
int run_cmd(cmd *cmd){
	if (!cmd) return 0;
	if (cmd->env){
		int index = isInternalCommand(cmd); // get index
		return internal_command_t[index].func(cmd); 
	} else { // EXTERNAL
		return run_ext_cmd(cmd);
	}
}

/**
 * @brief Check if the number of arguments is as expected. Print error if not.
 * @return 0 if the number of arguments is correct, -1 otherwise.
 */
int args_num_error(cmd *cmd, int expected_num){
    if (cmd->nargs != expected_num){
        printf("smash error: %s: expected %d arguments\n", cmd->command, expected_num);
		return INVALID_COMMAND;
	}
    return VALID_COMMAND;
}

/**
 * @brief print smash pid
 * @return end status
 */
int showpid(cmd *cmd){
	if (args_num_error(cmd, 0) == INVALID_COMMAND){
		return SMASH_FAIL;
	} else {
		if (cmd->status == FOREGROUND && cmd->env == INTERNAL){	// running in smash process
			printf("smash pid is %d\n", getpid());
		} else {	// running in child-process
			printf("smash pid is %d\n", getppid());
		}
		return SMASH_SUCCESS;
	}
}

/**
 * @brief print current working directory
 * @return end status
 */
int pwd(cmd *cmd){ 
	if (args_num_error(cmd, 0) == INVALID_COMMAND){
		return SMASH_FAIL;
	}
	if (cmd->status == BACKGROUND && cmd->env == INTERNAL){	// running in smash process
		char *pwd = getcwd(NULL, 0);
		if (pwd != NULL){
			printf("%s\n", pwd);
			free(pwd);
			return SMASH_SUCCESS;
		} 
		perror("smash error: pwd: getcwd failed");
		free(pwd);
		return SMASH_FAIL;
	} else { // running in job
		// use global pointers struct and free previously allocated memory if exists
		int id = jobPIDLookup(getpid())->ID;
		if (!globals->pwd_pointers[id]) free(globals->pwd_pointers[id]);

		globals->pwd_pointers[id] = getcwd(NULL, 0); 
		if (globals->pwd_pointers[id] != NULL){
			printf("%s\n", globals->pwd_pointers[id]);
			return SMASH_SUCCESS;
		} 
		perror("smash error: pwd: getcwd failed");
		return SMASH_FAIL;
	}
}

int cd(cmd *cmd){
	if (args_num_error(cmd, 1) == INVALID_COMMAND){
		return SMASH_FAIL;
	}
	if (strcmp(cmd->args[1], "-") == 0){ // case: "-" (go to last path)
		if(!globals->last_path){
			printf("smash error: cd: old pwd not set\n");
			return SMASH_FAIL;
		} else {
			free(globals->cur_path);
			globals->cur_path = globals->last_path;
			globals->last_path = getcwd(NULL, 0); 
			chdir(globals->cur_path);
			return SMASH_SUCCESS;
		}
	} else { // case: general cd (including ".." as valid chdir argument)
		int res = chdir(cmd->args[1]);
		if (res == -1){
			if (errno == ENOENT){
				printf("smash error: cd: target directory does not exist\n");
				return SMASH_FAIL;
			} else if (errno = ENOTDIR){
				printf("smash error: cd: %s: not a directory\n", cmd->args[1]);
				return SMASH_FAIL;
			} else {
				perror("smash error: cd: chdir failed");
				return SMASH_FAIL;
			}
		} else { // good chdir
			return SMASH_SUCCESS;
		}
	}
	return SMASH_FAIL;
}

/**
 * @brief print all jobs in the job list
 * @return end status
 */
int jobs(cmd *cmd){
	if (args_num_error(cmd, 0) == INVALID_COMMAND){
		return SMASH_FAIL;
	}
	printJobList(globals->jobList);
	return SMASH_SUCCESS;
}

/**
 * @brief send signal to job with given jobID
 * @return end status
 */
int smashKill(cmd *cmd){
	if (cmd->nargs != 2 || 
		(int)(strtol(cmd->args[1], NULL, 10) < 0) || 
		(int)(strtol(cmd->args[2], NULL, 10) > JOBS_NUM_MAX) || 
		(int)(strtol(cmd->args[2], NULL, 10) < 0)){ 
		printf("smash error: cd: invalid arguments\n");
		return SMASH_FAIL;
	} 
	int signum = (int)(strtol(cmd->args[1], NULL, 10));
	int jobID = (int)(strtol(cmd->args[2], NULL, 10));

	if (!jobLookup(jobID))
		printf("smash error: kill: job id %d does not exist\n",jobID);
	sendSignal(signum, jobID);
	return SMASH_SUCCESS;
}

/**
 * @brief bring job to foreground and wait for it to finish. if no id is given, take max available jobID
 * @return end status
 */
int fg(cmd  *cmd){
	unsigned int jobID;
	if (cmd->nargs > 1){
		printf("smash error: fg: invalid arguments\n");
		return SMASH_FAIL;
	} else if (cmd->args[1] == NULL){
		if (maxAvailableJobID() == -1){
			printf("smash error: fg: jobs list is empty\n");
			return SMASH_FAIL;
		} 
		jobID = maxAvailableJobID();
	} else {
		jobID = (unsigned int)(strtol(cmd->args[1], NULL, 10));
		if (!jobLookup(jobID)){
			printf("smash error: fg: job id %d does not exist\n",jobID);
			return SMASH_FAIL;
		}
	}

	struct job *job = globals->jobList->jobs[jobID];
	if (getStatus(job) == STOPPED){
		kill(job->pid, SIGCONT);
	}
	printf("%s", job->cmd);
	waitpid(job->pid);
	removeJob(globals->jobList, jobID);
	return SMASH_SUCCESS;
}

/**
 * @brief bring job to background and continue running. if no id is given, take max stopped jobID
 * @return end status
 */
int bg(cmd *cmd){
	int jobID;
	if (cmd->nargs > 1){
		printf("smash error: bg: invalid arguments\n");
		return SMASH_FAIL;
	} else if (cmd->args[1] == NULL){
		if (maxStoppedJobID() == -1){
			printf("smash error: bg: there are no stopped jobs to resume\n");
			return SMASH_FAIL;
		} 
		jobID = maxStoppedJobID();
	} else {
		jobID = (unsigned int)(strtol(cmd->args[1], NULL, 10));
		if (!jobLookup(jobID)){
			printf("smash error: bg: job id %d does not exist\n", jobID);
			return SMASH_FAIL;
		} else if (getStatus(globals->jobList->jobs[jobID]) != STOPPED) {
			printf("smash error: bg: job id %d is already in background\n",jobID);
			return SMASH_FAIL;
		}
	}
	struct job *job = globals->jobList->jobs[jobID];
	printf("%s: %d", job->cmd, job->ID);
	kill(job->pid, SIGCONT);
	return SMASH_SUCCESS;
}

/**
 * @brief quit the shell. if "kill" is given, kill all jobs in the job list
 * @return end status
 */
int quit(cmd *cmd){
	if (cmd->nargs > 1){
		printf("smash error: quit: expected 0 or 1 arguments\n");
		return SMASH_FAIL;
	} else if (cmd->nargs == 0){
		return SMASH_QUIT;
	} else {
		if (strcmp(cmd->args[1], "kill") != 0){
			printf("smash error: quit: unexpected argument\n");
			return SMASH_FAIL;
		}
		// handle quit with kill
		for (int i = 0; i < JOBS_NUM_MAX; i++){ 
			if (jobLookup(i)){
				struct job *job = globals->jobList->jobs[i];
				printf("[%d] %s - ", i, job->cmd);
				fflush(stdout);
				kill(job->pid, SIGTERM);
				printf("sending SIGTERM... ");
				fflush(stdout);
				sleep(5);
				// use "dynamic" check during 5 seconds or "static" after them?
				if (waitpid(obList_t->jobs[i]->pid, WNOHANG) == 0){ // job still running
					kill(job->pid, SIGKILL);
					printf("sending SIGKILL... ");
					fflush(stdout);
				}
				printf("done\n");
			}
		}
	}
	return SMASH_QUIT;
}

int diff(cmd *cmd){
	if (args_num_error(cmd, 2) == INVALID_COMMAND){
		return SMASH_FAIL;
	}
	// check if paths are valid and files
	struct stat st;
	struct stat st2;
	stat(cmd->args[1], &st);
	if(stat(cmd->args[1], &st) == -1 || stat(cmd->args[2], &st2) == -1){
		printf("smash error: diff: expected valid paths for files\n");
		return SMASH_FAIL;
	}
	if(!S_ISREG(st.st_mode) || !S_ISREG(st2.st_mode)){
		printf("smash error: diff: paths are not files\n");
		return SMASH_FAIL;
	}
	// diff files
	int id = jobPIDLookup(getpid())->ID;
	if (globals->file1[id]) fclose(globals->file1[id]);
	if (globals->file2[id]) fclose(globals->file2[id]);
	globals->file1[id] = fopen(cmd->args[1], "r");
	if (!globals->file1[id]){
		perrorSmash("diff", "failed to open file 1");
		return SMASH_FAIL;
	}
	globals->file2[id] = fopen(cmd->args[2], "r");
	if (!globals->file2[id]){
		perrorSmash("diff", "failed to open file 2");
		fclose(globals->file1[id]);
		return SMASH_FAIL;
	}
	char line1[DIFF_LINE_SIZE]; // array for easier memory management
	char line2[DIFF_LINE_SIZE];
	while (true){
		char *ptr1 = fgets(line1, DIFF_LINE_SIZE, globals->file1[id]);
		char *ptr2 = fgets(line2, DIFF_LINE_SIZE, globals->file2[id]);
		if (!ptr1 || !ptr2){ // error or EOF
			if (ptr1 || ptr2){ //not finished together
				break;
			} else if (line1[0] && line2[0] && strcmp(line1, line2)){ // diff, last line till EOF
				break;
			} else if (feof(globals->file1[id]) && feof(globals->file2[id])){
				printf("0");
				fclose(globals->file1[id]);
				fclose(globals->file2[id]);
				globals->file1[id] = NULL;
				globals->file2[id] = NULL;
				return SMASH_SUCCESS;
			}

		} else if (strcmp(line1,line2)){ // found a diff
			break;
		} 
	}
	printf("1");
	fclose(globals->file1[id]);
	fclose(globals->file2[id]);
	globals->file1[id] = NULL;
	globals->file2[id] = NULL;
	return SMASH_FAIL;

}

/**
 * @brief Run an external command using fork, execv and wait if in fg.
 * @return End status - success or failure
 */
int run_ext_cmd(cmd *cmd){
	int is_file = access(cmd->command, F_OK);
	if (is_file == -1){ // file not found
		printf("smash error: external: cannot find program\n");
		return SMASH_FAIL;
	}
	int id = fork();
	if (id==0){
		int ret = execv(cmd->command, cmd->args);
		if (ret ==-1) printf("smash error: external: invalid command\n");
		return SMASH_FAIL;
	} else {
		if (cmd->status == FOREGROUND){
			waitpid(id, NULL, 0);
		}
	}
	return SMASH_SUCCESS;
}

int commandPID(cmd *cmd){
	return getpid();
}