#define _POSIX_C_SOURCE 200809L
#include "./utils.h"
#include "account.h"
#include "atm.h"
#include <stdbool.h>
#include <time.h>

//=============================================================================
// global variables
globals_t *globals = NULL;
int finished = 0;
//=============================================================================
//functions
void run_bank();
void *run_bank_aux(void *arg);
void *run_atm_aux(void *atm_ptr);
void charge_commission(account *account);
void check_delete_reqs();

int main(int argc, char *argv[])
{
    // globals init
    global_init();
    globals->atm_threads = MALLOC_VALIDATED(pthread_t* ,sizeof(pthread_t*) * (argc)); // zero is not used
    globals->atms = MALLOC_VALIDATED(atm_t, sizeof(atm_t) * (argc)); // zero is not used
    globals->num_atms = argc - 1;
    for (int i = 1; i < argc; i++){
        atm_t new_atm = atm_init(i, argv[i]);
        globals->atms[i] = new_atm;
        globals->atm_threads[i] = MALLOC_VALIDATED(pthread_t ,sizeof(pthread_t));
        if (pthread_create(globals->atm_threads[i], NULL, run_atm_aux, new_atm) != 0)
        {
            ERROR_EXIT("Error creating ATM thread");
        }
    }
    globals->bank_thread = MALLOC_VALIDATED(pthread_t ,sizeof(pthread_t));
    if (pthread_create(globals->bank_thread, NULL, run_bank_aux, NULL) != 0) // MEM LEAK HERE - during alloc
    {
        ERROR_EXIT("Error creating bank thread");
    }
    for (int i = 1; i < argc; i++){
        pthread_join(*globals->atm_threads[i], NULL);
    }
    pthread_join(*globals->bank_thread, NULL);
    global_free();
}

void *run_atm_aux(void *atm_ptr)
{
    atm_t atm = (atm_t)atm_ptr;
    run_atm(atm);
    return NULL;
}

void *run_bank_aux(void *arg)
{
    run_bank();
    return NULL;
}


void run_bank(){
    int counter = 0;
    struct timespec ts;
    ts.tv_sec = 0;
    ts.tv_nsec = 500000000;  // (0.5 seconds)
    while(1){
        rwlock_acquire_read(globals->atm_lock);
        if (finished == globals->num_atms){
            rwlock_release_read(globals->atm_lock);
            break;
        }
        rwlock_release_read(globals->atm_lock);
        nanosleep(&ts, NULL);
        check_delete_reqs();
        counter++;
        lock_all_accounts();    // write lock on all
        printf("\033[2J");
        printf("\033[1:1H");
        // go over all accounts
        Node *l;
        for (l = globals->accounts->head; l != NULL; l = l->next)
        {
            account *acnt = ((account_with_id *)(l->data))->acc;
            account_print(acnt);
            if (counter == 6){ // every 3 seconds
                charge_commission(acnt);
            }
        }
        counter = counter % 6;
        unlock_all_accounts();
    }
}

void charge_commission(account *account){
    // generate a random number
    int precentage = rand() % 4 + 1;
    int commission = account->balance * precentage / 100;
    account->balance -= commission;
    globals->bank_account->acc->balance += commission;
    fprintf(globals->log_file ,"Bank: commissions of %d %% were charged, bank gained %d from account %d\n",
        precentage, commission, account->id);
    fflush(globals->log_file);
}

void check_delete_reqs(){
    // check if any ATM is marked for deletion
    // rwlock_acquire_write((globals->delete_lock)); //not needed. worst case is another node during for loop.
    Node *l;
    for (l = globals->delete_requests->head; l != NULL; l = l->next)
    {
        delete_request_t *curr_delete_req = (delete_request_t *)l->data;
        atm_t target_atm = globals->atms[curr_delete_req->target_id];
        bool added = false;
        if (target_atm){
            rwlock_acquire_write((target_atm->lock));
            if (target_atm->delete_req == NULL) // atm isn't marked for deletion
            {
                target_atm->delete_req = curr_delete_req;
                added = true;
            }
            rwlock_release_write((target_atm->lock));
        }
        if (!added){
            log_lock();
            fprintf(globals->log_file, "Error %d: Your close operation failed - ATM ID %d is already in a closed state\n",
                 curr_delete_req->source_id, curr_delete_req->target_id);
            fflush(globals->log_file);
            log_unlock();
        }
        rwlock_acquire_write((globals->delete_lock));
        linked_list_remove(globals->delete_requests, curr_delete_req);
        rwlock_release_write((globals->delete_lock));
        // free(curr_delete_req); // freed with atm
    }
    for (int i = 1; i <= globals->num_atms; i++){
        atm_t atm = globals->atms[i];
        if (atm && atm->delete_req != NULL){
            // ATM is marked for deletion
            delete_request_t *atm_delete_req = atm->delete_req;
            pthread_join(*globals->atm_threads[i], NULL);
            destroy_atm(atm);
            globals->atms[i] = NULL;
            log_lock();
            fprintf(globals->log_file, "Bank: ATM %d closed %d successfully\n", atm_delete_req->source_id ,atm_delete_req->target_id);
            fflush(globals->log_file);
            log_unlock();
        }
    }
}