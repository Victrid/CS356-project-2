/*
 * WRR Scheduling Class
 */

#ifndef _wrr_included_
#define _wrr_included_

#include "sched.h"
#include <linux/slab.h>

#endif

#include "wrr.h"

/* 1. sched class definition */

const struct sched_class wrr_sched_class = {
    .next               = &fair_sched_class,      /*Required*/
    .enqueue_task       = enqueue_task_wrr,       /*Required*/
    .dequeue_task       = dequeue_task_wrr,       /*Required*/
    .yield_task         = yield_task_wrr,         /*Required*/
    .check_preempt_curr = check_preempt_curr_wrr, /*Required*/
    .pick_next_task     = pick_next_task_wrr,     /*Required*/
    .put_prev_task      = put_prev_task_wrr,      /*Required*/

#ifdef CONFIG_SMP
    .select_task_rq   = select_task_rq_wrr,   /*Never need impl */
    .set_cpus_allowed = set_cpus_allowed_wrr, /*Never need impl */
    .rq_online        = rq_online_wrr,        /*Never need impl */
    .rq_offline       = rq_offline_wrr,       /*Never need impl */
    .pre_schedule     = pre_schedule_wrr,     /*Never need impl */
    .post_schedule    = post_schedule_wrr,    /*Never need impl */
    .task_woken       = task_woken_wrr,       /*Never need impl */
    .switched_from    = switched_from_wrr,    /*Never need impl */
#endif

    .set_curr_task   = set_curr_task_wrr, /*Required*/
    .task_tick       = task_tick_wrr,     /*Required*/
    .get_rr_interval = get_rr_interval_wrr,
    .prio_changed    = prio_changed_wrr, /*Never need impl */
    .switched_to     = switched_to_wrr,  /*Never need impl */
};

/* 2. Functional tools */

static inline struct rq* rq_of_wrr_rq(struct wrr_rq* wrr_rq) {
    return wrr_rq->rq;
}

static inline struct wrr_rq* wrr_rq_of_se(struct sched_wrr_entity* wrr_se) {
    return wrr_se->wrr_rq;
}

static inline int on_wrr_rq(struct sched_wrr_entity* wrr_se) {
    // Check if wrr_se's runlist is empty
    return !list_empty(&wrr_se->run_list);
}

static char group_path[4096];

static char* task_group_path(struct task_group* tg) {
    /* From debug.c */
    if (autogroup_path(tg, group_path, 4096))
        return group_path;

    /*
     * May be NULL if the underlying cgroup isn't fully-created yet
     */
    if (!tg->css.cgroup) {
        group_path[0] = '\0';
        return group_path;
    }
    cgroup_path(tg->css.cgroup, group_path, 4096);
    return group_path;
}

static inline struct task_struct*
wrr_task_of(struct sched_wrr_entity* wrr_se) {
#ifdef CONFIG_SCHED_DEBUG
    WARN_ON_ONCE(!wrr_entity_is_task(wrr_se));
#endif
    return container_of(wrr_se, struct task_struct, wrr);
}

/* ! We also have several macros in the wrr.h */

/* 3. Essential functions in sched_class */

/*
 * adding and removing task.
 */
static void enqueue_task_wrr(struct rq* rq, struct task_struct* p,
                             int flags) {
    struct sched_wrr_entity* wrr_se = &p->wrr;

    /* the enqueue is a wakeup of a sleeping task. ? */
    // if (flags & ENQUEUE_WAKEUP)
    // 	wrr_se->timeout = 0;

    enqueue_wrr_entity(wrr_se, flags & ENQUEUE_HEAD);

    /* Pushable task? */
    // if (!task_current(rq, p) && p->wrr.nr_cpus_allowed > 1)
    // 	enqueue_pushable_task(rq, p);

    inc_nr_running(rq);
}

static void dequeue_task_wrr(struct rq* rq, struct task_struct* p,
                             int flags) {
    struct sched_wrr_entity* wrr_se = &p->wrr;

    /* Update the current task's runtime statistics. */
    update_curr_wrr(rq);

    dequeue_wrr_entity(wrr_se);

    /* Pushable task? */
    // dequeue_pushable_task(rq, p);

    dec_nr_running(rq);
}

static void yield_task_wrr(struct rq* rq) {
    requeue_task_wrr(rq, rq->curr, 0);
}

