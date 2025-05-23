#ifndef LOCK_H
#define LOCK_H

#include <pthread.h>

typedef struct {
    pthread_mutex_t lock;
    pthread_cond_t readers_ok;
    pthread_cond_t writers_ok;
    int readers;         // current active readers
    int writers;         // current active writers
} rwlock_t;

void rwlock_init(rwlock_t *rw);
void rwlock_acquire_read(rwlock_t *rw);
void rwlock_release_read(rwlock_t *rw);
void rwlock_acquire_write(rwlock_t *rw);
void rwlock_release_write(rwlock_t *rw);
void rwlock_destroy(rwlock_t *rw);


#endif // LOCK_H