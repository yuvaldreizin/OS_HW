#ifndef ATM_H
#define ATM_H

#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "account.h"
#include <utils.h>
#define ARGS_NUM_MAX 4

struct atm
{
    int id;
    char * file;
};
typedef struct *atm atm_t;

struct command
{
    char type;
    int args[ARGS_NUM_MAX];
};

typedef struct command *command_t;

atm_t atm_init(int id, char * file);
void destroy_atm(atm_t atm);
command_t read_next_command(atm_t atm);
fstatus_t execute_command(atm_t atm, char *cmd);
void run_atm(atm_t atm);


#endif // ATM_H