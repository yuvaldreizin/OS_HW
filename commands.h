#ifndef COMMANDS_H
#define COMMANDS_H
/*=============================================================================
* includes, defines, usings
=============================================================================*/
#include <stdlib.h>
#include <stdio.h>
#include "jobs.h"
#include <unistd.h>
#include "utils.h"

#define CMD_LENGTH_MAX 120
#define ARGS_NUM_MAX 20
#define JOBS_NUM_MAX 100


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

#define MAX_PATH_SIZE 200   // ASSUMPTION - is there a max path size? PWD seems the best way to get pwd but requires a buffer size.



/*=============================================================================
* error definitions
=============================================================================*/
typedef enum  {
	INVALID_COMMAND = 0,
    VALID_COMMAND = 1
} ParsingError;

typedef enum {
	SMASH_SUCCESS = 0,
	SMASH_QUIT,
	SMASH_FAIL
	//feel free to add more values here or delete this
} CommandResult;

/*=============================================================================
* global functions
=============================================================================*/
int parseCmd(char* line, struct cmd *command);

void destroyCmd(struct cmd* cmd);

int isInternalCommand(struct cmd* cmd);

int run_cmd(struct cmd* cmd);

int showpid(struct cmd* cmd);
int pwd(struct cmd* cmd);
int cd(struct cmd* cmd);
int jobs(struct cmd* cmd);
int kill(struct cmd* cmd);
int fg(struct cmd* cmd);
int bg(struct cmd* cmd);
int quit(struct cmd* cmd);
int diff(struct cmd* cmd);


internal_command internal_command_t[] = {
    {"showpid", showpid},
    {"pwd", pwd},
    {"cd", cd},
    {"jobs", jobs},
    {"kill", kill},
    {"fg", fg},
    {"bg", bg},
    {"quit", quit},
    {"diff", diff}
};

#define NUM_INTERNAL_COMMANDS (sizeof(internal_command_t) / sizeof(struct internal_command))

#endif //COMMANDS_H