/*
 * Preempt the current task with a newly woken task if needed:
 */
static void check_preempt_curr_wrr(struct rq* rq, struct task_struct* p,
                                   int flags) {
    /* That's not how WRR works, so we just dummy this */
}

static struct task_struct* pick_next_task_wrr(struct rq* rq) {
    /* We skip the SMP and pushable task here*/
    struct sched_wrr_entity* wrr_se;
    struct task_struct* p;
    struct wrr_rq* wrr_rq;

    wrr_rq = &rq->wrr;

    if (!wrr_rq->wrr_nr_running)
        return NULL;

    /* No throttling */
    // if (wrr_rq_throttled(wrr_rq))
    // 	return NULL;

    wrr_se = pick_next_wrr_entity(rq, wrr_rq);
    BUG_ON(!wrr_se);

    p                = wrr_task_of(wrr_se);
    p->se.exec_start = rq->clock_task;

    return p;
}

static void put_prev_task_wrr(struct rq* rq, struct task_struct* p) {
    update_curr_wrr(rq);

    /* Pushable task? */
    /*
     * The previous task needs to be made eligible for pushing
     * if it is still active
     */
    // if (on_wrr_rq(&p->wrr) && p->wrr.nr_cpus_allowed > 1)
    // 	enqueue_pushable_task(rq, p);
}
static void set_curr_task_wrr(struct rq* rq) {
    struct task_struct* p = rq->curr;

    p->se.exec_start = rq->clock_task;

    /* Pushable task? */
    /* The running task is never eligible for pushing */
    //    dequeue_pushable_task(rq, p);
}

static void task_tick_wrr(struct rq* rq, struct task_struct* p, int queued) {
    struct sched_wrr_entity* wrr_se = &p->wrr;
    update_curr_wrr(rq);

    /*?*/
    watchdog(rq, p);

    /* If time slice has not used up, we just decrease and return */
    if (--(p->wrr.time_slice))
        return;

    p->wrr.time_slice = p->wrr.weight * WRR_WEIGHT_UNIT;

    /*
     * Requeue to the end of queue if we (and all of our ancestors) are the
     * only element on the queue
     */
    for_each_sched_wrr_entity(wrr_se) {
        if (wrr_se->run_list.prev != wrr_se->run_list.next) {
            requeue_task_wrr(rq, p, 0);
            set_tsk_need_resched(p);
            return;
        }
    }
}

static unsigned int get_rr_interval_wrr(struct rq* rq,
                                        struct task_struct* task) {
    /*
     * Time slice is 0 for SCHED_FIFO tasks
     */
    if (task->policy == SCHED_WRR)
        return task->wrr.weight * WRR_WEIGHT_UNIT;
    else
        return 0;
}

/* 4. Main implementation */
static void requeue_task_wrr(struct rq* rq, struct task_struct* p,
                             int head) {
    struct sched_wrr_entity* wrr_se = &p->wrr;
    struct wrr_rq* wrr_rq;

    for_each_sched_wrr_entity(wrr_se) {
        wrr_rq = wrr_rq_of_se(wrr_se);
        requeue_wrr_entity(wrr_rq, wrr_se, head);
    }
}

static void enqueue_wrr_entity(struct sched_wrr_entity* wrr_se, bool head) {
    for_each_sched_wrr_entity(wrr_se) {
        struct wrr_rq* wrr_rq   = wrr_rq_of_se(wrr_se);
        struct list_head* queue = &(wrr_rq->active);
        /* It seems that rt.c here uses some strange never-use code, only get
         * the functional one here. */
        requeue_wrr_entity(wrr_rq, wrr_se, head);
        update_weight(wrr_se);
        inc_wrr_tasks(wrr_se, wrr_rq);
    }
}
static void dequeue_wrr_entity(struct sched_wrr_entity* wrr_se) {
    list_del_init(&wrr_se->run_list);
    dec_wrr_tasks(wrr_se, wrr_rq_of_se(wrr_se));
}

static void requeue_wrr_entity(struct wrr_rq* wrr_rq,
                               struct sched_wrr_entity* wrr_se, int head) {
    if (on_wrr_rq(wrr_se)) {
        struct list_head* queue = &wrr_rq->active;

        if (head)
            list_move(&wrr_se->run_list, queue);
        else
            list_move_tail(&wrr_se->run_list, queue);
    }
}
/*
 * Update the current task's runtime statistics. Skip current tasks that
 * are not in our scheduling class.
 */
