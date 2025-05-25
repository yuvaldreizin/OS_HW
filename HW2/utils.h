#ifndef UTILS_H
#define UTILS_H

#include <unistd.h>
#include <stdio.h>
#include "linked_list.h"
#include "lock.h"


typedef struct account{
    int id;
    int pass;
    int balance;
    rwlock_t lock;
} account;

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


extern account* account_init(int id, int pass, int balance);
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
    account *bank_account; // the bank account
    rwlock_t account_lock;
    rwlock_t atm_lock;
    rwlock_t log_lock;
    rwlock_t delete_lock;
    FILE *log_file;
    pthread_t **atm_threads;
    pthread_t *bank_thread;
};

#define LOG_FILE_NAME "log.txt"

typedef struct globals globals_t;

extern globals_t *globals;

void global_init();
void global_free();
void log_lock();
void log_unlock();

#endif // UTILS_H