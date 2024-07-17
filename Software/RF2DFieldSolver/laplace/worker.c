#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "worker.h"

double iterate(struct worker* worker);

struct worker*
worker_new(struct worker* next, struct lattice* lattice, struct config* conf) {
    struct worker* worker;
    int status;

    /* make sure at least one of them is provided */
    if(next == NULL && conf == NULL)
        goto ERROR1;

    /* allocate memory for the worker */
    worker = malloc(sizeof(struct worker));
    if(worker == NULL) goto ERROR1;

    /* initialise the spinlocks */
    status = pthread_spin_init(&worker->lock, PTHREAD_PROCESS_SHARED);
    if(status != 0) goto ERROR1;

    status = pthread_spin_init(&worker->listLock, PTHREAD_PROCESS_SHARED);
    if(status != 0) goto ERROR2;

    /* initialise the mutex */
    status = pthread_mutex_init(&worker->mutex, NULL);
    if(status != 0) goto ERROR3;

    /* initialise the wait condition */
    status = pthread_cond_init(&worker->cond, NULL);
    if(status != 0) goto ERROR4;

    /* update the linked list */
    if(next != NULL) {
        worker->id = next->id+1;

        /* TODO we might need to lock here */
        int needsPreviousLock = next->previous != next;
        if(needsPreviousLock) {
            pthread_spin_lock(&next->previous->listLock);
        }
        pthread_spin_lock(&next->listLock);
        worker->previous = next->previous;
        worker->next     = next;

        /* copy the configuration from the next worker */
        worker->conf.distance  = next->conf.distance;
        worker->conf.threads   = next->conf.threads;
        worker->conf.threshold = next->conf.threshold;

        /* insert ourself into the linked list */
        next->previous->next = worker;
        next->previous       = worker;
        if(needsPreviousLock) {
            pthread_spin_unlock(&worker->previous->listLock);
        }
        pthread_spin_unlock(&next->listLock);
    } else {
        worker->id = 1;

        /* this is the first worker */
        worker->previous = worker;
        worker->next = worker;

        /*
         * TODO make sure the maximum number of thread
         *      is smaller than the resolution of the
         *      lattice
         */

        /* copy the configuration */
        worker->conf.distance  = conf->distance;
        worker->conf.threads   = conf->threads;
        worker->conf.threshold = conf->threshold;
    }

    /* initialize fields */
    worker->done = 0;
    worker->lattice = lattice;
    worker->iterations = 0;
    worker->pos.x = 0;
    worker->pos.y = 0;

    /* start the thread */
    pthread_create(&worker->thread, NULL, &work, (void*) worker);

    return worker;

//ERROR4:
//    pthread_cond_destroy(&worker->cond);
ERROR4:
    pthread_mutex_destroy(&worker->mutex);
ERROR3:
    pthread_spin_destroy(&worker->listLock);
ERROR2:
    pthread_spin_destroy(&worker->lock);
ERROR1:
    if(worker != NULL) free(worker);

    return NULL;
}

void worker_delete(struct worker* worker) {
    pthread_cond_destroy(&worker->cond);
    pthread_mutex_destroy(&worker->mutex);
    pthread_spin_destroy(&worker->lock);
    free(worker);
}

void* work(void* ptr) {
    struct worker* worker = (struct worker*) ptr;
    double diff;

    printf("Thread %2d: starting\n", worker->id);

    do {
        worker->iterations++;
        diff = iterate(worker);

        /* safely reset the y position */
        pthread_spin_lock(&worker->lock);
        worker->pos.y = 0;
        pthread_spin_unlock(&worker->lock);
    } while(diff > worker->conf.threshold);

    /* safely reset the y position */
    pthread_spin_lock(&worker->lock);
    worker->done = 1;
    pthread_spin_unlock(&worker->lock);

    /* wake up the previous worker if it was waiting for us */
    if(pthread_mutex_trylock(&worker->previous->mutex) == 0) {
        pthread_cond_signal(&worker->previous->cond);
        pthread_mutex_unlock(&worker->previous->mutex);
    }

    printf("Thread %2d: finished with %d iterations\n",
        worker->id, worker->iterations);

    /* wait for all the threads */
    if(worker->previous->id != 1) {
        pthread_join(worker->previous->thread, NULL);
    }

    printf("Thread %2d: exiting\n", worker->id);

    return NULL;
}

double iterate(struct worker* worker) {
    struct lattice* lattice = worker->lattice;
    uint32_t w = lattice->dim.x;
    uint32_t h = lattice->dim.y;
    uint32_t increment = 1;
    double diff = 0;

    do {
        worker->pos.x = 0;
        do {
            uint32_t index = worker->pos.x+worker->pos.y*w;
            struct cell* cell = &lattice->cells[index];
            double value, check;

            /* skip the cell if possible */
            if(lattice->update[index] == NULL) {
                worker->pos.x++;
                continue;
            }

            /* compute the new value*/
            value = (*lattice->update[index])(lattice, cell);
            check = fabs(value-cell->value);
            if(check > diff) diff = check;

            /* update the cell */
            cell->value = value;
            worker->pos.x++;
        } while(worker->pos.x < w);

        /* make sure we can safely increment */
        pthread_spin_lock(&worker->listLock);
        pthread_spin_lock(&worker->next->lock);

        /* compute the distance to the next worker */
        uint32_t next;
        if(worker->pos.y <= worker->next->pos.y)
            next = worker->next->pos.y-worker->pos.y;
        else
            next = worker->next->pos.y-worker->pos.y+h;
        pthread_spin_unlock(&worker->next->lock);
        pthread_spin_unlock(&worker->listLock);

        /* increment is possible */
        if(next >= 2 || worker->next->done)
            increment = 1;
        else
            increment = 0;

        /* if it's too early to change, then we have to wait */
        if(!increment && worker->next != worker) {
            pthread_mutex_lock(&worker->mutex);
            pthread_cond_wait(&worker->cond, &worker->mutex);
            pthread_mutex_unlock(&worker->mutex);
        }

        /* safely increment the y position */
        pthread_spin_lock(&worker->lock);
        worker->pos.y++;
        pthread_spin_unlock(&worker->lock);
//        printf("Thread %2d: y: %u\n", worker->id, worker->pos.y);

        /* wake up the previous worker if it was waiting for us */
        if(pthread_mutex_trylock(&worker->previous->mutex) == 0) {
            pthread_cond_signal(&worker->previous->cond);
            pthread_mutex_unlock(&worker->previous->mutex);
        }

        /* create a new worker if possible */
        if(worker->pos.y == worker->conf.distance
        && worker->id < worker->conf.threads
        && worker->previous->id-1 != worker->id)
           worker_new(worker, worker->lattice, NULL);

        /* check if this iteration is finished */
        if(worker->pos.y == h)
            break;
    } while(1);

    return diff;
}
