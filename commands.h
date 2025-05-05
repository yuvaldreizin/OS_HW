#ifndef COMMANDS_H
#define COMMANDS_H
/*=============================================================================
* includes, defines, usings
=============================================================================*/
#include <stdlib.h>
#include <stdio.h>
#include "jobs.h"
#include "signals.h"
#include <unistd.h>
#include "utils.h"
#include "sys/stat.h"
#include <errno.h>

#define CMD_LENGTH_MAX 120
#define ARGS_NUM_MAX 20
#define JOBS_NUM_MAX 100
#define DIFF_LINE_SIZE 500
#define FIRST_ARG 1
#define SECOND_ARG 2

/**
 * @enum environment
 * @brief Represents if the command is internal or external.
 */
typedef enum {
    INTERNAL = 1,
    EXTERNAL
} environment;

/**
 * @struct cmd
 * @brief Represents a command with its arguments and status (internal/external and background).
 */
 struct cmdData {
    char* input;          /**< The command itself. */
    char** args;            /**< List of command's atguments. */
    int nargs;              /**< Number of arguments. */
    jobStatus cmdStatus;       /**< Current status of the command. */
    environment env;        /**< Command's running environment (Internal/External). */
};

typedef struct cmdData cmd_t;
typedef int (*cmd_func)(cmd_t *curr_cmd);

struct internal_command{
    char* command;             /**< Name of the internal command. */
    cmd_func func;             /**< Function pointer to the command's implementation. */
};
typedef struct internal_command internal_command_t;





int args_num_error(cmd_t *curr_cmd, int expected_num);

/*=============================================================================
* error definitions
=============================================================================*/
typedef enum  {
	INVALID_COMMAND = 0,
    VALID_COMMAND
} ParsingError;

typedef enum {
	SMASH_SUCCESS = 0,
	SMASH_QUIT,
	SMASH_FAIL,
    SMASH_NULL
	//feel free to add more values here or delete this
} CommandResult;


/*=============================================================================
* global functions
=============================================================================*/
int parseCmd(char* line, cmd_t **curr_cmd);

void destroyCmd(cmd_t *curr_cmd);

int isInternalCommand(cmd_t *curr_cmd);

int run_cmd(cmd_t *curr_cmd);

int showpid(cmd_t *curr_cmd);
int pwd(cmd_t *curr_cmd);
int cd(cmd_t *curr_cmd);
int jobs(cmd_t *curr_cmd);
int smashKill(cmd_t *curr_cmd);
int fg(cmd_t *curr_cmd);
int bg(cmd_t *curr_cmd);
int quit(cmd_t *curr_cmd);
int diff(cmd_t *curr_cmd);


int run_ext_cmd(cmd_t *curr_cmd);

#endif //COMMANDS_H