#include <linux/Yimin_oom.h>
 
static unsigned long mm_current_list[e_num][2] = {{0}}; /* mm_current_list[][0]: uid  mm_current_list[][1]: rss of the user */

/** 
 * ! This function is called inside `__Yimin_oom_killer()`
 * 
 * find_mm_current_list_index -
 * 	  `mm_current_list[e_num][2]` is a table containing the running app and it's rss sum.
 * 	   1. Return the index of the entry with the wanted uid. 
 * 	   2. If the entry is not in the table, this function will return an 
 *    	  availuable index where is empty.
 * @uid: The uid of the app (of the entry u want to locate) 
 * @mm_current_list: The table which contains uid and its rss
 */
int find_mm_current_list_index(unsigned long uid)
{
	int i;
	int availuable_index = -1;
	for(i = 0; i < e_num; i++)
	{
		if(mm_current_list[i][0] == uid)
			return i;
		if(mm_current_list[i][1] == 0) //This means that this entry is empty
			availuable_index = i;
	}
	return availuable_index;
}

/**
 * ! This function is called inside `__Yimin_oom_killer()`
 * 
 * __Yimin_kill - This function kill the process that has the highest RSS among all processes belonging to `uid`
 * @uid: The user id of the app who exceeds the memory limits set by syscall
 */
void __Yimin_kill(uid_t uid,
				  unsigned long usr_rss_before_killing, 
				  unsigned long mm_max
				 )
{
	struct task_struct *iterator;
	struct task_struct *p;
	struct task_struct *killed_process = NULL;
	unsigned long killed_process_rss = 0;
	pid_t killed_process_pid;

	//* Step 1 - Find the process which has the highest RSS among all processes belonging to the user.
	for_each_process(iterator)
	{
		if (iterator -> cred -> uid != uid)
			continue;
		p = find_lock_task_mm(iterator);
		if (!p)
			continue;
		if(get_mm_rss(p -> mm) > killed_process_rss)
		{
			killed_process_rss = get_mm_rss(p -> mm);
			killed_process = iterator;
		}
		task_unlock(p);
	}
	if (killed_process == NULL)
	{
		//! error handling -> can not find the killed process
		printk(KERN_ERR "can not find the killed process in `__Yimin_kill");
		return;
	}
	killed_process_pid = killed_process -> pid;
	//* Step 2 - Kill the chosen process 
	printk("uid = %d,\t uRSS = %d,\t mm_max = %d,\t pid = %d,\t pRSS = %d", 
		    uid, usr_rss_before_killing, mm_max, killed_process_pid, killed_process_rss);
	do_send_sig_info(SIGKILL, SEND_SIG_FORCED, killed_process, true);
}

/**
 * __Yimin_oom_killer - this is the main procedure of prj2 
 * 	  In this function, my oom-killer will work. It's job can be divided into 2 steps:
 * 	  1. Traverse all processes and collect the ones that are created by the same user.
 * 		 (The infomation will be saved in `mm_current_list[e_num][2] //[index][0]: uid [index][1]: rss_sum`)
 *    2. 
 * 		 2.1. Traverse memory limits for apps set by syscall defined in "sys_arm.c" 
 * 		 	  (i.e. `Yimin_mm_limits` defined in <linux/Yimin_struct> and initialized in "linux/Yimin_mm_limits.c")
 *    	 2.2. Check if any limit is exceeded :
 * 			  If yes : kill the process that has the highest RSS among all processes belonging to the user.
 * 			  If no  : Do nothing and just return
 */
void __Yimin_oom_killer()
{
	struct task_struct *iterator;
	struct task_struct *p;
	uid_t uid_iter;
	unsigned long rss_iter;
	int index;
	int i;

	//* Step 1 - Traverse all processes and collect the ones that are created by the same user.
	for_each_process(iterator){
		p = find_lock_task_mm(iterator);
		if (!p)
			continue;
		uid_iter = iterator -> cred -> uid;
		rss_iter = get_mm_rss(p -> mm);
		task_unlock(p);
		if(rss_iter <= 0)
			continue;
		index = find_mm_current_list_index(uid_iter);
		//! error handling -> too many uid entries to handle
		if(index == -1)
		{
			printk(KERN_ERR "too many uid entries to handle");
			return;
		}
		mm_current_list[index][0] =  uid_iter;
		mm_current_list[index][1] += rss_iter;
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

	//* Step 2 - Check memory limits set by syscall and kill some processes if needed
	extern struct Yimin_struct Yimin_mm_limits;
	for(i = 0; i < e_num; i++)
	{
		if(Yimin_mm_limits.mm_entries[i][2] == 0)
			continue; //! Invalid entry in memory limits

		//* Valid entry in memory limits
		index = find_mm_current_list_index(Yimin_mm_limits.mm_entries[i][0]);
		if(mm_current_list[index][1] == 0 || index == -1)
			continue; //! entry is not currently running app
		
		//* Valid entry in memory limits && entry is currently running
		if(Yimin_mm_limits.mm_entries[i][1] < mm_current_list[index][1])
			__Yimin_kill(mm_current_list[index][0], mm_current_list[index][1], Yimin_mm_limits.mm_entries[i][1]);
	}
	
}
