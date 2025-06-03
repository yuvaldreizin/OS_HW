#ifndef UTILS_H
#define UTILS_H
#define _POSIX_C_SOURCE 200809L

#include <unistd.h>
#include <stdio.h>
#include "linked_list.h"
#include "lock.h"


typedef struct accoun{
    int id;
    char *pass;
    int balance;
    rwlock_t lock;
} account;

typedef struct account_id{
    account *acc;
    int id;
} account_with_id;

struct delete_request
{
    int source_id;
    int target_id;
};
typedef struct delete_request delete_request_t;
struct atm
{
    int id;
    FILE *file;
    delete_request_t *delete_req;
    rwlock_t lock;
};
typedef struct atm *atm_t;


extern account_with_id* account_init(int id, char *pass, int balance);
extern void account_free(void *acc);
extern void destroy_atm(atm_t atm);

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
    FAILURE = 1,
    INVALID_ID = 2,
} f_status_t;

/*=============================================================================
* GLOBALS
=============================================================================*/

struct globals {
    LinkedList *accounts;
    LinkedList *delete_requests;
    atm_t *atms;
    int num_atms;
    account_with_id *bank_account; // the bank account
    rwlock_t account_lock;
    rwlock_t atm_lock;
    rwlock_t log_lock;
    rwlock_t delete_lock;
    rwlock_t finished_lock;
    FILE *log_file;
    pthread_t **atm_threads;
    pthread_t *bank_thread;
    int finished;
};

#define LOG_FILE_NAME "log.txt"

typedef struct globals globals_t;

extern globals_t *globals;

void global_init();
void global_free();
void log_lock();
void log_unlock();

#endif // UTILS_H