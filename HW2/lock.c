#include "lock.h"

void rwlock_init(rwlock_t *rw) {
    pthread_mutex_init(&rw->lock, NULL);
    pthread_cond_init(&rw->readers_ok, NULL);
    pthread_cond_init(&rw->writers_ok, NULL);
    rw->readers = 0;
    rw->writers = 0;
    rw->waiting_writers = 0;
}


void rwlock_acquire_read(rwlock_t *rw) {
    pthread_mutex_lock(&rw->lock);
    while (rw->writers > 0 || rw->waiting_writers > 0) {
        pthread_cond_wait(&rw->readers_ok, &rw->lock);
    }
    rw->readers++;
    pthread_mutex_unlock(&rw->lock);
}


void rwlock_release_read(rwlock_t *rw) {
    pthread_mutex_lock(&rw->lock);
    rw->readers--;
    if (rw->readers == 0) {
        pthread_cond_signal(&rw->writers_ok);
    }
    pthread_mutex_unlock(&rw->lock);
}


void rwlock_acquire_write(rwlock_t *rw) {
    pthread_mutex_lock(&rw->lock);
    rw->waiting_writers++;
    while (rw->readers > 0 || rw->writers > 0) {
        pthread_cond_wait(&rw->writers_ok, &rw->lock);
    }
    rw->waiting_writers--;
    rw->writers = 1;
    pthread_mutex_unlock(&rw->lock);
}


void rwlock_release_write(rwlock_t *rw) {
    pthread_mutex_lock(&rw->lock);
    rw->writers = 0;
    if (rw->waiting_writers > 0) {
        pthread_cond_signal(&rw->writers_ok);
    } else {
        pthread_cond_broadcast(&rw->readers_ok);
    }
    pthread_mutex_unlock(&rw->lock);
}


void rwlock_destroy(rwlock_t *rw) {
    pthread_mutex_destroy(&rw->lock);
    pthread_cond_destroy(&rw->readers_ok);
    pthread_cond_destroy(&rw->writers_ok);
}