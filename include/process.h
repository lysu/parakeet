#ifndef LIMITER_PROCESS_H
#define LIMITER_PROCESS_H

#include <sched.h>
#include "config.h"

#if (HAVE_SCHED_YIELD)
#define l_sched_yield()  sched_yield()
#else
#define l_sched_yield()  usleep(1)
#endif

#endif //LIMITER_PROCESS_H
