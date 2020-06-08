#include <linux/Yimin_oom.h>

void __Yimin_oom_killer()
{
	//first traverse all processes and collect the ones that are created by the same user.
	/**
	 * Ref:
	 * 	#define for_each_process(p) \
     *		for (p = &init_task ; (p = next_task(p)) != &init_task ;  )    
	 */
	struct task_struct *iterator;
	uid_t uid_iter;
	unsigned long rss_iter;

	for_each_process(iterator){
		uid_iter = iterator -> cred -> uid;
		//rss_iter = get_mm_rss(iterator -> mm);
		rss_iter = 0;
		printk("uid = %u \t rss = %lu \n", uid_iter, rss_iter);
	}


}