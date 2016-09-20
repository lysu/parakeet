#include <stdio.h>
#include <errno.h>
#include <semaphore.h>
#include <unistd.h>

#include "mtx.h"
#include "process.h"

int current_ncpu;

static void limiter_shmtx_wakeup(limiter_mtx_t *mtx);

int limiter_mtx_create(limiter_mtx_t *mtx, limiter_mtx_share_t *addr, char *name) {
    mtx->lock = &addr->lock;
    mtx->sem = &addr->sem;

    if (mtx->spin == (uint64_t) -1) {
        return 0;
    }

    mtx->spin = 2048;

    mtx->wait = &addr->wait;
    if (sem_init(mtx->sem, 1, 0) == -1) {
        fprintf(stderr, "sem_init() failed\n");
    } else {
        mtx->semaphore = 1;
    }

    current_ncpu = 3;

    return 0;
}

void limiter_mtx_destroy(limiter_mtx_t *mtx) {
    if (mtx->semaphore) {
        if (sem_destroy(mtx->sem) == -1) {
            fprintf(stderr, "sem_desctroy() failed\n");
        }
    }
}

void limiter_mtx_lock(limiter_mtx_t *mtx) {
    atomic_uint_t pid = (atomic_uint_t) getpid();
    uint64_t i, n;
    for (;;) {
        if (*mtx->lock == 0) {
            atomic_uint_t ret = atomic_cmp_set(mtx->lock, 0, pid);
            if (ret) {
                if (*mtx->lock != pid) {
                    fprintf(stderr, "cas success ret %lu but value not eq %lu != %lu %p\n", ret, *mtx->lock, pid,
                            mtx->lock);
                }
                return;
            }
        }

        if (current_ncpu > 1) {
            for (n = 1; n < mtx->spin; n <<= 1) {
                for (i = 0; i < n; i++) {
                    cpu_pause();
                }
                if (*mtx->lock == 0
                    && atomic_cmp_set(mtx->lock, 0, pid)) {
                    return;
                }
            }
        }

        if (mtx->semaphore) {
            (void) atomic_fetch_add(mtx->wait, 1);
            if (*mtx->lock == 0 && atomic_cmp_set(mtx->lock, 0, pid)) {
                (void) atomic_fetch_add(mtx->wait, -1);
                return;
            }
//            fprintf(stderr, "shmtx wait %u %d\n", *mtx->wait, getpid());
            while (sem_wait(mtx->sem) == -1) {
                if (errno != EINTR) {
                    fprintf(stderr, "sem_wait() failed while waiting on shmtx");
                    break;
                }
            }
//            fprintf(stderr, "shmtx awoke\n");
            continue;
        }

        l_sched_yield();
    }
}

void limiter_mtx_unlock(limiter_mtx_t *mtx) {
    atomic_uint_t pid = (atomic_uint_t) getpid();
    if (atomic_cmp_set(mtx->lock, pid, 0)) {
//        fprintf(stderr, "unlock success, set current pid: %d, want: %d => 0, lock addr: %p\n", *mtx->lock, pid, mtx->lock);
        limiter_shmtx_wakeup(mtx);
    } else {
        fprintf(stderr, "unlock failure, set current pid: %lu, want: %lu => 0, lock addr: %p\n", *mtx->lock, pid,
                mtx->lock);
    }
}

static void limiter_shmtx_wakeup(limiter_mtx_t *mtx) {
    uint64_t wait;

    if (!mtx->semaphore) {
        return;
    }

    for (;;) {
        wait = *mtx->wait;
        if ((uint64_t) wait <= 0) {
            return;
        }
        if (atomic_cmp_set(mtx->wait, wait, wait - 1)) {
            break;
        }
    }

    if (sem_post(mtx->sem) == -1) {
        fprintf(stderr, "sem_post() failed while wake shmtx");
    }
//    fprintf(stderr, "shmtx waked %u %d\n", wait, getpid());
}
