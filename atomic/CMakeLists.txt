check_c_source_compiles("
      int main() {
        volatile unsigned long val = 1;
        __sync_synchronize();
        __sync_val_compare_and_swap(&val, 1, 0);
        __sync_add_and_fetch(&val, 1);
        __sync_sub_and_fetch(&val, 1);
        return 0;
      }
" HAVE_GCC_ATOMIC)

check_c_source_compiles("
        int main() {
            sched_yield();
            return 0;
        }
" HAVE_SCHED_YIELD)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config.h.in ${CMAKE_CURRENT_SOURCE_DIR}/config.h)

add_library(atomic spinlock.c)