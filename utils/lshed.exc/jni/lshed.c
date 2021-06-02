#include <ctype.h>
#include <linux/sched.h>
#include <linux/unistd.h>
#include <sched.h> /* for std. scheduling system calls */
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h> /* high-res timers */
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>

char* policychecker(int policy) {
    switch (policy) {
    case 0:
        return "SCHED_NORMAL";
    case 1:
        return "SCHED_FIFO";
    case 2:
        return "SCHED_RR";
    case 3:
        return "SCHED_BATCH";
    case 5:
        return "SCHED_IDLE";
    case 6:
        return "SCHED_WRR";
    default:
        return "UNKNOWN";
    }
}

int main() {
    int policy, timeslice;
    pid_t pid;
    struct timespec wrr_time;
    printf("Input the process id (PID) you want to check: ");
    scanf("%d", &pid);
    if (sched_rr_get_interval(pid, &wrr_time)) {
        printf("Failed getting interval: %s\n", strerror(errno));
        return -1;
    }
    policy = sched_getscheduler(pid);
    printf("Schedule policy: %s\n", policychecker(policy));
    timeslice = wrr_time.tv_nsec / 1000000;
    printf("Timeslice: %d milisec\n", timeslice);
    return 0;
}
