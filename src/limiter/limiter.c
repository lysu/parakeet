#include <string.h>
#include <sys/time.h>
#include <limiter.h>
#include "limiter.h"

#define scale 1000
#define cache_line 128

static long current_ms();

static atomic_t active_count0 = 0;
static atomic_t last_add_ts0 = 0;

int limiter_init(limiter_t *l, uintptr_t bucket_capcity, uintptr_t count_per_second, long active_threshold) {
    l->bucket_capcity = bucket_capcity * scale;
    l->bucket_add_rate = count_per_second * scale;
    l->active_threshold = active_threshold;

    l->bucket = calloc(1, sizeof(bucket_t));
    if (l->bucket == NULL) {
        return -1;
    }
    l->mtx = calloc(1, sizeof(limiter_mtx_t));
    if (l->mtx == NULL) {
        return -1;
    }

    size_t shared_size = cache_line + // sh_mtx
                         cache_line + // bucket->last_add_ts
                         cache_line + // bucket->tokens
                         cache_line;  // bucket->active_count
    l->s_zone = calloc(1, sizeof(shm_zone_t));
    l->s_zone->size = shared_size;
    if (shm_alloc(l->s_zone) == -1) {
        return -1;
    }
    unsigned char *shared = l->s_zone->addr;
    memset(shared, 0, l->s_zone->size);
    limiter_sh_mtx_t *sh_mtx = (limiter_sh_mtx_t *) shared;
    l->bucket->last_add_ts = (atomic_t *) (shared + 1 * cache_line);
    l->bucket->tokens = (atomic_t *) (shared + 2 * cache_line);
    l->bucket->active_count = (atomic_t *) (shared + 3 * cache_line);

    *(l->bucket->last_add_ts) = (atomic_t) current_ms();
    *(l->bucket->tokens) = l->bucket_capcity;
    *(l->bucket->active_count) = 0;

    if (limiter_mtx_create(l->mtx, sh_mtx) == -1) {
        return -1;
    }

    return 0;
}

int limiter_allow(limiter_t *l, pid_t pid) {
    if (l->active_threshold > 0 && *(l->bucket->active_count) > l->active_threshold) {
        return 0;
    }
    limiter_mtx_lock(l->mtx, pid);
    long now = current_ms();
    long calTokens = *(l->bucket->tokens) + ((now - *(l->bucket->last_add_ts)) * l->bucket_add_rate / 1000);
    *(l->bucket->tokens) = calTokens > l->bucket_capcity ? l->bucket_capcity : (uintptr_t) calTokens;
    *(l->bucket->last_add_ts) = (atomic_t) now;
    if (*(l->bucket->tokens) < 1 * scale) {
        limiter_mtx_unlock(l->mtx, pid);
        return 0;
    }

    long new_tokens = *(l->bucket->tokens) -= 1 * scale;
    if (new_tokens < 0) {
        new_tokens = 0;
    }
    *(l->bucket->tokens) = (uint64_t) new_tokens;
    limiter_mtx_unlock(l->mtx, pid);
    return 1;
}

int limiter_destroy(limiter_t *l) {
    shm_free(l->s_zone);
    free(l->bucket);
    free(l->mtx);
    return 1;
}

void limiter_start_action(limiter_t *l) {
    if (l->active_threshold > 0) {
        (void) atomic_fetch_add(l->bucket->active_count, 1);
    }
}

void limiter_end_action(limiter_t *l) {
    if (l->active_threshold > 0) {
        (void) atomic_fetch_add(l->bucket->active_count, -1);
    }
}

static long current_ms() {
    struct timeval start_ts;
    gettimeofday(&start_ts, 0);
    return ((long) start_ts.tv_sec) * 1000 + start_ts.tv_usec / 1000;
}
