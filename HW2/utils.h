#ifndef UTILS_H
#define UTILS_H

#include <glib.h>
#include <unistd.h>

/*=============================================================================
* error handling - some useful macros and examples of error handling,
* feel free to not use any of this
=============================================================================*/

#define ERROR_EXIT(msg) \
    do { \
        fprintf(stderr, "%s: %d\n%s", __FILE__, __LINE__, msg); \
        exit(1); \
    } while(0);

static inline void* _validatedMalloc(size_t size)
{
    void* ptr = malloc(size);
    if(!ptr) ERROR_EXIT("malloc");
    return ptr;
}

#define MALLOC_VALIDATED(type, size) \
    ((type*)_validatedMalloc((size)))


/*=============================================================================
* ENUMS
=============================================================================*/

typedef enum {
    SUCCESS = 0,
    FAILURE = -1,
    INVALID_ID = -2,
} f_status_t;

/*=============================================================================
* GLOBALS
=============================================================================*/

struct globals {
    GList *accounts;
    GList *delete_requests;
    atm_t *atms;
    int num_accounts;
    int num_atms;
    account *bank_account; // the bank account
    rwlock_t account_lock;
    rwlock_t atm_lock;
    rwlock_t log_lock;
    rwlock_t delete_lock;
    char *log_file;
    pthread_t *atm_threads;
    pthread_t *bank_thread;
};

typedef struct globals globals_t;
extern globals_t *globals;

void global_init(){
    globals = MALLOC_VALIDATED(globals_t, sizeof(globals_t));
    globals->accounts = NULL;
    globals->atms = NULL;
    globals->num_accounts = 0;
    rwlock_init(&(globals->account_lock));
    rwlock_init(&(globals->atm_lock));
    rwlock_init(&(globals->delete_lock));
    globals->num_atms = 0;
    globals->bank_account = account_init(0, 0, 0); // bank account
    rwlock_init(&(globals->log_lock));
    globals->log_file = "log.txt";
    // remove existing log file if it exists
    if (access(globals->log_file, F_OK) != -1) {
        remove(globals->log_file);
    }
    FILE *log_file = fopen(globals->log_file, "w");
    fclose(log_file);
    globals->atm_threads = NULL;
    globals->bank_thread = NULL;
    globals->delete_requests = NULL;
}

void global_free(){
    if (globals == NULL) return;
    // TODO - Free all accounts
    g_list_free_full(globals->accounts, free);
    g_list_free_full(globals->delete_requests, free);
    // TODO - Free all ATMs
    for (int i = 0; i < globals->num_atms; i++) {
        destroy_atm(globals->atms[i]);
    }
    rwlock_destroy(&(globals->log_lock));
    rwlock_destroy(&(globals->atm_lock));
    rwlock_destroy(&(globals->account_lock));
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
        free(globals->log_file);
        globals->log_file = NULL;
    }
    free(globals);
    globals = NULL;
}

void log_lock(){
    rwlock_acquire_write(&(globals->log_lock));
}
void log_unlock(){
    rwlock_release_write(&(globals->log_lock));
}

#endif // UTILS_H