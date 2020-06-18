#include <linux/module.h>
#include <linux/sched.h>
#include <linux/Yimin_oom.h>

#define VERSION "0.0.1"
#define NAME "Yimin-trace"

MODULE_AUTHOR("Yimin Zhao <ym-zhao@qq.com>");
MODULE_DESCRIPTION("This module inplements the ptree instruction for Android");
MODULE_LICENSE("Dual BSD/GPL");


static void probe_Yimin_eventDoFork(void *ignore, uid_t p_uid)
{
	//printk(KERN_ERR "init !! -> uid = %lu\n", p_uid);
}

static void probe_Yimin_eventDoExit(void *ignore, uid_t p_uid)
{
	//printk(KERN_ERR "exit !! -> uid = %lu\n", p_uid);
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