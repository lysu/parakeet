#include <stddef.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <stdio.h>

#include "shm.h"

int shm_alloc(shm_zone_t *shm) {
    shm->addr = (unsigned char *) mmap(NULL, shm->size, PROT_READ | PROT_WRITE,
                                       MAP_ANON | MAP_SHARED, -1, 0);
    if (shm->addr == MAP_FAILED) {
        fprintf(stderr, "mmap(%zu, ANON) failured\n", shm->size);
        return -1;
    }
    return 0;
}

void shm_free(shm_zone_t *shm) {
    if (munmap((void *) shm->addr, shm->size) == -1) {
        fprintf(stderr, "mummap(%p, %zu) failured\n", shm->addr, shm->size);
    }
}
