#ifndef ___wrr___
#define ___wrr___

#ifndef _wrr_included_
#define _wrr_included_

#include "sched.h"
#include <linux/slab.h>

#endif
/* Header file used for WRR. */

/* 1. sched class definition */

/* 2. Functional Tools */
/* These are different depend on setting CONFIG_WRR_GROUP_SCHED */
static inline struct rq* rq_of_wrr_rq(struct wrr_rq* wrr_rq);
static inline struct wrr_rq* wrr_rq_of_se(struct sched_wrr_entity* wrr_se);
static inline struct wrr_rq *group_wrr_rq(struct sched_wrr_entity *wrr_se);
static inline struct task_struct*
wrr_task_of(struct sched_wrr_entity* wrr_se);
static inline void list_add_leaf_wrr_rq(struct wrr_rq *wrr_rq);

static inline int on_wrr_rq(struct sched_wrr_entity* wrr_se);
static char* task_group_path(struct task_group* tg);

#ifdef CONFIG_WRR_GROUP_SCHED
#define for_each_sched_wrr_entity(wrr_se) \
	for (; wrr_se; wrr_se = wrr_se->parent)
#else
#define for_each_sched_wrr_entity(wrr_se) \
    for (; wrr_se; wrr_se = NULL)
#endif

/* 3. Essential functions in sched_class */

static void enqueue_task_wrr(struct rq* rq, struct task_struct* p,
                             int flags);
static void dequeue_task_wrr(struct rq* rq, struct task_struct* p,
                             int flags);
static void yield_task_wrr(struct rq* rq);
static void check_preempt_curr_wrr(struct rq* rq, struct task_struct* p,
                                   int flags);
static struct task_struct* pick_next_task_wrr(struct rq* rq);
static void put_prev_task_wrr(struct rq* rq, struct task_struct* p);

static void set_curr_task_wrr(struct rq* rq);
static void task_tick_wrr(struct rq* rq, struct task_struct* p, int queued);
static unsigned int get_rr_interval_wrr(struct rq* rq,
                                        struct task_struct* task);

void init_wrr_rq(struct wrr_rq* wrr_rq, struct rq* rq);

/* 4. Main implementation */
static void requeue_task_wrr(struct rq* rq, struct task_struct* p, int head);
static void enqueue_wrr_entity(struct sched_wrr_entity* wrr_se, bool head);
static void dequeue_wrr_entity(struct sched_wrr_entity* wrr_se);
static void requeue_wrr_entity(struct wrr_rq* wrr_rq,
                               struct sched_wrr_entity* wrr_se, int head);
static void update_curr_wrr(struct rq* rq);
static void update_weight(struct sched_wrr_entity* wrr_se);
static void watchdog(struct rq* rq, struct task_struct* p);
static inline void inc_wrr_tasks(struct sched_wrr_entity* wrr_se,
                                 struct wrr_rq* wrr_rq);
static inline void dec_wrr_tasks(struct sched_wrr_entity* wrr_se,
                                 struct wrr_rq* wrr_rq);
static inline struct sched_wrr_entity*
pick_next_wrr_entity(struct rq* rq, struct wrr_rq* wrr_rq);
void init_tg_wrr_entry(struct task_group* tg, struct wrr_rq* wrr_rq,
                       struct sched_wrr_entity* wrr_se, int cpu,
                       struct sched_wrr_entity* parent);
/* 5. Dummy functions */
#ifdef CONFIG_SMP
static int select_task_rq_wrr(struct task_struct* p, int sd_flag, int flags);
static void set_cpus_allowed_wrr(struct task_struct* p,
                                 const struct cpumask* new_mask);
static void rq_online_wrr(struct rq* rq);
static void rq_offline_wrr(struct rq* rq);
static void pre_schedule_wrr(struct rq* rq, struct task_struct* prev);
static void post_schedule_wrr(struct rq* rq);
static void task_woken_wrr(struct rq* rq, struct task_struct* p);
static void switched_from_wrr(struct rq* rq, struct task_struct* p);
#endif
static void prio_changed_wrr(struct rq* rq, struct task_struct* p,
                             int oldprio);
static void switched_to_wrr(struct rq* rq, struct task_struct* p);

#endif