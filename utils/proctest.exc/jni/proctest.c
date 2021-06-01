#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <unistd.h>
#include <string.h>
#include <linux/unistd.h>
#include <linux/sched.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/time.h> /* high-res timers */
#include <sched.h> /* for std. scheduling system calls */

int main(int argc, char *argv[])
{
    int forks = 2;
    int pid;
    int loopind = 0;
    int t;
    struct sched_param param;
    param.sched_priority = 0;
    struct timespec spec1, spec2;
    if (argc == 2){
        forks = atoi(argv[1]);
    }
    for(t=0;t<forks;t++){
        pid = fork();
        if(pid != 0)
            continue;
        else
            goto loop;
    }
    wait(NULL);
	return 0;
loop:
    /* Set to WRR */
    clock_gettime(CLOCK_REALTIME, &spec1);
    sched_setscheduler(getpid(), 6, &param);
    for(loopind=0;;loopind++){
        if (loopind == 100000000){
            loopind = 0;
            if(t == 0){
            clock_gettime(CLOCK_REALTIME, &spec2);
            printf("PID=%d, %ldms\n",getpid(), (spec2.tv_sec-spec1.tv_sec)*1000 +(spec2.tv_nsec-spec1.tv_nsec) / 1000000);
            clock_gettime(CLOCK_REALTIME, &spec1);
        }
        }
        
    }
}
