cmake_minimum_required(VERSION 2.8)
project(parakeet)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O0 -Wall -std=gnu99")

set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)

include(CheckCSourceCompiles)
check_c_source_compiles("
      #include <unistd.h>
      int main() {
        sysconf(_SC_NPROCESSORS_ONLN);
        return 0;
      }
" HAVE_SC_NPROCESSORS_ONLN)

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
      #include <stddef.h>
      #include <sys/types.h>
      #include <sys/mman.h>
      int main() {
        void *p;
        p = mmap(NULL, 4096, PROT_READ|PROT_WRITE,
            MAP_ANON|MAP_SHARED, -1, 0);
        if (p == MAP_FAILED) return 1;
        return 0;
      }
" HAVE_MAP_ANON)

check_c_source_compiles("
        int main() {
            sched_yield();
            return 0;
        }
" HAVE_SCHED_YIELD)

check_c_source_compiles("
        #include <semaphore.h>
        int main() {
            sem_t  sem;
            if (sem_init(&sem, 1, 0) == -1) return 1;
            sem_destroy(&sem);
            return 0;
        }
" HAVE_POSIX_SEM)

set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")

configure_file(${PROJECT_SOURCE_DIR}/config.h.in ${PROJECT_SOURCE_DIR}/include/config.h)

add_subdirectory(src)
add_subdirectory(test)

