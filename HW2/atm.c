#include "atm.h"

extern int finished;

atm_t atm_init(int id, char * file)
{
    atm_t new_atm = MALLOC_VALIDATED(atruct atm, sizeof(struct atm));
    new_atm->id = id;
    new_atm->file = file;
    fopen(new_atm->file, "r");
    if (new_atm->file == NULL)
    {
        ERROR_EXIT("Error opening file");
    }
    new_atm->delete_req= NULL;
    rwlock_init(&(new_atm->lock));
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
    read = getline(&line, &len, atm->file);
    if (read == -1) {
        if (feof(atm->file)) {
            // End of file reached
            return NULL;
        } else if (ferror(atm->file)) {
            // Some error occurred
            ERROR_EXIT("Read error");
        }
    }
    // Parse the line into command
    command_t new_command = MALLOC_VALIDATED(struct command, sizeof(struct command));
	char* delimiters = " \n";
    new_command->type = *(strtok(line, delimiters));
	for(int i = 0; i < ARGS_NUM_MAX; i++)
	{
        char* arg = strtok(NULL, delimiters);
        if(!arg)
            break;
        new_command->args[i] = atoi(arg); // convert string to int
	}
    // Free the line buffer
    free(line);
    return new_command;
}

void execute_command(atm_t atm, char *cmd)
{
    switch (cmd->type)
    {
        case 'O':
            account_o(cmd->args[0], cmd->args[1], cmd->args[2], atm->id);
            break;
        case 'D':
            account_d(cmd->args[0], cmd->args[1], cmd->args[2], atm->id);
            break;
        case 'W':
            account_w(cmd->args[0], cmd->args[1], cmd->args[2], atm->id);
            break;
        case 'B':
            account_b(cmd->args[0], cmd->args[1], atm->id);
            break;
        case 'Q':
            account_q(cmd->args[0], cmd->args[1], atm->id);
            break;
        case 'T':
            account_t(cmd->args[0], cmd->args[1], cmd->args[2], c, atm->id);
            break;
        case 'C':
            delete_atm(cmd->args[0], atm->id);
            break;
        default:
            ERROR_EXIT("Invalid command");
    }
    // Free the command
    free(command);
}

void run_atm(atm_t atm)
{
    command_t cmd;
    while (1)
    {
        rwlock_acquire_read(&(atm->lock));
        if (atm->delete_req != NULL){
            rwlock_release_read(&(atm->lock));
            break;
        }
        rwlock_release_read(&(atm->lock));
        usleep(100000);
        if ((cmd = read_next_command(atm)) != NULL){
            break;
        }
        execute_command(atm, cmd);
        sleep(1)
    }
    // TODO - lock ATMs struct and specific ATM
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
        log_unlock();
        return;
    }
    delete_request_t *delete_req = MALLOC_VALIDATED(struct delete_request, sizeof(struct delete_request));
    delete_req->source_id = source_id;
    delete_req->target_id = target_id;
    rwlock_acquire_write(&(globals->delete_lock));
    globals->delete_requests = g_list_append(globals->delete_requests, delete_req);
    rwlock_release_write(&(globals->delete_lock));
}