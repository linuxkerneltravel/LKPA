# 对内核补丁 [[V3,0/2] sched/fair: Fallback to sched-idle CPU in absence of idle CPUs](https://lore.kernel.org/patchwork/cover/1094197/) 详解
-------

## <center>简要说明</center>
![head](./images/head.png)

当前处于 : `[V3,0/2] sched/fair: Fallback to sched-idle CPU in absence of idle CPUs`
`V3`表示第三版,`0/2`表示当前为概述,且后续有两篇有序的`patch`。
右上角的`mbox`点击会下载一份`.mbox`文件,内容即是`Message`。
右上角的`series`点击会下载一份`.patch`文件,内容包含这组`patch`的所有改动。

-------
## <center>概述</center>

### 背景
当某个进程`由睡眠态到可运行态`时,会存在一个选核的问题。
### 问题
如何保证该进程最快得到执行?
1. 查看是否有`idle cpu`,如若有将其唤醒,并且将该进程放入`run queue`中即可。
2. 查看是否有只运行了`SCHED_IDLE`策略的进程,根据进程优先级抢占,该就绪态进程会抢占正在运行的`SCHED_IDLE`策略进程,于是可以保证该进程很快地运行。


-------
## <center>问题</center>

1. 如何判断一个`CPU`为`idle CPU`？
2. 如何判断一个`CPU`上所有进程都是`SCHED_IDLE`策略？
3. 如何去判断优化是否有效(优化不能只凭借模糊的猜测,应该有真实的测试结果支持)?
4. 如若已有一个函数可判断某个`CPU`上所有进程是否都是`SCHED_IDLE`策略,那么应该在哪里调用这个函数保证进程的最快执行?
-------
## <center> [V3,0/2] sched/fair: Fallback to sched-idle CPU in absence of idle CPUs </center>
### Message
```
We try to find an idle CPU to run the next task, but in case we don't
find an idle CPU it is better to pick a CPU which will run the task the
soonest, for performance reason.
```
我们试图寻找一个`idle CPU`去运行下一个任务,但是如果我们找不到空闲的`CPU`,出于性能原因,最好选择一个运行任务最快的`CPU`。
```
A CPU which isn't idle but has only SCHED_IDLE activity queued on it
should be a good target based on this criteria as any normal fair task
will most likely preempt the currently running SCHED_IDLE task
immediately. In fact, choosing a SCHED_IDLE CPU over a fully idle one
shall give better results as it should be able to run the task sooner
than an idle CPU (which requires to be woken up from an idle state).

This patchset updates both fast and slow paths with this optimization.
```
根据这个标准,不空闲但`只有SCHED_IDLE task的CPU`应该是一个很好的目标,因为任何正常的公平任务都很可能立即抢占当前运行的`SCHED_IDLE task`。实际上,选择`只有SCHED_IDLE task的CPU`而不是`idle CPU`将获得更好的结果,因为它应该能够比`idle CPU`(需要从空闲状态中唤醒)更快地运行任务。
这里阐述了选择`只有SCHED_IDLE task的CPU`的好处 : 
1. 因为 `idle cpu` 需要唤醒,唤醒时间可能比抢占的时间更长。
2. `idle cpu` 如若唤醒,需要更大的功耗(部分cpu支持idle模式下低功耗运行)。
```
Testing is done with the help of rt-app currently and here are the
details:
```
测试是在rt-app(rt-app是一个开源项目,请点击[链接](https://github.com/scheduler-tools/rt-app))的帮助下完成的，这里是
细节如下:
```
- Tested on Octacore Hikey platform (all CPUs change frequency
  together).
```
测试是在`Octacore Hikey platform`上进行的(所有cpu一起改变频率,`一般嵌入式或者终端芯片都是一组cpu一起调频的。per cpu单独调频，硬件和软件成本都很高。本身硬件设计成本也高`)
```
- rt-app json [1] creates few tasks and we monitor the scheduling
  latency for them by looking at "wu_lat" field (usec).
```
通过`rt-app json`指令创建了一些任务,`json`配置告知了`rt-app`需要创建哪些任务([rt-app 的 json配置链接](https://pastebin.com/TMHGGBxD)),我们通过查看"wu_lat"字段(usec)来监控它们的调度延迟。
```
- The histograms are created using
  https://github.com/adkein/textogram: textogram -a 0 -z 1000 -n 10
```
histograms的项目请点击[链接](https://github.com/adkein/textogram),使用指令`textogram -a 0 -z 1000 -n 10`运行。
```
- the stats are accumulated using: https://github.com/nferraz/st
```
统计数据是通过使用[st项目](https://github.com/nferraz/st)积累的。
```
- NOTE: The % values shown don't add up, just look at total numbers
  instead
```
注意 : 这里显示的%值并没有加起来,只是查看总数。
```
Test 1: Create 8 CFS tasks (no SCHED_IDLE tasks) without this patchset:

      0 - 100  : ##################################################   72% (3688)
    100 - 200  : ################                                     24% (1253)
    200 - 300  : ##                                                    2% (149)
    300 - 400  :                                                       0% (22)
    400 - 500  :                                                       0% (1)
    500 - 600  :                                                       0% (3)
    600 - 700  :                                                       0% (1)
    700 - 800  :                                                       0% (1)
    800 - 900  :
    900 - 1000 :                                                       0% (1)
         >1000 : 0% (17)

   N       min     max     sum     mean    stddev
   5136    0       2452    535985  104.358 104.585
```


```
Test 2: Create 8 CFS tasks and 5 SCHED_IDLE tasks:

        A. Without sched-idle patchset:

      0 - 100  : ##################################################   88% (3102)
    100 - 200  : ##                                                    4% (148)
    200 - 300  :                                                       1% (41)
    300 - 400  :                                                       0% (27)
    400 - 500  :                                                       0% (33)
    500 - 600  :                                                       0% (32)
    600 - 700  :                                                       1% (36)
    700 - 800  :                                                       0% (27)
    800 - 900  :                                                       0% (19)
    900 - 1000 :                                                       0% (26)
         >1000 : 34% (1218)

   N       min     max     sum             mean    stddev
   4710    0       67664   5.25956e+06     1116.68 2315.09


        B. With sched-idle patchset:

      0 - 100  : ##################################################   99% (5042)
    100 - 200  :                                                       0% (8)
    200 - 300  :
    300 - 400  :
    400 - 500  :                                                       0% (2)
    500 - 600  :                                                       0% (1)
    600 - 700  :
    700 - 800  :                                                       0% (1)
    800 - 900  :                                                       0% (1)
    900 - 1000 :
         >1000 : 0% (40)

   N       min     max     sum     mean    stddev
   5095    0       7773    523170  102.683 475.482
```
```
The mean latency dropped to 10% and the stddev to around 25% with this
patchset.
```
### Comments
##### `Peter Zijlstra`对`patch`的评论 :
```
On Wed, Jun 26, 2019 at 10:36:28AM +0530, Viresh Kumar wrote:
> Hi,
> 
> We try to find an idle CPU to run the next task, but in case we don't
> find an idle CPU it is better to pick a CPU which will run the task the
> soonest, for performance reason.
> 
> A CPU which isn't idle but has only SCHED_IDLE activity queued on it
> should be a good target based on this criteria as any normal fair task
> will most likely preempt the currently running SCHED_IDLE task
> immediately. In fact, choosing a SCHED_IDLE CPU over a fully idle one
> shall give better results as it should be able to run the task sooner
> than an idle CPU (which requires to be woken up from an idle state).
> 
> This patchset updates both fast and slow paths with this optimization.

So this basically does the trivial SCHED_IDLE<-* wakeup preemption test;
one could consider doing the full wakeup preemption test instead.

Now; the obvious argument against doing this is cost; esp. the cgroup
case is very expensive I suppose. But it might be a fun experiment to
try.

That said; I'm tempted to apply these patches..
```
所以这基本上是做简单的`SCHED_IDLE<-*`唤醒抢占测试;可以考虑进行完全唤醒抢占测试。

现在;反对这样做的明显理由是成本;我想，特别是cgroup的情况是非常昂贵的。但这可能是一个有趣的尝试。

也就是说;我很想用这些`patches`。
##### `Viresh Kumar`回复`Peter Zijlstra` :
```
On 01-07-19, 15:43, Peter Zijlstra wrote:
> On Wed, Jun 26, 2019 at 10:36:28AM +0530, Viresh Kumar wrote:
> > Hi,
> > 
> > We try to find an idle CPU to run the next task, but in case we don't
> > find an idle CPU it is better to pick a CPU which will run the task the
> > soonest, for performance reason.
> > 
> > A CPU which isn't idle but has only SCHED_IDLE activity queued on it
> > should be a good target based on this criteria as any normal fair task
> > will most likely preempt the currently running SCHED_IDLE task
> > immediately. In fact, choosing a SCHED_IDLE CPU over a fully idle one
> > shall give better results as it should be able to run the task sooner
> > than an idle CPU (which requires to be woken up from an idle state).
> > 
> > This patchset updates both fast and slow paths with this optimization.
> 
> So this basically does the trivial SCHED_IDLE<-* wakeup preemption test;

Right.
```


```
> one could consider doing the full wakeup preemption test instead.

I am not sure what you meant by "full wakeup preemption test" :(
```

```
> Now; the obvious argument against doing this is cost; esp. the cgroup
> case is very expensive I suppose. But it might be a fun experiment to
> try.

> That said; I'm tempted to apply these patches..

Please do, who is stopping you :)
```


-------
## <center> [V3,1/2] sched: Start tracking SCHED_IDLE tasks count in cfs_rq </center>
### Commit Message
```
Track how many tasks are present with SCHED_IDLE policy in each cfs_rq.
This will be used by later commits.

Signed-off-by: Viresh Kumar <viresh.kumar@linaro.org>
---
 kernel/sched/fair.c  | 14 ++++++++++++--
 kernel/sched/sched.h |  2 ++
 2 files changed, 14 insertions(+), 2 deletions(-)
```
跟踪每个 `cfs_rq` 中有多少任务使用`SCHED_IDLE` 策略(通过添加一个字段来实现)。
这将在以后的提交中使用。
### Patch
```
diff --git a/kernel/sched/sched.h b/kernel/sched/sched.h
index 802b1f3405f2..1f95411f5e61 100644
--- a/kernel/sched/sched.h
+++ b/kernel/sched/sched.h
@@ -484,6 +484,8 @@  struct cfs_rq {
 	unsigned long		runnable_weight;
 	unsigned int		nr_running;
 	unsigned int		h_nr_running;
+	/* h_nr_running for SCHED_IDLE tasks */
+	unsigned int		idle_h_nr_running;
 
 	u64			exec_clock;
 	u64			min_vruntime;
```
添加字段`idle_h_nr_running`表示当前`cfs_rq`内有多少个使用`SCHED_IDLE` 策略的任务。
```
diff --git a/kernel/sched/fair.c b/kernel/sched/fair.c
index 036be95a87e9..1277adc3e7ed 100644
--- a/kernel/sched/fair.c
+++ b/kernel/sched/fair.c
@@ -4500,7 +4500,7 @@  static void throttle_cfs_rq(struct cfs_rq *cfs_rq)
 	struct rq *rq = rq_of(cfs_rq);
 	struct cfs_bandwidth *cfs_b = tg_cfs_bandwidth(cfs_rq->tg);
 	struct sched_entity *se;
-	long task_delta, dequeue = 1;
+	long task_delta, idle_task_delta, dequeue = 1;
 	bool empty;
 
 	se = cfs_rq->tg->se[cpu_of(rq_of(cfs_rq))];
@@ -4511,6 +4511,7 @@  static void throttle_cfs_rq(struct cfs_rq *cfs_rq)
 	rcu_read_unlock();
 
 	task_delta = cfs_rq->h_nr_running;
+	idle_task_delta = cfs_rq->idle_h_nr_running;
 	for_each_sched_entity(se) {
 		struct cfs_rq *qcfs_rq = cfs_rq_of(se);
 		/* throttled entity or throttle-on-deactivate */
@@ -4520,6 +4521,7 @@  static void throttle_cfs_rq(struct cfs_rq *cfs_rq)
 		if (dequeue)
 			dequeue_entity(qcfs_rq, se, DEQUEUE_SLEEP);
 		qcfs_rq->h_nr_running -= task_delta;
+		qcfs_rq->idle_h_nr_running -= idle_task_delta;
 
 		if (qcfs_rq->load.weight)
 			dequeue = 0;
@@ -4559,7 +4561,7 @@  void unthrottle_cfs_rq(struct cfs_rq *cfs_rq)
 	struct cfs_bandwidth *cfs_b = tg_cfs_bandwidth(cfs_rq->tg);
 	struct sched_entity *se;
 	int enqueue = 1;
-	long task_delta;
+	long task_delta, idle_task_delta;
 
 	se = cfs_rq->tg->se[cpu_of(rq)];
 
@@ -4579,6 +4581,7 @@  void unthrottle_cfs_rq(struct cfs_rq *cfs_rq)
 		return;
 
 	task_delta = cfs_rq->h_nr_running;
+	idle_task_delta = cfs_rq->idle_h_nr_running;
 	for_each_sched_entity(se) {
 		if (se->on_rq)
 			enqueue = 0;
@@ -4587,6 +4590,7 @@  void unthrottle_cfs_rq(struct cfs_rq *cfs_rq)
 		if (enqueue)
 			enqueue_entity(cfs_rq, se, ENQUEUE_WAKEUP);
 		cfs_rq->h_nr_running += task_delta;
+		cfs_rq->idle_h_nr_running += idle_task_delta;
 
 		if (cfs_rq_throttled(cfs_rq))
 			break;
@@ -5200,6 +5204,7 @@  enqueue_task_fair(struct rq *rq, struct task_struct *p, int flags)
 {
 	struct cfs_rq *cfs_rq;
 	struct sched_entity *se = &p->se;
+	int idle_h_nr_running = task_has_idle_policy(p);
 
 	/*
 	 * The code below (indirectly) updates schedutil which looks at
@@ -5232,6 +5237,7 @@  enqueue_task_fair(struct rq *rq, struct task_struct *p, int flags)
 		if (cfs_rq_throttled(cfs_rq))
 			break;
 		cfs_rq->h_nr_running++;
+		cfs_rq->idle_h_nr_running += idle_h_nr_running;
 
 		flags = ENQUEUE_WAKEUP;
 	}
@@ -5239,6 +5245,7 @@  enqueue_task_fair(struct rq *rq, struct task_struct *p, int flags)
 	for_each_sched_entity(se) {
 		cfs_rq = cfs_rq_of(se);
 		cfs_rq->h_nr_running++;
+		cfs_rq->idle_h_nr_running += idle_h_nr_running;
 
 		if (cfs_rq_throttled(cfs_rq))
 			break;
@@ -5300,6 +5307,7 @@  static void dequeue_task_fair(struct rq *rq, struct task_struct *p, int flags)
 	struct cfs_rq *cfs_rq;
 	struct sched_entity *se = &p->se;
 	int task_sleep = flags & DEQUEUE_SLEEP;
+	int idle_h_nr_running = task_has_idle_policy(p);
 
 	for_each_sched_entity(se) {
 		cfs_rq = cfs_rq_of(se);
@@ -5314,6 +5322,7 @@  static void dequeue_task_fair(struct rq *rq, struct task_struct *p, int flags)
 		if (cfs_rq_throttled(cfs_rq))
 			break;
 		cfs_rq->h_nr_running--;
+		cfs_rq->idle_h_nr_running -= idle_h_nr_running;
 
 		/* Don't dequeue parent if it has other entities besides us */
 		if (cfs_rq->load.weight) {
@@ -5333,6 +5342,7 @@  static void dequeue_task_fair(struct rq *rq, struct task_struct *p, int flags)
 	for_each_sched_entity(se) {
 		cfs_rq = cfs_rq_of(se);
 		cfs_rq->h_nr_running--;
+		cfs_rq->idle_h_nr_running -= idle_h_nr_running;
 
 		if (cfs_rq_throttled(cfs_rq))
 			break;
```
当当前`cfs_rq`内使用`SCHED_IDLE`策略的任务数发生变化时,进行相应修改来维护`idle_h_nr_running`属性。

-------
## <center> [V3,2/2] sched/fair: Fallback to sched-idle CPU if idle CPU isn't found </center>
### Commit Message
```
We try to find an idle CPU to run the next task, but in case we don't
find an idle CPU it is better to pick a CPU which will run the task the
soonest, for performance reason.

A CPU which isn't idle but has only SCHED_IDLE activity queued on it
should be a good target based on this criteria as any normal fair task
will most likely preempt the currently running SCHED_IDLE task
immediately. In fact, choosing a SCHED_IDLE CPU over a fully idle one
shall give better results as it should be able to run the task sooner
than an idle CPU (which requires to be woken up from an idle state).

This patch updates both fast and slow paths with this optimization.

Signed-off-by: Viresh Kumar <viresh.kumar@linaro.org>
---
 kernel/sched/fair.c | 43 +++++++++++++++++++++++++++++++++----------
 1 file changed, 33 insertions(+), 10 deletions(-)
```
以上`Commit Message`与`[V3,0/2]`的`Message`完全相同,故不赘述。

### Patch
```
diff --git a/kernel/sched/fair.c b/kernel/sched/fair.c
index 1277adc3e7ed..2e0527fd468c 100644
--- a/kernel/sched/fair.c
+++ b/kernel/sched/fair.c
```
此次`patch`仅修改了`kernel/sched/fair.c`,由于修复篇幅较大,以函数为单位进行分析。
```
@@ -5376,6 +5376,15 @@  static struct {
 
 #endif /* CONFIG_NO_HZ_COMMON */
 
+/* CPU only has SCHED_IDLE tasks enqueued */
+static int sched_idle_cpu(int cpu)
+{
+	struct rq *rq = cpu_rq(cpu);
+
+	return unlikely(rq->nr_running == rq->cfs.idle_h_nr_running &&
+			rq->nr_running);
+}
+

 static unsigned long cpu_runnable_load(struct rq *rq)
 {
 	return cfs_rq_runnable_load_avg(&rq->cfs);
```
如若该`CPU`不是`idle CPU`(`rq->nr_running != 0`)且只运行了`SCHED_IDLE`策略的任务(`rq->nr_running == rq->cfs.idle_h_nr_running`),返回非零值。
否则返回0。
```
@@ -5698,7 +5707,7 @@  find_idlest_group_cpu(struct sched_group *group, struct task_struct *p, int this
 	unsigned int min_exit_latency = UINT_MAX;
 	u64 latest_idle_timestamp = 0;
 	int least_loaded_cpu = this_cpu;
-	int shallowest_idle_cpu = -1;
+	int shallowest_idle_cpu = -1, si_cpu = -1;
 	int i;
 
 	/* Check if we have any choice: */

@@ -5729,7 +5738,12 @@  find_idlest_group_cpu(struct sched_group *group, struct task_struct *p, int this
 				latest_idle_timestamp = rq->idle_stamp;
 				shallowest_idle_cpu = i;
 			}
-		} else if (shallowest_idle_cpu == -1) {
+		} else if (shallowest_idle_cpu == -1 && si_cpu == -1) {
+			if (sched_idle_cpu(i)) {
+				si_cpu = i;
+				continue;
+			}
+
 			load = cpu_runnable_load(cpu_rq(i));
 			if (load < min_load) {
 				min_load = load;
@@ -5738,7 +5752,11 @@  find_idlest_group_cpu(struct sched_group *group, struct task_struct *p, int this
 		}
 	}
 
-	return shallowest_idle_cpu != -1 ? shallowest_idle_cpu : least_loaded_cpu;
+	if (shallowest_idle_cpu != -1)
+		return shallowest_idle_cpu;
+	if (si_cpu != -1)
+		return si_cpu;
+	return least_loaded_cpu;
 }
```
`find_idlest_group_cpu`—从组内CPU中找出最空闲的`CPU`。
```
 static inline int find_idlest_cpu(struct sched_domain *sd, struct task_struct *p,
@@ -5891,7 +5909,7 @@  static int select_idle_core(struct task_struct *p, struct sched_domain *sd, int
  */
 static int select_idle_smt(struct task_struct *p, int target)
 {
-	int cpu;
+	int cpu, si_cpu = -1;
 
 	if (!static_branch_likely(&sched_smt_present))
 		return -1;
@@ -5901,9 +5919,11 @@  static int select_idle_smt(struct task_struct *p, int target)
 			continue;
 		if (available_idle_cpu(cpu))
 			return cpu;
+		if (si_cpu == -1 && sched_idle_cpu(cpu))
+			si_cpu = cpu;
 	}
 
-	return -1;
+	return si_cpu;
 }
 
 #else /* CONFIG_SCHED_SMT */
@@ -5931,7 +5951,7 @@  static int select_idle_cpu(struct task_struct *p, struct sched_domain *sd, int t
 	u64 avg_cost, avg_idle;
 	u64 time, cost;
 	s64 delta;
-	int cpu, nr = INT_MAX;
+	int cpu, nr = INT_MAX, si_cpu = -1;
 	int this = smp_processor_id();
 
 	this_sd = rcu_dereference(*this_cpu_ptr(&sd_llc));
@@ -5960,11 +5980,13 @@  static int select_idle_cpu(struct task_struct *p, struct sched_domain *sd, int t
 
 	for_each_cpu_wrap(cpu, sched_domain_span(sd), target) {
 		if (!--nr)
-			return -1;
+			return si_cpu;
 		if (!cpumask_test_cpu(cpu, p->cpus_ptr))
 			continue;
 		if (available_idle_cpu(cpu))
 			break;
+		if (si_cpu == -1 && sched_idle_cpu(cpu))
+			si_cpu = cpu;
 	}
 
 	time = cpu_clock(this) - time;
@@ -5983,13 +6005,14 @@  static int select_idle_sibling(struct task_struct *p, int prev, int target)
 	struct sched_domain *sd;
 	int i, recent_used_cpu;
 
-	if (available_idle_cpu(target))
+	if (available_idle_cpu(target) || sched_idle_cpu(target))
 		return target;
 
 	/*
 	 * If the previous CPU is cache affine and idle, don't be stupid:
 	 */
-	if (prev != target && cpus_share_cache(prev, target) && available_idle_cpu(prev))
+	if (prev != target && cpus_share_cache(prev, target) &&
+	    (available_idle_cpu(prev) || sched_idle_cpu(prev)))
 		return prev;
 
 	/* Check a recently used CPU as a potential idle candidate: */
@@ -5997,7 +6020,7 @@  static int select_idle_sibling(struct task_struct *p, int prev, int target)
 	if (recent_used_cpu != prev &&
 	    recent_used_cpu != target &&
 	    cpus_share_cache(recent_used_cpu, target) &&
-	    available_idle_cpu(recent_used_cpu) &&
+	    (available_idle_cpu(recent_used_cpu) || sched_idle_cpu(recent_used_cpu)) &&
 	    cpumask_test_cpu(p->recent_used_cpu, p->cpus_ptr)) {
 		/*
 		 * Replace recent_used_cpu with prev as it is a potential
```

### Comments