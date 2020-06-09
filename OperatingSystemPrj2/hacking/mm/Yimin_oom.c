#include <linux/Yimin_oom.h>

int find_mm_current_list_index(unsigned long uid, unsigned long  mm_current_list[e_num][2])
{
	int i;
	int availuable_index = -1;
	for(i = 0; i < e_num; i++)
	{
		if(mm_current_list[i][0] == uid)
			return i;
		if(mm_current_list[i][1] == 0)
			availuable_index = i;
	}
	return availuable_index;
}

void __Yimin_oom_killer()
{
	//first traverse all processes and collect the ones that are created by the same user.
	/**
	 * Ref:
	 * 	#define for_each_process(p) \
     *		for (p = &init_task ; (p = next_task(p)) != &init_task ;  )    
	 */
	struct task_struct *iterator;
	struct task_struct *p;
	uid_t uid_iter;
	unsigned long rss_iter;
	int index;
	unsigned long mm_current_list[e_num][2] = {{0}}; //0: uid 1: rss_sum
	int i;

	for_each_process(iterator){
		p = find_lock_task_mm(iterator);
		if (!p)
			continue;
		uid_iter = iterator -> cred -> uid;
		rss_iter = get_mm_rss(p -> mm);
		index = find_mm_current_list_index(uid_iter, mm_current_list);
		//! error handling -> too many uid entries to handle
		if(index == -1)
		{
			printk(KERN_ERR "too many uid entries to handle");
			return;
		}
		mm_current_list[index][0] =  uid_iter;
		mm_current_list[index][1] += rss_iter;
		task_unlock(p);
	}
	//! for debugging
	/*
	for(i = 0; i < e_num; i++)
	{
		if(mm_current_list[i][1] == 0)
			continue;
		printk("uid = %u \t rss = %lu \n", mm_current_list[i][0], mm_current_list[i][1]);
	}
	*/

}
