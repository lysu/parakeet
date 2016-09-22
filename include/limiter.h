#ifndef __LIMITER_H_
#define __LIMITER_H_

#include <stdlib.h>
#include "shm.h"
#include "mtx.h"

typedef struct {
    atomic_t *last_add_ts;
    atomic_t *tokens;
    atomic_t *active_count;
} bucket_t;

typedef struct {
    shm_zone_t *s_zone;

    limiter_mtx_t *mtx;
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
