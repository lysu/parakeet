#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>

#include "limiter.h"

pid_t child(limiter_t *l) {
    pid_t pid = fork();
    if (pid <= 0) {
        return pid;
    }

    int ret;
    for (;;) {
        ret = limiter_allow(l, pid);
        if (ret > 0) {
            printf("%d-----%d\n", ret, pid);
        } else {
        }
    }
}

int main(void) {

    limiter_mtx_init();

    limiter_t *l = calloc(1, sizeof(limiter_t));
    if (limiter_init(l, 10, 1, 10) == -1) {
        fprintf(stderr, "init limiter failure\n");
    }
    fprintf(stderr, "init ok\n");

    int x;
    for (x = 0; x < 2; x++) {
        pid_t pid = child(l);
        if (pid == 0) {
            continue;
        }
        return 0;
    }

    fprintf(stderr, "waiting...\n");
    wait(NULL);

}




