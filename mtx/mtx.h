#ifndef LIMITER_MTX_H
#define LIMITER_MTX_H

#include <stdint.h>
#include <semaphore.h>
#include "shm.h"
#include "atomic.h"

typedef struct {
    atomic_t *lock;
    atomic_t *wait;
    uintptr_t semaphore;
    sem_t *sem;
    uintptr_t spin;
} limiter_mtx_t;

typedef struct {
    atomic_t lock;
    atomic_t wait;
    sem_t sem;
} limiter_mtx_share_t;

int limiter_mtx_create(limiter_mtx_t *mtx, limiter_mtx_share_t *addr, char *name);

void limiter_mtx_destroy(limiter_mtx_t *mtx);

void limiter_mtx_lock(limiter_mtx_t *mtx);

void limiter_mtx_unlock(limiter_mtx_t *mtx);

#endif //LIMITER_MTX_H