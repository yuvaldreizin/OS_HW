#define _POSIX_C_SOURCE 200809L
#include "./utils.h"

void global_init(){
    globals = MALLOC_VALIDATED(globals_t, sizeof(globals_t));
    globals->accounts = linked_list_init();
    globals->atms = NULL;
    globals->account_lock = rwlock_init();
    globals->atm_lock = rwlock_init();
    globals->delete_lock = rwlock_init();
    globals->finished_lock = rwlock_init();
    globals->num_atms = 0;
    globals->bank_account = account_init(0, "0000", 0); // bank account
    globals->log_lock = rwlock_init();
    // remove existing log file if it exists
    if (access(LOG_FILE_NAME, F_OK) != -1) {
        remove(LOG_FILE_NAME);
    }
    globals->log_file = fopen(LOG_FILE_NAME, "w");
    globals->atm_threads = NULL;
    globals->bank_thread = NULL;
    globals->delete_requests = linked_list_init();
    globals->finished = 0;
}

void global_free(){
    if (globals == NULL) return;
    // no locks since all threads ended
    linked_list_free(globals->accounts, account_free);
    linked_list_free(globals->delete_requests, free);
    for (int i = 0; i < globals->num_atms; i++) {
        if (globals->atms[i] != NULL){
            destroy_atm(globals->atms[i]);
        }
        if (globals->atm_threads[i] != NULL) {
            free(globals->atm_threads[i]);
            globals->atm_threads[i] = NULL;
        }
    }
    free(globals->atms);
    rwlock_destroy((globals->log_lock));
    rwlock_destroy((globals->atm_lock));
    rwlock_destroy((globals->account_lock));
    rwlock_destroy((globals->delete_lock));
    if (globals->bank_account != NULL) {
        account_free(globals->bank_account);
        globals->bank_account = NULL;
    }
    if (globals->atm_threads != NULL) {
        free(globals->atm_threads);
        globals->atm_threads = NULL;
    }
    if (globals->bank_thread != NULL) {
        free(globals->bank_thread);
        globals->bank_thread = NULL;
    }
    if (globals->log_file != NULL) {
        fclose(globals->log_file);
        globals->log_file = NULL;
    }
    free(globals);
    globals = NULL;
}

void log_lock(){
    rwlock_acquire_write((globals->log_lock));
}
void log_unlock(){
    rwlock_release_write((globals->log_lock));
}