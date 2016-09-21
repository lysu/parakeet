
/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */


/*
 * "casa   [r1] 0x80, r2, r0"  and
 * "casxa  [r1] 0x80, r2, r0"  do the following:
 *
 *     if ([r1] == r2) {
 *         swap(r0, [r1]);
 *     } else {
 *         r0 = [r1];
 *     }
 *
 * so "r0 == r2" means that the operation was successful.
 *
 *
 * The "r" means the general register.
 * The "+r" means the general register used for both input and output.
 */


#if (PTR_SIZE == 4)
#define CASA  "casa"
#else
#define CASA  "casxa"
#endif


static atomic_uint_t
atomic_cmp_set(atomic_t *lock, atomic_uint_t old,
               atomic_uint_t set) {
    __asm__ volatile (

    CASA " [%1] 0x80, %2, %0"

    : "+r" (set) : "r" (lock), "r" (old) : "memory");

    return (set == old);
}


static atomic_int_t
atomic_fetch_add(atomic_t *value, atomic_int_t add) {
    atomic_uint_t old, res;

    old = *value;

    for (;;) {

        res = old + add;

        __asm__ volatile (

        CASA " [%1] 0x80, %2, %0"

        : "+r" (res) : "r" (value), "r" (old) : "memory");

        if (res == old) {
            return res;
        }

        old = res;
    }
}


#if (SMP)
#define memory_barrier()                                                  \
            __asm__ volatile (                                                \
            "membar #LoadLoad | #LoadStore | #StoreStore | #StoreLoad"        \
            ::: "memory")
#else
#define memory_barrier()   __asm__ volatile ("" ::: "memory")
#endif

#define cpu_pause()
