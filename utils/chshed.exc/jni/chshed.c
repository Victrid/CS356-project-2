#include <ctype.h>
#include <errno.h>
#include <linux/sched.h>
#include <linux/unistd.h>
#include <sched.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#define SCHED_NORMAL 0
#define SCHED_FIFO 1
#define SCHED_RR 2
#define SCHED_BATCH 3
#define SCHED_IDLE 5
#define SCHED_WRR 6

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
    int prio, policy;
    struct sched_param param;
    pid_t pid;
    printf("Please input the choice of scheduling algorithms(0-NORMAL, "
           "1-FIFO, 2-RR, 6-WRR): ");
    scanf("%d", &policy);
    printf("Current scheduling algorithm is %s\n", policychecker(policy));
    printf("Please input the PID of the process: ");
    scanf("%d", &pid);
    if (policy != 6) {
        printf("Set process's priority (1-99): ");
        scanf("%d", &prio);
        param.sched_priority = prio;
    } else
        param.sched_priority = 0;

    if (sched_setscheduler(pid, policy, &param)) {
        printf("Changing failed: %s\n", strerror(errno));
        return -1;
    }
    printf("Done.\n");
    return 0;
}
