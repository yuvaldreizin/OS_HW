#include "utils.h"
#include "account.h"
#include "atm.h"

//=============================================================================
// global variables
globals_t *globals = NULL;
int finished = 0;
//=============================================================================
//functions
void run_bank();
void charge_commission(account_t *account);
void check_atms();

int main(int argc, char *argv[])
{
    // globals init
    global_init();
    globals->atm_threads = MALLOC_VALIDATED(pthread_t ,sizeof(pthread_t) * (argc - 1));
    for (int i = 1; i < argc; i++){
        atm_t atm = atm_init(i, argv[i]);
        globals->atms = g_list_append(globals->atms, atm);
        globals->num_atms++;
        if (pthread_create(&globals->atm_threads[i-1], NULL, run_atm, atm) != 0)
        {
            ERROR_EXIT("Error creating ATM thread");
        }
    }
    globals->bank_thread = MALLOC_VALIDATED(pthread_t ,sizeof(pthread_t));
    if (pthread_create(&globals->bank_thread, NULL, run_bank, NULL) != 0)
    {
        ERROR_EXIT("Error creating bank thread");
    }
    for (int i = 0; i < argc - 1; i++){
        pthread_join(globals->atm_threads[i], NULL);
    }
    pthread_join(globals->bank_thread, NULL);
    free(atm_threads);
    global_free();

}

void run_bank(){
    int counter = 0;
    while(finished < globals->num_atms){
        usleep(500000);
        check_atms();
        counter++;
        lock_all_accounts();
        printf("\033[2J]");
        printf("\033[1:1H");
        // go over all accounts
        GList *l;
        for (l = globals->accounts; l != NULL; l = l->next)
        {
            account_t *account = (account_t *)l->data;
            account_print(account);
            if (counter == 6){ // every 3 seconds
                charge_commission(account);

            }
        }
        counter = counter % 6;
        unlock_all_accounts();
    }
}

void charge_commission(account_t *account){
    // generate a random number
    int precentage = rand() % 4 + 1;
    int commission = account->balance * precentage / 100;
    account->balance -= commission;
    globals->bank_account->balance += commission;
    fprintf(globals->log_file ,"Bank: commissions of %d %% were charged, bank gained %d from account %d\n",
        precentage, commission, account->id);
}

void check_atms(){
    // check if any ATM is marked for deletion
    GList *l;
    for (l = globals->atms; l != NULL; l = l->next)
    {
        atm_t atm = (atm_t *)l->data;
        if (atm->delete_req.status == REQUEST)
        {
            // ATM found, mark for deletion
            atm->delete_req.status = APPROVED;
            pthread_join(globals->atm_threads[atm->id - 1], NULL); // wait for ATM to finish command and close
            log_lock();
            fprintf(globals->log_file, "Bank: ATM %d closed %d successfully\n", atm->delete_req.source_id ,atm->id);
            log_unlock();
        }
    }
}