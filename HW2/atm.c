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
}

void destroy_atm(atm_t atm)
{
    if (atm != NULL)
    {
        fclose(atm->file);
        free(atm);
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
            account_c(cmd->args[0] atm->id);
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
        usleep(100000);
        if ((cmd = read_next_command(atm)) != NULL){
            break;
        }
        execute_command(atm, cmd);
        sleep(1)
    }
    finished++;
    destroy_atm(atm);
}