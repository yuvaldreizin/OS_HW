#ifndef UTILS_H
#define UTILS_H

#include <glib.h>

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
    GList *atms;
    int num_accounts;
    int num_atms;
};

typedef struct globals globals_t;
extern globals_t *globals;

void global_init(){
    globals = MALLOC_VALIDATED(globals_t, 1);
    globals->accounts = NULL;
    globals->atms = NULL;
    globals->num_accounts = 0;
    globals->num_atms = 0;
}

void global_free(){
    if (globals == NULL) return;
    // TODO - Free all accounts
    g_list_free_full(globals->accounts, free);
    // TODO - Free all ATMs
    g_list_free_full(globals->atms, free);
    free(globals);
    globals = NULL;
}


#endif // UTILS_H