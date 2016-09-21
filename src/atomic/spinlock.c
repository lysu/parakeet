#include "atomic.h"
#include "process.h"

void spinlocK(atomic_t *lock, atomic_t value, unsigned int spin) {
    unsigned int i, n;
    for (;;) {
        if (*lock == 0 && atomic_cmp_set(lock, 0, value)) {
            return;
        }
        if (3 > 1) {
            for (n = 1; n < spin; n <<= 1) {
                for (i = 0; i < n; i++) {
                    cpu_pause();
                }
                if (*lock == 0 && atomic_cmp_set(lock, 0, value)) {
                    return;
                }
            }
        }
        l_sched_yield();
    }
}
