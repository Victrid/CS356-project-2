#include <ctype.h>
#include <errno.h>
#include <linux/sched.h>
#include <linux/unistd.h>
#include <sched.h> /* sched_param */
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

int main(int argc, char* argv[]) {
    int forks = 2;
    int pid;
    int loopind = 0;
    int t, vt;
    int r;
    int stepgn = 3;
    char c[20];
    vt = 6;
    struct sched_param param;
    param.sched_priority = 99;
    // Set child count
    if (argc >= 2)
        forks = atoi(argv[1]);

    // Set child scheduler
    if (argc >= 3)
        vt = atoi(argv[2]);

    if(argc >= 4)
        stepgn = atoi(argv[3]);

    // Set this daemon to the highest priority,
    // or it will be stuck
    if (sched_setscheduler(getpid(), 1, &param)) {
        printf("d:%s\n", strerror(errno));
    }
    struct timespec spec1, spec2;
    for (t = 0; t < forks; t++) {
        pid = fork();
        if (pid != 0)
            continue;
        if (vt == 1 || vt == 2) {
            param.sched_priority = stepgn * t + 5;
        } else if (vt == 6) {
            param.sched_priority = stepgn * (t + 5);
        } else {
            param.sched_priority = 0;
        }
        if (sched_setscheduler(getpid(), vt, &param)) {
            printf("s%d:%s\n", t, strerror(errno));
        }
        goto loop;
    }
    wait(NULL);
    return 0;
loop:
    /* Set to WRR */
    sleep(1); // Pull out of runqueue
    clock_gettime(CLOCK_REALTIME, &spec1);

    for (loopind = 0;; loopind++) {
        if (loopind == 40000000) {
            loopind = 0;
            clock_gettime(CLOCK_REALTIME, &spec2);
            printf("PID=%d ", getpid());
            for (r = 0; r < t; r++)
                printf("\t");
            printf("%ldms\n", (spec2.tv_sec - spec1.tv_sec) * 1000 +
                                  (spec2.tv_nsec - spec1.tv_nsec) / 1000000);
            sleep(1); // Pull out of runqueue
            clock_gettime(CLOCK_REALTIME, &spec1);
        }
    }
}
