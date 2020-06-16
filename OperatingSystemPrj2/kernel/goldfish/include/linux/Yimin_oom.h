#ifndef	_Yimin_oom_H
#define _Yimin_oom_H

#include <linux/sched.h>
#include <linux/types.h>
#include <linux/mm.h>
#include <linux/oom.h>
#include <linux/Yimin_struct.h>
#include <linux/signal.h> //for using `do_send_sig_info(...)` in __Yimin_kill(...)
#include <linux/timer.h>  //add `struct timer_list Yimin_timer`
#include <linux/time.h>   //for using `void getnstimeofday`

#define KILLER_TIMEOUT 2 //This is the biggest time interval of endlessly revoking `Yimin_oom_killer` -- now 0.02s

void __Yimin_oom_killer(void);           //Real killer
void Yimin_oom_killer(unsigned long);    /*This function invokes real killer with intervals controlled by `Yimin_timer`*/

#endif /*_Yimin_oom_H*/