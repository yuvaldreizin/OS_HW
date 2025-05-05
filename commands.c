//commands.c
#include "commands.h"

internal_command_t commands_list[] = {
    {"showpid", showpid},
    {"pwd", pwd},
    {"cd", cd},
    {"jobs", jobs},
    {"kill", smashKill},
    {"fg", fg},
    {"bg", bg},
    {"quit", quit},
    {"diff", diff}
};

#define NUM_INTERNAL_COMMANDS (sizeof(commands_list) / sizeof(internal_command_t))



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
int parseCmd(char* line, cmd_t **curr_cmd){
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

	*curr_cmd = MALLOC_VALIDATED(cmd_t, sizeof(cmd_t));
	(*curr_cmd)->input = MALLOC_VALIDATED(char, (strlen(recieved_cmd) + 1));
	strcpy((*curr_cmd)->input, recieved_cmd);
	if (strcmp(args[nargs],"&") == 0){
		(*curr_cmd)->cmdStatus = BACKGROUND;
		nargs--;	
	} else (*curr_cmd)->cmdStatus = FOREGROUND;
	(*curr_cmd)->args = MALLOC_VALIDATED(char*, nargs * sizeof(char*));
	for (int i = 0; i <= nargs; i++){ // include the command name
		(*curr_cmd)->args[i] = MALLOC_VALIDATED(char, (strlen(args[i]) + 1));
		strcpy((*curr_cmd)->args[i], args[i]);
	}
	(*curr_cmd)->nargs = nargs;
	// (*curr_cmd)->cmdStatus = (strcmp(args[nargs],"&") == 0) ? BACKGROUND : FOREGROUND;
	(*curr_cmd)->env = (isInternalCommand((*curr_cmd)) >= 0) ? INTERNAL : EXTERNAL;

	return VALID_COMMAND; 
}

/**
 * @brief Frees the memory of a command structure.
 */
void destroyCmd(cmd_t* desCmd){
	if (desCmd){
		if(desCmd->args){
			for (int i = 0; i < desCmd->nargs; i++){
				if(desCmd->args[i]) free(desCmd->args[i]);
			}
			free(desCmd->args);
		}
		if(desCmd->input) free(desCmd->input);
		free(desCmd);
	}
}

/**
 * @brief Checks if a given command is an internal command.
 * @return The index of the internal command if found, or -1 if the command is external.
 */
int isInternalCommand(cmd_t *curr_cmd){
	//check if command is internal or external
	for (int i = 0; i < NUM_INTERNAL_COMMANDS; i++){
		if (strcmp(curr_cmd->input, commands_list[i].command) == 0)
			return i; 
	}
	return -1; // External command
}

/**
 * @brief run the given command, handle both internal and external commands
 * @return End status - success or failure
 */
int run_cmd(cmd_t *curr_cmd){
	if (!curr_cmd) return SMASH_FAIL;
	if (curr_cmd->env == INTERNAL){
		int index = isInternalCommand(curr_cmd); // get index
		return commands_list[index].func(curr_cmd); 
	} else { // EXTERNAL
		return run_ext_cmd(curr_cmd);
	}
}

/**
 * @brief Check if the number of arguments is as expected. Print error if not.
 * @return 0 if the number of arguments is correct, -1 otherwise.
 */
int args_num_error(cmd_t *curr_cmd, int expected_num){
    if (curr_cmd->nargs != expected_num){
        printf("smash error: %s: expected %d arguments\n", curr_cmd->input, expected_num);
		return INVALID_COMMAND;
	}
    return VALID_COMMAND;
}

/**
 * @brief print smash pid
 * @return end status
 */
