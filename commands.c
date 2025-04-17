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
int parseCmd(char* line, struct cmd *command){
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

void destroyCmd(struct cmd* cmd){
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

int isInternalCommand(struct cmd *cmd){
	//check if command is internal or external
	for (int i = 0; i < NUM_INTERNAL_COMMANDS; i++)
	{
		if (strcmp(cmd->command, internal_command_t[i].command) == 0)
			return i; 
	}
	return -1; // External command
}

int run_cmd(struct cmd* cmd){
	if (!cmd) return 0;
	int index = isInternalCommand(cmd);
	if (index != -1){
		internal_command_t[index].func(cmd); // Call the command function
		return 0;
	} else {	// Add External command handle
		return -1;
	}
}

int showpid(struct cmd *cmd){
	if (cmd->nargs > 1){
		// handle error
		// smash error: showpid: expected 0 arguments
		return INVALID_COMMAND;
	} else {
		if (cmd->status == FOREGROUND && cmd->env == INTERNAL){	// running in smash process
			printf("smash pid is %d\n", getpid());
		} else {												// running in child-process
			printf("smash pid is %d\n", getppid());				// TODO - compiling error, not sure why yet
		}
		return VALID_COMMAND;
	}
}

int pwd(struct cmd *cmd){
	char pwd[MAX_PATH_SIZE];
	if (cmd->nargs == 1 && getcwd(pwd, sizeof(pwd)) != NULL){
		printf("%s\n", pwd);
		return VALID_COMMAND;
	} 
	// handle error
	// smash error: showpid: expected 0 arguments
	return INVALID_COMMAND;
}

