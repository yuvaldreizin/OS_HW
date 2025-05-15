#include "utils.h"
#include "account.h"
#include "atm.h"


globals_t *globals = NULL;
int finished = 0;
void run_bank(){
    int counter = 0;
    while(finished < globals->num_atms){
        usleep(500000);
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
                // generate a random number
                int precentage = rand() % 4 + 1;
                int commission = account_get_balance(account) * precentage / 100;
                account->balance -= commission;
                globals->bank_account->balance += commission;
                fprintf("Bank: commissions of %d %% were charged, bank gained %d from account %d\n",
                    precentage, commission, account_get_id(account));
            }
        }
        counter = counter % 6;
        unlock_all_accounts();
    }
}

int main(int argc, char *argv[])
{
    // globals init
    global_init();
    pthread_t *atm_threads = MALLOC_VALIDATED(pthread_t, sizeof(pthread_t) * (argc - 1)); 
    for (int i = 1; i < argc; i++){
        atm_t atm = atm_init(i, argv[i]);
        globals->atms = g_list_append(globals->atms, atm);
        globals->num_atms++;
        if (pthread_create(&atm_threads[i-1], NULL, run_atm, atm) != 0)
        {
            ERROR_EXIT("Error creating ATM thread");
        }
        pthread_join(atm_threads[i], NULL);
    }
    pthread_t bank_thread;
    if (pthread_create(&bank_thread, NULL, run_bank, NULL) != 0)
    {
        ERROR_EXIT("Error creating bank thread");
    }
    for (int i = 0; i < argc - 1; i++){
        pthread_join(atm_threads[i], NULL);
    }
    free(atm_threads);
    global_free();

}