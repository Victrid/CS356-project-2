 # Report: Weighted Round-Robin scheduler

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