int showpid(cmd_t *curr_cmd){
	if (args_num_error(curr_cmd, 0) == INVALID_COMMAND){
		return SMASH_FAIL;
	} else {
		if (curr_cmd->cmdStatus == FOREGROUND && curr_cmd->env == INTERNAL){	// running in smash process
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
int pwd(cmd_t *curr_cmd){ 
	if (args_num_error(curr_cmd, 0) == INVALID_COMMAND){
		return SMASH_FAIL;
	}
	if (curr_cmd->cmdStatus == BACKGROUND && curr_cmd->env == INTERNAL){	// running in smash process
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
		int id = jobPIDLookup(getpid());
		if (globals->pwd_pointers[id]) free(globals->pwd_pointers[id]);

		globals->pwd_pointers[id] = getcwd(NULL, 0); 
		if (globals->pwd_pointers[id]){
			printf("%s\n", globals->pwd_pointers[id]);
			free(globals->pwd_pointers[id]);
			globals->pwd_pointers[id] = NULL;
			return SMASH_SUCCESS;
		} 
		perror("smash error: pwd: getcwd failed");
		return SMASH_FAIL;
	}
}

int cd(cmd_t *curr_cmd){
	if (args_num_error(curr_cmd, 1) == INVALID_COMMAND){
		return SMASH_FAIL;
	}
	if (!globals->cur_path) { // set path the first time cd is called
		globals->cur_path = getcwd(NULL, 0);
	}
	if (strcmp(curr_cmd->args[FIRST_ARG], "-") == 0){ // case: "-" (go to last path)
		if(!globals->last_path){
			printf("smash error: cd: old pwd not set\n");
			return SMASH_FAIL;
		} else {
			if (globals->cur_path) {
				free(globals->cur_path);
				globals->cur_path = NULL;
			}
			globals->cur_path = globals->last_path;
			globals->last_path = getcwd(NULL, 0);
			chdir(globals->cur_path);
			return SMASH_SUCCESS;
		}
	} else { // case: general cd (including ".." as valid chdir argument)
		int res = chdir(curr_cmd->args[FIRST_ARG]); 
		if (res == -1){
			if (errno == ENOENT){
				printf("smash error: cd: target directory does not exist\n");
				return SMASH_FAIL;
			} else if (errno = ENOTDIR){
				printf("smash error: cd: %s: not a directory\n", curr_cmd->args[FIRST_ARG]);
				return SMASH_FAIL;
			} else {
				perror("smash error: cd: chdir failed");
				return SMASH_FAIL;
			}
		} else { // good chdir
			if (globals->last_path) {
				free(globals->last_path);
				globals->last_path = NULL;
			}
			globals->last_path = globals->cur_path;
			globals->cur_path = getcwd(NULL, 0);
			return SMASH_SUCCESS;
		}
	}
	return SMASH_FAIL;
}

/**
 * @brief print all jobs in the job list
 * @return end status
 */
int jobs(cmd_t *curr_cmd){
	if (args_num_error(curr_cmd, 0) == INVALID_COMMAND){
		return SMASH_FAIL;
	}
	printf("smash: jobs count: %d\n", globals->jobList->count);
	printJobList(globals->jobList);
	return SMASH_SUCCESS;
}

/**
 * @brief send signal to job with given jobID
 * @return end status
 */
int smashKill(cmd_t *curr_cmd){
	if (curr_cmd->nargs != 2 || 
		(int)(strtol(curr_cmd->args[FIRST_ARG], NULL, 10) < 0) || 
		(int)(strtol(curr_cmd->args[SECOND_ARG], NULL, 10) > JOBS_NUM_MAX) || 
		(int)(strtol(curr_cmd->args[SECOND_ARG], NULL, 10) < 0)){ 
		printf("smash error: cd: invalid arguments\n");
		return SMASH_FAIL;
	} 
	int signum = (int)(strtol(curr_cmd->args[FIRST_ARG], NULL, 10));
	int jobID = (int)(strtol(curr_cmd->args[SECOND_ARG], NULL, 10));

	if (!jobLookup(jobID))
		printf("smash error: kill: job id %d does not exist\n",jobID);
	sendSignal(signum, jobID);
	return SMASH_SUCCESS;
}

/**
 * @brief bring job to foreground and wait for it to finish. if no id is given, take max available jobID
 * @return end status
 */
int fg(cmd_t *curr_cmd){
	unsigned int jobID;
	if (curr_cmd->nargs > 1){
		printf("smash error: fg: invalid arguments\n");
		return SMASH_FAIL;
	} else if (curr_cmd->args[FIRST_ARG] == NULL){
		if (maxAvailableJobID() == -1){
			printf("smash error: fg: jobs list is empty\n");
			return SMASH_FAIL;
		} 
		jobID = maxAvailableJobID();
	} else {
		jobID = (unsigned int)(strtol(curr_cmd->args[FIRST_ARG], NULL, 10));
		if (!jobLookup(jobID)){
			printf("smash error: fg: job id %d does not exist\n",jobID);
			return SMASH_FAIL;
		}
	}

	job_t curr_job = globals->jobList->jobs[jobID];
	if (getStatus(curr_job) == STOPPED){
		kill(curr_job->pid, SIGCONT);
	}
	printf("%s", curr_job->user_input);
	waitpid(curr_job->pid, NULL, 0);
	removeJob(jobID);
	return SMASH_SUCCESS;
}

/**
 * @brief bring job to background and continue running. if no id is given, take max stopped jobID
 * @return end status
 */
int bg(cmd_t *curr_cmd){
	int jobID;
	if (curr_cmd->nargs > 1){
		printf("smash error: bg: invalid arguments\n");
		return SMASH_FAIL;
	} else if (curr_cmd->args[FIRST_ARG] == NULL){
		if (maxStoppedJobID() == -1){
			printf("smash error: bg: there are no stopped jobs to resume\n");
			return SMASH_FAIL;
		} 
		jobID = maxStoppedJobID();
	} else {
		jobID = (unsigned int)(strtol(curr_cmd->args[FIRST_ARG], NULL, 10));
		if (!jobLookup(jobID)){
			printf("smash error: bg: job id %d does not exist\n", jobID);
			return SMASH_FAIL;
		} else if (getStatus(globals->jobList->jobs[jobID]) != STOPPED) {
			printf("smash error: bg: job id %d is already in background\n",jobID);
			return SMASH_FAIL;
		}
	}
	job_t curr_job = globals->jobList->jobs[jobID];
	printf("%s: %d", curr_job->user_input, getpid());
	kill(curr_job->pid, SIGCONT);
	return SMASH_SUCCESS;
}

/**
 * @brief quit the shell. if "kill" is given, kill all jobs in the job list
 * @return end status
 */
int quit(cmd_t *curr_cmd){
	if (curr_cmd->nargs > 1){
		printf("smash error: quit: expected 0 or 1 arguments\n");
		return SMASH_FAIL;
	} else if (curr_cmd->nargs == 0){
		return SMASH_QUIT;
	} else {
		if (strcmp(curr_cmd->args[FIRST_ARG], "kill") != 0){
			printf("smash error: quit: unexpected argument\n");
			return SMASH_FAIL;
		}
		// handle quit with kill
		for (int i = 0; i < JOBS_NUM_MAX; i++){ 
			if (globals->jobList->count == 0) break; // no jobs in the list
			if (jobLookup(i)){
				job_t curr_job = globals->jobList->jobs[i];
				printf("[%d] %s - ", i, curr_job->user_input);
				fflush(stdout);
				kill(curr_job->pid, SIGTERM);
				printf("sending SIGTERM... ");
				fflush(stdout);
				sleep(5);
				// use "dynamic" check during 5 seconds or "static" after them?
				if (waitpid(globals->jobList->jobs[i]->pid, NULL, WNOHANG) == 0){ // job still running
					kill(curr_job->pid, SIGKILL);
					printf("sending SIGKILL... ");
					fflush(stdout);
				}
				printf("done\n");
			}
		}
	}
	return SMASH_QUIT;
}

int diff(cmd_t *curr_cmd){
	if (args_num_error(curr_cmd, 2) == INVALID_COMMAND){
		return SMASH_FAIL;
	}
	// check if paths are valid and files
	struct stat st;
	struct stat st2;
	stat(curr_cmd->args[FIRST_ARG], &st);
	if(stat(curr_cmd->args[FIRST_ARG], &st) == -1 || stat(curr_cmd->args[SECOND_ARG], &st2) == -1){
		printf("smash error: diff: expected valid paths for files\n");
		return SMASH_FAIL;
	}
	if(!S_ISREG(st.st_mode) || !S_ISREG(st2.st_mode)){
		printf("smash error: diff: paths are not files\n");
		return SMASH_FAIL;
	}
	// diff files
	int id = jobPIDLookup(getpid());
	if (globals->file1[id]) fclose(globals->file1[id]);
	if (globals->file2[id]) fclose(globals->file2[id]);
	globals->file1[id] = fopen(curr_cmd->args[FIRST_ARG], "r");
	if (!globals->file1[id]){
		perrorSmash("diff", "failed to open file 1\n");
		return SMASH_FAIL;
	}
	globals->file2[id] = fopen(curr_cmd->args[SECOND_ARG], "r");
	if (!globals->file2[id]){
		perrorSmash("diff", "failed to open file 2\n");
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
				printf("0\n");
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
	printf("1\n");
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
int run_ext_cmd(cmd_t *curr_cmd){
	if (!strcmp(curr_cmd->input, "\n")) return SMASH_SUCCESS;
	int is_file = access(curr_cmd->input, F_OK);
	if (is_file == -1){ // file not found
		printf("smash error: external: cannot find program\n");
		return SMASH_FAIL;
	}
	int id = fork();
	if (id==0){
		if (curr_cmd->cmdStatus == FOREGROUND){
			globals->fgJob->pid = getpid();
		}
		int ret = execv(curr_cmd->input, curr_cmd->args);
		if (ret == -1) {
			printf("smash error: external: invalid command\n");
			return SMASH_FAIL;
		}
	} else {
		if (curr_cmd->cmdStatus == FOREGROUND){
			waitpid(id, NULL, 0);
		}
	}
	return SMASH_SUCCESS;
}
