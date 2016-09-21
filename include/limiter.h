#ifndef __LIMITER_H_
#define __LIMITER_H_

#include <stdlib.h>
#include "shm.h"
#include "mtx.h"

typedef struct {
    volatile int lock;
    volatile long last_add_ts;
    volatile uintptr_t tokens;
    volatile uintptr_t active_count;
} bucket_t;

typedef struct {
    shm_zone_t *mtx_zone;
    limiter_mtx_share_t *mtx_share;
    limiter_mtx_t *mtx;

    shm_zone_t *bucket_zone;
    bucket_t *bucket;

    uintptr_t bucket_capcity;
    uintptr_t bucket_add_rate;
    long active_threshold;
} limiter_t;

int limiter_init(limiter_t *l, uintptr_t bucket_capcity, uintptr_t count_per_second, long active_threshold);

int limiter_allow(limiter_t *l, pid_t pid);

int limiter_destroy(limiter_t *l);

void limiter_start_action(limiter_t *l);

void limiter_end_action(limiter_t *l);

#endif
