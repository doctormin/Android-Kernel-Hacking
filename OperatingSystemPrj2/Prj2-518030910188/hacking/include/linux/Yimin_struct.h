#ifndef	_Yimin_struct_H
#define _Yimin_struct_H

#include <linux/mutex.h>
#define e_num 200
#define mm_page_size 4096
struct Yimin_struct {
    /**
     *  uid               = Yimin_mm_limits.mm_entries[i][0]; 
	 *	memory limit      = Yimin_mm_limits.mm_entries[i][1];
	 *	valid bit         = Yimin_mm_limits.mm_entries[i][2];
	 *	time_allow_exceed = Yimin_mm_limits.mm_entries[i][3];
     */
     unsigned long mm_entries[e_num][4];
};

#endif /*_Yimin_struct_H*/
