
/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */


#if (SMP)
#define SMP_LOCK  "lock;"
#else
#define SMP_LOCK
#endif


/*
 * "cmpxchgq  r, [m]":
 *
 *     if (rax == [m]) {
 *         zf = 1;
 *         [m] = r;
 *     } else {
 *         zf = 0;
 *         rax = [m];
 *     }
 *
 *
 * The "r" is any register, %rax (%r0) - %r16.
 * The "=a" and "a" are the %rax register.
 * Although we can return result in any register, we use "a" because it is
 * used in cmpxchgq anyway.  The result is actually in %al but not in $rax,
 * however as the code is inlined gcc can test %al as well as %rax.
 *
 * The "cc" means that flags were changed.
 */

static atomic_uint_t
atomic_cmp_set(atomic_t *lock, atomic_uint_t old,
               atomic_uint_t set) {
    u_char res;

    __asm__ volatile (

    SMP_LOCK
    "    cmpxchgq  %3, %1;   "
            "    sete      %0;       "

    : "=a" (res) : "m" (*lock), "a" (old), "r" (set) : "cc", "memory");

    return res;
}


/*
 * "xaddq  r, [m]":
 *
 *     temp = [m];
 *     [m] += r;
 *     r = temp;
 *
 *
 * The "+r" is any register, %rax (%r0) - %r16.
 * The "cc" means that flags were changed.
 */

static inline atomic_int_t
atomic_fetch_add(atomic_t *value, atomic_int_t add) {
    __asm__ volatile (

    SMP_LOCK
    "    xaddq  %0, %1;   "

    : "+r" (add) : "m" (*value) : "cc", "memory");

    return add;
}


#define memory_barrier()    __asm__ volatile ("" ::: "memory")

#define cpu_pause()         __asm__ ("pause")
