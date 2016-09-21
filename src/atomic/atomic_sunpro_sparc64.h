
/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */


#if (PTR_SIZE == 4)
#define CASA  casa
#else
#define CASA  casxa
#endif


atomic_uint_t
casa(atomic_uint_t set, atomic_uint_t old, atomic_t *lock);

atomic_uint_t
casxa(atomic_uint_t set, atomic_uint_t old, atomic_t *lock);

/* the code in src/os/unix/ngx_sunpro_sparc64.il */


static atomic_uint_t
atomic_cmp_set(atomic_t *lock, atomic_uint_t old,
               atomic_uint_t set) {
    set = CASA(set, old, lock);

    return (set == old);
}


static atomic_int_t
atomic_fetch_add(atomic_t *value, atomic_int_t add) {
    atomic_uint_t old, res;

    old = *value;

    for (;;) {

        res = old + add;

        res = CASA(res, old, value);

        if (res == old) {
            return res;
        }

        old = res;
    }
}


#define memory_barrier()                                                  \
        __asm (".volatile");                                                  \
        __asm ("membar #LoadLoad | #LoadStore | #StoreStore | #StoreLoad");   \
        __asm (".nonvolatile")

#define cpu_pause()
