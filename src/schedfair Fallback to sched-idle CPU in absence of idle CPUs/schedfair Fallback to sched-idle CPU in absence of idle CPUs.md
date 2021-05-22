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
1. 因为 idle cpu 需要唤醒,唤醒时间可能比抢占的时间更长。
2. idle cpu 如若唤醒,需要更大的功耗(部分 cpu 支持 idle模式下低功耗运行)。
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
测试是在`Octacore Hikey platform`上进行的(所有cpu一起改变频率)
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
所以这基本上是做简单的SCHED_IDLE<-*唤醒抢占测试;可以考虑进行完全唤醒抢占测试。

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

### Patch

-------
## <center> [V3,2/2] sched/fair: Fallback to sched-idle CPU if idle CPU isn't found </center>
### Commit Message

### Patch

### Comments