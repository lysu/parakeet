#include <stdio.h>
#include <errno.h>
#include <semaphore.h>
#include <unistd.h>
#include <mtx.h>

#include "mtx.h"
#include "process.h"
#include "config.h"

long current_ncpu;

static void limiter_shmtx_wakeup(limiter_mtx_t *mtx);

void limiter_mtx_init(void) {
#if (HAVE_SC_NPROCESSORS_ONLN)
    if (current_ncpu == 0) {
        current_ncpu = sysconf(_SC_NPROCESSORS_ONLN);
    }
#endif
    if (current_ncpu < 1) {
        current_ncpu = 1;
    }
}

int limiter_mtx_create(limiter_mtx_t *mtx, limiter_sh_mtx_t *sh_mtx) {
    mtx->lock = &sh_mtx->lock;
    if (mtx->spin == (uintptr_t) -1) {
        return 0;
    }

    mtx->spin = 2048;

#if (HAVE_POSIX_SEM)
    mtx->wait = &sh_mtx->wait;
    if (sem_init(&(mtx->sem), 1, 0) == -1) {
        fprintf(stderr, "sem_init() failed\n");
    } else {
        mtx->semaphore = 1;
    }
#endif

    return 0;
}

void limiter_mtx_destroy(limiter_mtx_t *mtx) {
#if (HAVE_POSIX_SEM)
    if (mtx->semaphore) {
        if (sem_destroy(&(mtx->sem)) == -1) {
            fprintf(stderr, "sem_desctroy() failed\n");
        }
    }
#endif
}

void limiter_mtx_lock(limiter_mtx_t *mtx, pid_t pid0) {
    atomic_uint_t pid = (atomic_uint_t) pid0;
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

#if (HAVE_POSIX_SEM)
        if (mtx->semaphore) {
            (void) atomic_fetch_add(mtx->wait, 1);
            if (*mtx->lock == 0 && atomic_cmp_set(mtx->lock, 0, pid)) {
                (void) atomic_fetch_add(mtx->wait, -1);
                return;
            }
            while (sem_wait(&(mtx->sem)) == -1) {
                if (errno != EINTR) {
                    fprintf(stderr, "sem_wait() failed while waiting on shmtx");
                    break;
                }
            }
            continue;
        }
#endif
        l_sched_yield();
    }
}


uintptr_t limiter_mtx_trylock(limiter_mtx_t *mtx, pid_t pid) {
    return (uintptr_t) (*mtx->lock == 0 && atomic_cmp_set(mtx->lock, 0, pid));
}

void limiter_mtx_unlock(limiter_mtx_t *mtx, pid_t pid0) {
    atomic_uint_t pid = (atomic_uint_t) pid0;
    if (atomic_cmp_set(mtx->lock, pid, 0)) {
        limiter_shmtx_wakeup(mtx);
    } else {
        fprintf(stderr, "unlock failure, set current pid: %lu, want: %lu => 0, lock addr: %p\n", *mtx->lock, pid,
                mtx->lock);
    }
}

static void limiter_shmtx_wakeup(limiter_mtx_t *mtx) {
#if (HAVE_POSIX_SEM)
    atomic_t wait;

    if (!mtx->semaphore) {
        return;
    }

    for (;;) {
        wait = *mtx->wait;
        if ((atomic_int_t) wait <= 0) {
            return;
        }
        if (atomic_cmp_set(mtx->wait, wait, wait - 1)) {
            break;
        }
    }

    if (sem_post(&(mtx->sem)) == -1) {
        fprintf(stderr, "sem_post() failed while wake shmtx");
    }
#endif
}
