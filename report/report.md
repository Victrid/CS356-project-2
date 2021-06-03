# Report: Weighted Round-Robin scheduler

{{TOC}}

## Introduction

CPU scheduling is the basis of multiprogrammed operating systems. By switching the CPU among processes, the operating system can make the computer more productive[quote here: OSC, 8th, 183]. The Linux kernel, and the Android OS built on top of it, uses both First-in-First-out (**FIFO**) queuing and round robin (**RR**) strategies to build its real-time tasks scheduler, and the completely fair scheduling (**CFS**) algorithm to build its default task scheduler. We implement here a weighted round robin (**WRR**) scheduler and compare the typical features of different schedulers.

As in the original requirement, our implementation of the WRR scheduler sets the scheduling weights and time slices only according to whether it is a foreground or a background application. However, the distinction does not bring out the true power of the WRR scheduler, so we modified it by making a system call that allows the task weights to be modified, and demonstrated this in the scheduler comparison.

We also implemented several utilities to help us inspect and change scheduling policies. To provide an image of the comparison between schedulers, we have also written a process testing tool.

## Analysis and Implementation

### `wrr.c`, the scheduler

In order to design our scheduler, we first studied the scheduler mechanism of linux. We found that linux uses the `sched_class` structure to implement the scheduler in order to abstract different scheduling mechanism. Therefore, our scheduler implementation is mainly to implement the `sched_class` structure of the `WRR` scheduler, which is mainly in `kernel/sched/wrr.c`. To provide better structured information, we have also added a header file `kernel/sched/wrr.h` that is only referenced in `wrr.c`.

In the requirements, we do not need to implement SMP, i.e. the symmetric multiprocessor architecture, so we ignore all SMP-related contents when implementing. In addition, our WRR here is more oriented to scheduling common tasks than RT scheduler scheduling real-time tasks, and in the requirements we also set the weights through the task group mechanism, so instead of using the task group and group inheritance scheduling structure like the real-time tasks, we use an overall round robin mechanism, and each polling will be selected among all tasks that use the WRR scheduling mechanism.

Our WRR structure here requires the use of task groups to distinguish between the foreground and background. Therefore, we refer to the relevant function `task_group_path()` in the `debug.c` file to get the task group name and thus determine the foreground and background. In this implementation, the weight is updated every time it happens to enqueue or dequeue, and when the time slice is being refreshed.

The main implementation of the rest of the content refers to the original real-time scheduler section. However, we do not need the part of the real-time scheduler about priority, so instead of using a bitmap structure, we operate directly on a list. This (list) operation process is also reflected in the real-time scheduler, because in fact the real-time scheduler also contains the round robin scheduling algorithm.

### Embedding into the system

In order to add our WRR scheduler to the kernel, we need to make some adjustments to the original system content.

- Compile Flags

    We introduced a new compile flag `CONFIG_WRR_GROUP_SCHED`, which will automatically generated into a header file at compile time and define the `CONFIG_WRR_GROUP_SCHED` macro. Add this option to the `init/Kconfig` file and add `CONFIG_WRR_GROUP_SCHED=y` to the `arch/arm/configs/goldfish_armv7_defconfig` file so that this macro is defined.

    We have introduced a number of debugging statements at the time of coding and they are all defined under the `CONFIG_SCHED_DEBUG` macro. However, these printing operations cause a lot of extra overhead for the scheduler, so we created a new file `arch/arm/configs/goldfish_armv7_nodebug_defconfig` here to disable it and save the overhead. The `task_group_path()` mentioned above also needs to be adjusted at this point, as removing this macro will make a function from `autogroup.h` invalid.

- Linux header files

    We adjusted `include/linux/sched.h` to define the `SCHED_WRR` macro identifier and some values for weights and time slices. We declared `wrr_rq` as `rt_rq` do.

    we also defined `sched_wrr_entity` structure to store necessary information for each WRR task: a link_head `run_list` for WRR task queueing, `time_slice` and `weight` for weighting and time slice counting. After defining we added an instance `wrr` into `task_struct`.

- Scheduler Makefile

    Added `wrr.o` to link our `wrr.c` into the scheduler.

- Scheduler `rt.c`

    Set the next scheduler of the RT scheduler to be the WRR scheduler (originally it would be the CFS scheduler), so that the main scheduler will be scheduled in the order RT - WRR - CFS.

- Scheduler `core.c`

    Added the time slice update and weight update related operation functions for WRR. Since other schedulers do not set the weights in WRR, we need this function to set them when switching schedulers.

    We initialize lists of WRR, allowable values, etc., just like the RT do. `core.c` has a common judgment: if it is a real-time task, set it to `rt_sched_class`, otherwise set it to `fair_sched_class`. We use the `policy` value of the task to determine this and set the task with the WRR policy to `wrr_sched_class`.

    We refine the judgment of the value of `policy` in the related function of the task scheduler switching.
    
- Scheduler `sched.h`

    defined struct `wrr_rq`, which is the running queue of WRR. the list_head `active` points to the head of the queue of `sched_wrr_entity`, and `wrr_nr_running` indicates the running task number in the WRR queue.

### Testing utilities

In order to test the scheduler more easily, we wrote two utilities `lshed` and `chshed`. By using the system calls `sched_setscheduler` and `sched_rr_get_interval`, the former can output the used scheduler and time slice information based on the input PID, and the latter can change the scheduler and scheduling parameters for a specific PID.

Since the given `processtest.apk` is not very comprehensive in what it tests, we also wrote a `proctest` test program that forks a specified number of test processes and specifies their schedulers and scheduling parameters. The test process simulates a common CPU burst / IO burst loop, waiting for a certain number of seconds (simulates an IO waiting) after a large number of adding loop (which is a CPU burst), and print out the loop time.

## Test Results

## Additional Part

### Throttle Mechanism

Because the main scheduler is scheduled sequentially, i.e. the later scheduler will be called only when the previous one has finished scheduling. Therefore, when intensive tasks are executed with WRR policy, it will be difficult for the CFS scheduler immediately after it to get time for running. Since the vast majority of the system's tasks, including `adb`, display, and control, are executed under the CFS scheduler, this will make the entire virtual machine not responding.

Following the example of the throttle mechanism in the real-time scheduler, we have introduced this mechanism in the WRR scheduler to share more runtime with other schedulers (i.e. CFS scheduler).

### Adjustable Weight

There are many programs, especially those executing on the linux level (the other example is the android level), that do not use task groups to distinguish themselves. For example, the android debug bridge, and all programs forked from the adb shell, behave as so-called foreground programs. This degrades the WRR scheduler to a normal RR scheduler in most situations.

Therefore we modified the implementation functions behind the `sched_setscheduler` and `sched_setparam` system calls, and also modified `sched_wrr_entity` appropriately to enable flexible adjustment of the weights. Since the `sched_param` structure is defined in the GNU C library and not in the kernel, it is difficult to adjust the contents of this structure, so we have reused the priority in the configuration real-time task. Setting this value to a non-negative value manually adjusts the weight of the task, while setting it to 0 restores the original weighting convention.

This feature is critical to utilize the WRR scheduler’s full ability, and is applied in the comparison of scheduler features that follows.

### Scheduler Comparison

## Achievements

Through this project, I have gained a deeper understanding of the scheduler in the linux kernel. By combining kernel-level programming with application-level programming, I gradually understood their similarities and differences. It was especially gratifying to see that my additions to the kernel could be performed and called correctly by the outer programs. From understanding the implementation of a large project, to editing, compiling and debugging it, them all forms a great challenge which could not be found in past practical courses, and I think this project is very practical and instructive for future programming practice.