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


#define CMD_LENGTH_MAX 120
#define ARGS_NUM_MAX 20
#define JOBS_NUM_MAX 100
#define DIFF_LINE_SIZE 500


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
typedef struct cmd {
    char* command;          /**< The command itsel. */
    char** args;            /**< List of command's atguments. */
    int nargs;              /**< Number of arguments. */
    jobStatus status;       /**< Current status of the command. */
    environment env;        /**< Command's running environment (Internal/External). */
} cmd;

typedef int (*cmd_func)(struct cmd *cmd);

typedef struct internal_command {
    char* command;             /**< Name of the internal command. */
    cmd_func func;             /**< Function pointer to the command's implementation. */
} internal_command;





int args_num_error(cmd *cmd, int expected_num);

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
int parseCmd(char* line, cmd *command);

void destroyCmd(cmd *cmd);

int isInternalCommand(cmd *cmd);

int run_cmd(cmd *cmd);

int showpid(cmd *cmd);
int pwd(cmd *cmd);
int cd(cmd *cmd);
int jobs(cmd *cmd);
int smashKill(cmd *cmd);
int fg(cmd *cmd);
int bg(cmd *cmd);
int quit(cmd *cmd);
int diff(cmd *cmd);


internal_command internal_command_t[] = {
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

#define NUM_INTERNAL_COMMANDS (sizeof(internal_command_t) / sizeof(struct internal_command))

int run_ext_cmd(cmd *cmd);

int commandPID(cmd* cmd);

#endif //COMMANDS_H