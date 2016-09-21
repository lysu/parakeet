#include <string.h>
#include <sys/time.h>
#include <limiter.h>
#include "limiter.h"

#define scale 1000

static long current_ms();

int limiter_init(limiter_t *l, uintptr_t bucket_capcity, uintptr_t count_per_second, long active_threshold) {
    l->bucket_capcity = bucket_capcity * scale;
    l->bucket_add_rate = count_per_second * scale;

    l->bucket_zone = calloc(1, sizeof(shm_zone_t));
    l->bucket_zone->size = sizeof(bucket_t);
    if (shm_alloc(l->bucket_zone) == -1) {
        return -1;
    }
    l->bucket = (bucket_t *) l->bucket_zone->addr;
    memset(l->bucket, 0, l->bucket_zone->size);

    l->bucket->last_add_ts = current_ms();
    l->bucket->tokens = l->bucket_capcity;

    l->bucket->active_count = 0;
    l->active_threshold = active_threshold;

    l->mtx_zone = calloc(1, sizeof(shm_zone_t));
    l->mtx_zone->size = sizeof(limiter_mtx_share_t);
    if (shm_alloc(l->mtx_zone) == -1) {
        return -1;
    }
    l->mtx_share = (limiter_mtx_share_t *) l->mtx_zone->addr;
    memset(l->mtx_share, 0, l->mtx_zone->size);

    l->mtx = calloc(1, sizeof(limiter_mtx_t));
    if (limiter_mtx_create(l->mtx, l->mtx_share) == -1) {
        return -1;
    }

    return 0;
}

int limiter_allow(limiter_t *l, pid_t pid) {
    if (l->active_threshold > 0 && l->bucket->active_count > l->active_threshold) {
        return 0;
    }
    limiter_mtx_lock(l->mtx, pid);
    long now = current_ms();
    long calTokens = l->bucket->tokens + ((now - l->bucket->last_add_ts) * l->bucket_add_rate / 1000);
    l->bucket->tokens = calTokens > l->bucket_capcity ? l->bucket_capcity : (uintptr_t) calTokens;
    l->bucket->last_add_ts = now;
    if (l->bucket->tokens < 1 * scale) {
        limiter_mtx_unlock(l->mtx, pid);
        return 0;
    }

    long new_tokens = l->bucket->tokens -= 1 * scale;
    if (new_tokens < 0) {
        new_tokens = 0;
    }
    l->bucket->tokens = (uint64_t) new_tokens;
    limiter_mtx_unlock(l->mtx, pid);
    return 1;
}

int limiter_destroy(limiter_t *l) {
    shm_free(l->bucket_zone);
    shm_free(l->mtx_zone);
    free(l->bucket_zone);
    free(l->mtx_zone);
    free(l->mtx);
    return 1;
}

void limiter_start_action(limiter_t *l) {
    if (l->active_threshold > 0) {
        (void) atomic_fetch_add(&l->bucket->active_count, 1);
    }
}

void limiter_end_action(limiter_t *l) {
    if (l->active_threshold > 0) {
        (void) atomic_fetch_add(&l->bucket->active_count, -1);
    }
}

static long current_ms() {
    struct timeval start_ts;
    gettimeofday(&start_ts, 0);
    return ((long) start_ts.tv_sec) * 1000 + start_ts.tv_usec / 1000;
}