static void update_curr_wrr(struct rq* rq) {
    struct task_struct* curr        = rq->curr;
    struct sched_wrr_entity* wrr_se = &curr->wrr;
    struct wrr_rq* wrr_rq           = wrr_rq_of_se(wrr_se);
    u64 delta_exec;

    if (curr->sched_class != &wrr_sched_class)
        return;

    delta_exec = rq->clock_task - curr->se.exec_start;
    if (unlikely((s64)delta_exec < 0))
        delta_exec = 0;

    schedstat_set(curr->se.statistics.exec_max,
                  max(curr->se.statistics.exec_max, delta_exec));

    curr->se.sum_exec_runtime += delta_exec;
    account_group_exec_runtime(curr, delta_exec); /*?*/

    curr->se.exec_start = rq->clock_task;
    cpuacct_charge(curr, delta_exec); /*?*/

    /* No such things */
    // sched_wrr_avg_update(rq, delta_exec);

    /* Some runtime thing? */

    // if (!wrr_bandwidth_enabled())
    // 	return;

    // for_each_sched_wrr_entity(wrr_se) {
    // 	wrr_rq = wrr_rq_of_se(wrr_se);

    // 	if (sched_wrr_runtime(wrr_rq) != RUNTIME_INF) {
    // 		raw_spin_lock(&wrr_rq->wrr_runtime_lock);
    // 		wrr_rq->wrr_time += delta_exec;
    // 		if (sched_wrr_runtime_exceeded(wrr_rq))
    // 			resched_task(curr);
    // 		raw_spin_unlock(&wrr_rq->wrr_runtime_lock);
    // 	}
    // }
}

/* With FG/BG switching we may have wrong weight set. */
static void update_weight(struct sched_wrr_entity* wrr_se) {
    struct task_struct* p = wrr_task_of(wrr_se);
    struct task_group* tg = task_group(p);
    char* path            = task_group_path(tg);
    if (path[1] == 'b' /*(background)*/ && p->wrr.weight != WRR_BG_WEIGHT) {
        p->wrr.time_slice = WRR_BG_WEIGHT * WRR_WEIGHT_UNIT;
        p->wrr.weight     = WRR_BG_WEIGHT;
    }

    if (path[1] != 'b' /*(front)*/ && p->wrr.weight != WRR_FG_WEIGHT) {
        p->wrr.time_slice = WRR_FG_WEIGHT * WRR_WEIGHT_UNIT;
        p->wrr.weight     = WRR_FG_WEIGHT;
    }
}

static void watchdog(struct rq *rq, struct task_struct *p)
{
}

static inline void inc_wrr_tasks(struct sched_wrr_entity* wrr_se,
                                 struct wrr_rq* wrr_rq) {
    wrr_rq.total_weight += wrr_se->weight;
    ++(wrr_rq.wrr_nr_running);
}

static inline void dec_wrr_tasks(struct sched_wrr_entity* wrr_se,
                                 struct wrr_rq* wrr_rq) {
    wrr_rq.total_weight -= wrr_se->weight;
    --(wrr_rq.wrr_nr_running);
}

static inline struct sched_wrr_entity*
pick_next_wrr_entity(struct rq* rq, struct wrr_rq* wrr_rq) {
    return list_first_entry(&(wrr_rq->active), struct sched_wrr_entity,
                            run_list);
}

/* 5. Dummy functions */
static int select_task_rq_wrr(struct task_struct * p, int sd_flag,
                                int flags) {
    return 0;
}
static void set_cpus_allowed_wrr(struct task_struct * p,
                                    const struct cpumask* new_mask) {}
static void rq_online_wrr(struct rq * rq) {}
static void rq_offline_wrr(struct rq * rq) {}
static void pre_schedule_wrr(struct rq * rq, struct task_struct * prev) {
}
static void post_schedule_wrr(struct rq * rq) {}
static void task_woken_wrr(struct rq * rq, struct task_struct * p) {}
static void switched_from_wrr(struct rq * rq, struct task_struct * p) {}
static void prio_changed_wrr(struct rq * rq, struct task_struct * p,
                                int oldprio) {}
static void switched_to_wrr(struct rq * rq, struct task_struct * p) {}

#ifdef CONFIG_SMP

#endif