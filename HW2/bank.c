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
void check_delete_reqs();

int main(int argc, char *argv[])
{
    // globals init
    global_init();
    globals->atm_threads = MALLOC_VALIDATED(pthread_t ,sizeof(pthread_t) * (argc)); // zero is not used
    globals->atms = MALLOC_VALIDATED(struct atm, sizeof(struct atm) * (argc)); // zero is not used
    globals->num_atms = argc - 1;
    for (int i = 1; i < argc; i++){
        atm_t new_atm = atm_init(i, argv[i]);
        globals->atms[i] = new_atm;
        if (pthread_create(&globals->atm_threads[i], NULL, run_atm, new_atm) != 0)
        {
            ERROR_EXIT("Error creating ATM thread");
        }
    }
    globals->bank_thread = MALLOC_VALIDATED(pthread_t ,sizeof(pthread_t));
    if (pthread_create(&globals->bank_thread, NULL, run_bank, NULL) != 0)
    {
        ERROR_EXIT("Error creating bank thread");
    }
    for (int i = 1; i < argc; i++){
        pthread_join(globals->atm_threads[i], NULL);
    }
    pthread_join(globals->bank_thread, NULL);
    global_free();
}

void run_bank(){
    int counter = 0;
    while(finished < globals->num_atms){
        usleep(500000);
        check_delete_reqs();
        counter++;
        lock_all_accounts();    // split to read write in comment if loack_all for read is needed (default is write for this instance)
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

void check_delete_reqs(){
    // check if any ATM is marked for deletion
    rwlock_acquire_write(&(globals->delete_lock));
    GList *l;
    for (l = globals->delete_requests; l != NULL; l = l->next)
    {
        delete_request_t *curr_delete_req = (delete_request_t *)l->data;
        int target_atm = globals->atms[curr_delete_req->target_id];
        bool added = false;
        if (target_atm){
            rwlock_acquire_write(&(target_atm->lock));
            if (target_atm->delete_req == NULL) // atm isn't marked for deletion
            {
                target_atm->delete_req = curr_delete_req;
                added = true;
            }
            rwlock_release_write(&(target_atm->lock));
        }
        if (!added){
            log_lock();
            fprintf(globals->log_file, "Error %d: Your close operation failed - ATM ID %d is already in a closed state\n", globals->delete_requests[i].source_id, i);
            log_unlock();
        }
        rwlock_acquire_write(&(globals->delete_lock));
        globals->delete_requests = g_list_remove(globals->delete_requests, curr_delete_req);
        rwlock_release_write(&(globals->delete_lock));
        free(curr_delete_req);
    }
    for (int i = 1; i <= globals->num_atms; i++){
        atm_t atm = globals->atms[i];
        if (atm && atm->delete_req != NULL){
            // ATM is marked for deletion
            delete_request_t *atm_delete_req = atm->delete_req;
            pthread_join(globals->atm_threads[i], NULL);
            destroy_atm(atm);
            globals->atms[i] = NULL;
            log_lock();
            fprintf(globals->log_file, "Bank: ATM %d closed %d successfully\n", atm_delete_req->source_id ,atm_delete_req->target_id);
            log_unlock();
        }
    }
}