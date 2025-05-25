#define _POSIX_C_SOURCE 200809L
#include "lock.h"

rwlock_t rwlock_init() {
    rwlock_t rw = (rwlock_t)malloc(sizeof(struct rwlock));
    rw->lock = (pthread_mutex_t *)malloc(sizeof(pthread_mutex_t));
    rw->readers_ok = (pthread_cond_t *)malloc(sizeof(pthread_cond_t));
    rw->writers_ok = (pthread_cond_t *)malloc(sizeof(pthread_cond_t));
    
    pthread_mutex_init(rw->lock, NULL);
    pthread_cond_init(rw->readers_ok, NULL);
    pthread_cond_init(rw->writers_ok, NULL);
    rw->readers = 0;
    rw->writers = 0;
    return rw;
}


void rwlock_acquire_read(rwlock_t rw) {
    pthread_mutex_lock(rw->lock);
    while (rw->writers > 0) {
        pthread_cond_wait(rw->readers_ok, rw->lock);
    }
    rw->readers++;
    pthread_mutex_unlock(rw->lock);
}


void rwlock_release_read(rwlock_t rw) {
    pthread_mutex_lock(rw->lock);
    rw->readers--;
    if (rw->readers == 0) {
        pthread_cond_signal(rw->writers_ok);
    }
    pthread_mutex_unlock(rw->lock);
}


void rwlock_acquire_write(rwlock_t rw) {
    pthread_mutex_lock(rw->lock);
    rw->writers++;
    while (rw->readers > 0 || rw->writers > 1) {
        pthread_cond_wait(rw->writers_ok, rw->lock);
    }
    pthread_mutex_unlock(rw->lock);
}


void rwlock_release_write(rwlock_t rw) {
    pthread_mutex_lock(rw->lock);
    rw->writers--;
    if (rw->writers > 0) {  // change to writers (regular) to be like recitation?
        pthread_cond_signal(rw->writers_ok);
    } else {
        pthread_cond_broadcast(rw->readers_ok);
    }
    pthread_mutex_unlock(rw->lock);
}


void rwlock_destroy(rwlock_t rw) {
    // including free for rwlock itself
    pthread_mutex_lock(rw->lock);
    rw->writers++;
    //check if lock not in use 
    while (rw->readers > 0 || rw->writers != 1) {
        pthread_cond_wait(rw->writers_ok, rw->lock);
        pthread_mutex_lock(rw->lock);  // OK to lock? only to make sure no one enters
    }    
    // should be good since not in accounts list and all others finished
    pthread_mutex_unlock(rw->lock);
    pthread_mutex_destroy(rw->lock); 
    pthread_cond_destroy(rw->readers_ok);
    pthread_cond_destroy(rw->writers_ok);
    free(rw->lock);
    free(rw->readers_ok);
    free(rw->writers_ok);
    free(rw);
}