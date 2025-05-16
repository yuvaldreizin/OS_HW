#ifndef ATM_H
#define ATM_H

#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "account.h"
#include <utils.h>
#define ARGS_NUM_MAX 4

typedef enum
{
    NONE,
    REQUEST,
    APPROVED
} delete_status;

struct delete_request
{
    int source_id;
    delete_status status;
};
typedef struct delete_request delete_request_t;
struct atm
{
    int id;
    char * file;
    delete_request_t delete_req;
};
typedef struct *atm atm_t;

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

atm_t atm_init(int id, char * file);
void destroy_atm(atm_t atm);
command_t read_next_command(atm_t atm);
fstatus_t execute_command(atm_t atm, char *cmd);
void run_atm(atm_t atm);
void delete_atm(int target_id, int source_id);


#endif // ATM_H