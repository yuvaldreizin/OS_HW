#ifndef ATM_H
#define ATM_H

#include "./utils.h"
#include <unistd.h>
#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "account.h"
#include <time.h>

#define ARGS_NUM_MAX 4
struct command
{
    char type;
    int args[ARGS_NUM_MAX];
};

typedef enum
{
    NONE,
    REQUEST,
    APPROVED
} delete_status;

typedef struct command *command_t;

// extern globals_t *globals;

atm_t atm_init(int id, char * file);
void destroy_atm(atm_t atm);
command_t read_next_command(atm_t atm);
void execute_command(atm_t atm, command_t cmd);
void run_atm(atm_t atm);
void delete_atm(int target_id, int source_id);


#endif // ATM_H