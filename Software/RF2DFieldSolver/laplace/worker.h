#ifndef INCLUDE_WORKER_H
#define INCLUDE_WORKER_H

#include <stdint.h>
#include <pthread.h>

typedef void (*progress_callback_t)(void *ptr, double current_diff);

#include "lattice.h"
#include "tuple.h"

struct worker {
    uint8_t id;
    uint8_t done;
    struct worker* previous;
    struct worker* next;
    struct lattice* lattice;
    struct point pos;
    uint32_t iterations;

    struct config conf;

    pthread_t thread;
    pthread_spinlock_t lock, listLock;
    pthread_mutex_t mutex;
    pthread_cond_t cond;

    progress_callback_t cb;
    void *cb_ptr;
};

struct worker*
worker_new(struct worker* next, struct lattice* lattice, struct config* conf, progress_callback_t cb, void *cb_ptr);

void worker_delete(struct worker* worker);
void* work(void* ptr);

#endif
