#include <linux/module.h>
#include <linux/sched.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/unistd.h>
#include <linux/list.h>
#include <linux/slab.h>
#include <asm/uaccess.h> 
#include <linux/Yimin_oom.h>

#define VERSION "0.0.2"
#define NAME "Yimin-trace"

MODULE_AUTHOR("Yimin Zhao <ym-zhao@qq.com>");
MODULE_DESCRIPTION("This module trace events in do_fork() and do_exit()");
MODULE_LICENSE("Dual BSD/GPL");

/**
 * uid_in_limits - return a bool indicating whether the uid is in the memory limits  list
 * @uid: the uid
 */
static int uid_in_limits(uid_t uid)
{
	extern struct Yimin_struct Yimin_mm_limits;
	extern struct mutex Yimin_mutex;
	int i = 0;
	/**
     *  uid               = Yimin_mm_limits.mm_entries[i][0]; 
	 *	memory limit      = Yimin_mm_limits.mm_entries[i][1];
	 *	valid bit         = Yimin_mm_limits.mm_entries[i][2];
	 *	time_allow_exceed = Yimin_mm_limits.mm_entries[i][3];
     */
	mutex_lock(&Yimin_mutex);
	for(i = 0; i < e_num; i++)
	{
		if( Yimin_mm_limits.mm_entries[i][2] && Yimin_mm_limits.mm_entries[i][0] == uid)
		{
			mutex_unlock(&Yimin_mutex);
			return 1;
		}
	}
	mutex_unlock(&Yimin_mutex);
	return 0;
}

void process_event(uid_t uid, int flag)
{
	static int list_init_flag = 0;
	static int count = 0;
	int i;
	extern struct mutex Yimin_mutex;
	extern int timer_init_flag;
	extern int timer_exit_flag;
	extern struct timer_list Yimin_timer;
	/*watch_table is a list containing the uid which has limits set and is running currently*/
	static unsigned long watch_table[e_num] = {0};
	mutex_lock(&Yimin_mutex);
	if(!timer_init_flag){
		mutex_unlock(&Yimin_mutex);
		return;
	}
	mutex_unlock(&Yimin_mutex);
	if(!list_init_flag)
	{
		/**
		 *scan the whole task_struct to know is there any running processes in the limit list(detected by uid).
		 *This scaning process will happen only once(after the module is installed).
		 *Because after this scanning, evry change made to the list can be detected so there is 
		 *no need to scan the whole list again
		 */
		struct task_struct *iterator;
		struct task_struct *p;
		uid_t uid_iter;
		unsigned long rss_iter;
		for_each_process(iterator)
		{
			p = find_lock_task_mm(iterator);
			if (!p)
				continue;
			uid_iter = iterator -> cred -> uid;
			rss_iter = get_mm_rss(p -> mm);
			task_unlock(p);
			if(rss_iter <= 0)
				continue;
			//initialize the watch_table
			if(uid_in_limits(uid_iter))
			{
				watch_table[count++] = uid_iter;
			}
		}
		if(count > 0)
				Yimin_oom_killer(0);
	}
	if(flag > 0)
	{
		if(uid_in_limits(uid))
		{
			/*
			*The uid of the newly launched task is in the limits list
			*So 1.add it to the watch table
			*   2.begin the timer
			*/
			//printk(KERN_ERR "uid in the limits !\n");
			for(i = 0; i < count; i++)
			{
				if(watch_table[i] == uid)
					return;
			}
			watch_table[count++] = uid;
			Yimin_oom_killer(0);
		}
	}
	else
	{
		unsigned long tmp[e_num];
		int tmp_count;
		int j;
		tmp_count = count;
		j = 0;
		for(i = 0; i < tmp_count; i++)
		{
			if(unlikely(watch_table[i] == uid))
			{
				count--;
				continue;	
			}
			else
			{
				tmp[j++] = watch_table[i];
			}
		}
		for(i = 0; i < j; i++)
			watch_table[i] = tmp[i];
		if(count == 0)
		{
			mutex_lock(&Yimin_mutex);
			if(timer_exit_flag)
			{
				del_timer(&Yimin_timer); 
				timer_exit_flag = 0;
			}
			mutex_unlock(&Yimin_mutex);
		}
	}
}
static void probe_Yimin_eventDoFork(void *ignore, uid_t p_uid)
{	
	//printk(KERN_ERR "init !! -> uid = %lu\n", p_uid);
	process_event(p_uid, 1);
}

static void probe_Yimin_eventDoExit(void *ignore, uid_t p_uid)
{
	//printk(KERN_ERR "exit !! -> uid = %lu\n", p_uid);
	process_event(p_uid, -1);
}

static int __init Yimin_trace_init(void)
{
	int ret;

	ret = register_trace_Yimin_eventDoFork(probe_Yimin_eventDoFork, NULL);
	WARN_ON(ret);
	ret = register_trace_Yimin_eventDoExit(probe_Yimin_eventDoExit, NULL);
	WARN_ON(ret);

    printk(KERN_ERR "%s: version %s loaded successfully\n", NAME, VERSION);
	return 0;
}

static void __exit Yimin_trace_exit(void)
{
	unregister_trace_Yimin_eventDoFork(probe_Yimin_eventDoFork, NULL);
	unregister_trace_Yimin_eventDoExit(probe_Yimin_eventDoExit, NULL);
	tracepoint_synchronize_unregister();
    printk(KERN_ERR "%s: version %s unloaded successfully\n", NAME, VERSION);
}

module_init(Yimin_trace_init);
module_exit(Yimin_trace_exit);