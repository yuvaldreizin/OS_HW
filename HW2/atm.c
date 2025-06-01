#define _POSIX_C_SOURCE 200809L
#include "atm.h"

extern int finished;

atm_t atm_init(int id, char * file){
    atm_t new_atm = MALLOC_VALIDATED(struct atm, sizeof(struct atm));
    new_atm->id = id;
    new_atm->file = fopen(file, "r");
    if (new_atm->file == NULL)
    {
        ERROR_EXIT("Error opening file");
    }
    new_atm->delete_req= NULL;
    new_atm->lock = rwlock_init();
    return new_atm;
}

void destroy_atm(atm_t atm)
{
    if (atm != NULL)
    {
        if (atm->delete_req != NULL)
        {
            free(atm->delete_req);
            atm->delete_req = NULL;
        }
        fclose(atm->file);
        rwlock_destroy(atm->lock);
        free(atm);
        atm = NULL;
    }  
}

command_t read_next_command(atm_t atm){
    //read line form file
    char * line = NULL;
    size_t len = 0;
    ssize_t read;
    if (atm->file == NULL)
    {
        ERROR_EXIT("Error reading file");
    }
    read = getline(&line, &len, atm->file); // MEM LEAK HERE
    if (read == -1) {
        if (feof(atm->file)) {
            // End of file reached
            return NULL;
        } else if (ferror(atm->file)) {
            // Some error occurred
            ERROR_EXIT("Read error");
        }
    }
    // ------ RUNTIME BUG - CAN'T GET HERE ------
    // Parse the line into command
    command_t new_command = MALLOC_VALIDATED(struct command, sizeof(struct command));
    new_command->password = NULL; // Initialize password to NULL
	char* delimiters = " \n";
    new_command->type = *(strtok(line, delimiters));
	for(int i = 0; i < ARGS_NUM_MAX; i++)
	{
        char* arg = strtok(NULL, delimiters);
        if(!arg)
            break;
        if (i==1){
            new_command->password = strdup(arg); // store password
        }
        else{
            new_command->args[i] = atoi(arg); // convert string to int
        }
	}
    // Free the line buffer
    free(line);
    return new_command;
}

void execute_command(atm_t atm, command_t cmd)
{
    switch (cmd->type)
    {
        case 'O':
            account_o(cmd->args[0], cmd->password, cmd->args[2], atm->id);
            break;
        case 'D':
            account_d(cmd->args[0], cmd->password, cmd->args[2], atm->id);
            break;
        case 'W':
            account_w(cmd->args[0], cmd->password, cmd->args[2], atm->id);
            break;
        case 'B':
            account_b(cmd->args[0], cmd->password, atm->id);
            break;
        case 'Q':
            account_q(cmd->args[0], cmd->password, atm->id);
            break;
        case 'T':
            account_t(cmd->args[0], cmd->password, cmd->args[2], cmd->args[3], atm->id);
            break;
        case 'C':
            delete_atm(cmd->args[0], atm->id);
            break;
        default:
            ERROR_EXIT("Invalid command");
    }
    // Free the command
    if (cmd->password != NULL) {
        free(cmd->password); // Free the password if it was allocated
    }
    free(cmd);
}

void run_atm(atm_t atm)
{
    command_t cmd;
    struct timespec ts;
    ts.tv_sec = 0;
    ts.tv_nsec = 100000000;  // (0.1 seconds) 
    while (1)
    {
        rwlock_acquire_read((atm->lock));
        if (atm->delete_req != NULL){
            rwlock_release_read((atm->lock));
            break;
        }
        rwlock_release_read((atm->lock));
        nanosleep(&ts, NULL);
        cmd = read_next_command(atm);
        fprintf(stderr, "ATM %d: Read command %c with args: ", atm->id, cmd->type); // MEM LEAK HERE
        for (int i = 0; i < ARGS_NUM_MAX && cmd->args[i] != 0; i++)
        {
            fprintf(stderr, "%d ", cmd->args[i]);
        }
        fprintf(stderr, "\n");
        if (!cmd){
            break;
        }
        execute_command(atm, cmd);
        sleep(1);
    }
    // TODO - lock ATMs struct and specific ATM
    fprintf(stderr, "ATM %d: finished\n", atm->id);
    globals->atms[atm->id] = NULL;
    finished++;
}

void delete_atm(int target_id, int source_id)
{
    if (target_id > globals->num_atms)
    {
        // ATM not found, print error message
        log_lock();
        fprintf(globals->log_file, "Error %d: Your delete operation failed - ATM ID %d does not exist\n", source_id, target_id);
        fflush(globals->log_file);
        log_unlock();
        return;
    }
    delete_request_t *delete_req = MALLOC_VALIDATED(struct delete_request, sizeof(struct delete_request));
    delete_req->source_id = source_id;
    delete_req->target_id = target_id;
    rwlock_acquire_write((globals->delete_lock));
    linked_list_add(globals->delete_requests, delete_req);
    rwlock_release_write((globals->delete_lock));
}

