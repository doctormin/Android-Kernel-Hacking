#include <linux/Yimin_oom.h>

static unsigned long mm_current_list[e_num][2] = {{0}}; 
/* mm_current_list[][0]: uid  
   mm_current_list[][1]: rss of the user 
*/
static unsigned long long mm_overcommit[e_num][4] = {{0}};
/* mm_overcommit[][0]: uid  
   mm_overcommit[][1]: exceed begin time
   mm_overcommit[][2]: valid bit
   mm_overcommit[][3]: touched bit //indicate whether touched during last call
*/

struct timer_list Yimin_timer; 

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
static int find_mm_current_list_index(unsigned long uid)
{
	int i;
	int availuable_index;

	availuable_index = -1;

	for(i = 0; i < e_num; i++)
	{
		if(mm_current_list[i][0] == uid)
			return i;
		if(mm_current_list[i][1] == 0) //This means that this entry is empty
			availuable_index = i;
	}
	return availuable_index;
}

static int find_mm_overcommit_index(unsigned long long uid)
{
	int i;
	int availuable_index;

	availuable_index = -1;

	for(i = 0; i < e_num; i++)
	{
		if(mm_current_list[i][0] == uid)
			return i;
		if(mm_current_list[i][2] == 0) //valid bit == 0 -> untouched in last round -> empty entry
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
static void __Yimin_kill(uid_t uid,
				  		 unsigned long usr_rss_before_killing, 
				  		 unsigned long mm_max
						)
{
	struct task_struct *iterator;
	struct task_struct *p;
	struct task_struct *killed_process = NULL;
	unsigned long killed_process_rss;
	pid_t killed_process_pid;
	unsigned long pRSS_in_byte;
	unsigned long mm_rss_tmp;
	
	killed_process_rss = 0;
	pRSS_in_byte = 0;

	//* Step 1 - Find the process which has the highest RSS among all processes belonging to the user.
	for_each_process(iterator)
	{
		if (iterator -> cred -> uid != uid)
		{
			//printk(KERN_ERR ">>> uid = %lu\n", iterator -> cred -> uid);
			continue;
		}
			
		p = find_lock_task_mm(iterator);

		if (!p)
		{
			//printk(KERN_ERR "> ERROR ! -> NULL returned by `find_lock_task_mm(iterator)`\n");
			continue;
		}

		mm_rss_tmp = get_mm_rss(p -> mm);
		//printk(KERN_ERR "-----------> get_mm_rss(p->mm) = %lu, pid = %d, killed_process_rss = %lu\n", mm_rss_tmp, p->pid, killed_process_rss);
		if(mm_rss_tmp > killed_process_rss)
		{
			killed_process_rss = mm_rss_tmp;
			//printk(KERN_ERR "-----------> Updated ! killed_process_rss = %lu\n", killed_process_rss);
			killed_process = iterator;
		}
		task_unlock(p);
	}

	if (killed_process == NULL)
	{
		//! error handling
		printk(KERN_ERR "> No killable processes\n");
		return;
	}

	//oom-unkillable cases (either or we panic...)
	if (is_global_init(killed_process) || (killed_process->flags & PF_KTHREAD)) 
	{
		killed_process = NULL;
		printk(KERN_ERR "> Refused to kill init (pid = 1) or kthread (pid = 1) in Yimin's oom killer(which leads to kernel panic)...\n> Let's find the second biggest process...\n");
		for_each_process(iterator)
		{
			if (iterator -> cred -> uid != uid)
				continue;
			p = find_lock_task_mm(iterator);
			if (!p)
				continue;
			if(get_mm_rss(p -> mm) > killed_process_rss)
			{
				if(iterator -> pid == 1 || iterator -> pid == 2)
					continue;
				killed_process_rss = get_mm_rss(p -> mm);
				killed_process = iterator;
			}
			task_unlock(p);
		}
	}

	if (killed_process == NULL)
	{
		//! error handling -> can not find the killed process
		printk(KERN_ERR "> No killable processes other than init (pid = 1) and kthread (pid = 2) !\n");
		return;
	}

	killed_process_pid = killed_process -> pid;

	//* Step 2 - Kill the chosen process 
	pRSS_in_byte =  killed_process_rss * mm_page_size;
	
	printk(KERN_ERR "Yimin's oom killer : uid = %lu, uRSS = %luB, mm_max = %luB, pid = %d, pRSS = %luB\n", 
		    uid, usr_rss_before_killing, mm_max, killed_process_pid, pRSS_in_byte);
	
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
void __Yimin_oom_killer(void)
{
	struct task_struct *iterator;
	struct task_struct *p;
	uid_t uid_iter;
	unsigned long rss_iter;
	int index;
	int o_index;
	int i;
	

	extern struct Yimin_struct Yimin_mm_limits;
	extern struct mutex Yimin_mutex;
	int valid;
	unsigned long mm_uid;
	unsigned long mm_max;
	unsigned long time_allow_exceed;
	unsigned long rss_in_byte;
	__u64         Yimin_time_now;
	struct timespec Yimin_ts;

	rss_iter = 0;

	for(i = 0; i < e_num; i++){
		mm_current_list[i][0] = 0;
		mm_current_list[i][1] = 0;
		mm_overcommit[i][3] = 0; //all set untouched
	}

	//* Step 1 - Traverse all processes and collect the ones that are created by the same user.
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
		
		
		o_index = find_mm_overcommit_index(uid_iter);
		//! error handling -> too many uid entries to handle
		if(o_index == -1)
		{
			printk(KERN_ERR "too many uid entries to handle");
			return;
		}
		mm_overcommit[o_index][0] = uid_iter;	//maintain uid -> case1: already in the entries || case2: newly come
		mm_overcommit[o_index][2] = 1; 			//set valid
		mm_overcommit[o_index][3] = 1; 			//set touched

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
		printk("uid = %u \t rss = %luB\n", mm_current_list[i][0], mm_current_list[i][1] * mm_page_size);
	}*/

	//* Step 2 - delete untouched item
	for(i = 0; i < e_num; i++)
	{
		if(!mm_overcommit[i][3])
		{
			mm_overcommit[i][0] = -1;  //uid -> INT_MAX(invalid)
			mm_overcommit[i][2] = 0;  //set invalid
			mm_overcommit[i][1] = 0;  //overcommit begin time -> 0
		}
	}

	//* Step 3 - Check memory limits set by syscall and kill some processes if needed
	for(i = 0; i < e_num; i++)
	{
		mutex_lock(&Yimin_mutex); //Protect -> MMLimits (i.e. `Yimin_mm_limits` in my prj)
		mm_uid            = Yimin_mm_limits.mm_entries[i][0]; 
		mm_max            = Yimin_mm_limits.mm_entries[i][1];
		valid             = Yimin_mm_limits.mm_entries[i][2];
		time_allow_exceed = Yimin_mm_limits.mm_entries[i][3];
		mutex_unlock(&Yimin_mutex);

		if(!valid)
			continue; //! Invalid entry in MMLimits

		//* Valid entry in MMLimits
		index = find_mm_current_list_index(mm_uid);
		if(index == -1 || mm_current_list[index][1] == 0)
			continue; //! Entry is not a currently running app

		//* Valid entry in memory limits && entry is currently running
		rss_in_byte = mm_current_list[index][1] * mm_page_size;

		o_index = find_mm_overcommit_index(mm_uid);
		//printk(KERN_ERR "uid = %d,\t rss_in_byte = %luB\n", mm_uid, rss_in_byte);
		/*
		*当再次遍历时，首先在另一个超时数组里面检查超时时间
		* 1.如果rss超限	
		* 	1.1 超时时间为0，那么写入系统时间 (第一次发现超时)
		* 	1.2 超时时间不为0，那么求当前系统时间作差，得到超时时间
		* 		1.2.1 假设时间没超，continue
		*  		1.2.2 假设时间超了，那么kill
		*			1.2.2.1 假设kill之后还是超了->下一次调用还该杀->不用改超时开始时间
		*			1.2.2.2 假设kill之后就不超了->超时开始时间改为0
		* 2.如果rss没超限
		* 	更新超时时间为0
		*/
		//No memory overcommit -> set exceed_begin_time = 0
		if(mm_max >= rss_in_byte)
		{
			mm_overcommit[o_index][1] = 0;
		}

		//Memory overcommit
		if(mm_overcommit[o_index][1] == 0)
		{
			//Catched overcommit for the first time
			getnstimeofday(&Yimin_ts);
			mm_overcommit[o_index][1] = Yimin_ts.tv_sec * 1000000000 + Yimin_ts.tv_nsec; //in ns
		} 
		else
		{
			//Catched overcommit again
			getnstimeofday(&Yimin_ts);
			Yimin_time_now = Yimin_ts.tv_sec * 1000000000 +  Yimin_ts.tv_nsec; //in ns
			if((Yimin_time_now - mm_overcommit[o_index][1]) > time_allow_exceed)
			{
				//printk(KERN_ERR "\nuid = %lu, rss_in_byte = %luB  ----> __Yimin_oom_killer triggered !\n", mm_uid, rss_in_byte);
				__Yimin_kill(mm_uid, rss_in_byte, mm_max);
			}
		}
	}
	
}

void Yimin_oom_killer(unsigned long data)
{
	//printk(KERN_ERR "Yimin_oom_killer -> invoked !\n");
	__Yimin_oom_killer();

	//reset `Yimin_timer`
	del_timer(&Yimin_timer);
	Yimin_timer.function = Yimin_oom_killer;
	Yimin_timer.expires = jiffies + KILLER_TIMEOUT;
	add_timer(&Yimin_timer); 
}