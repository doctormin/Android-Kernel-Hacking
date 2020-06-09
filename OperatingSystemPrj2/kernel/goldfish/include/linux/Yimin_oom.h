#ifndef	_Yimin_oom_H
#define _Yimin_oom_H

#include <linux/sched.h>
#include <linux/types.h>
#include <linux/mm.h>
#include <linux/oom.h>
#include <linux/Yimin_struct.h>
#include <linux/signal.h> //for using `do_send_sig_info(...)` in __Yimin_kill(...)

void __Yimin_oom_killer();

#endif /*_Yimin_oom_H*/