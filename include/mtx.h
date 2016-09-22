#ifndef LIMITER_MTX_H
#define LIMITER_MTX_H

#include <stdint.h>
#include <semaphore.h>
#include "shm.h"
#include "atomic.h"

typedef struct {
    atomic_t *lock;
    atomic_t *wait;
    sem_t *sem;
    uintptr_t semaphore;
    uintptr_t spin;
} limiter_mtx_t;

typedef struct {
    atomic_t lock;
#if (HAVE_POSIX_SEM)
    atomic_t wait;
#endif
} limiter_sh_mtx_t;

void limiter_mtx_init(void);
int limiter_mtx_create(limiter_mtx_t *mtx, limiter_sh_mtx_t *sh_mtx);
void limiter_mtx_destroy(limiter_mtx_t *mtx);
void limiter_mtx_lock(limiter_mtx_t *mtx, pid_t pid);
uintptr_t limiter_mtx_trylock(limiter_mtx_t *mtx, pid_t pid);
void limiter_mtx_unlock(limiter_mtx_t *mtx, pid_t pid);

#endif //LIMITER_MTX_H
