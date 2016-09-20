#ifndef __SHM_H_
#define __SHM_H_

typedef struct _shm_zone {
    unsigned char *addr;
    size_t size;
} shm_zone_t;

int shm_alloc(shm_zone_t *shm);

void shm_free(shm_zone_t *shm);

#endif