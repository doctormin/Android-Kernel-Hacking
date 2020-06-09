#ifndef	_Yimin_struct_H
#define _Yimin_struct_H

#include <linux/mutex.h>
#define e_num 200

struct Yimin_struct {
    unsigned long mm_entries[e_num][3];
};

unsigned long find_mm_limit(unsigned long uid);

#endif /*_Yimin_struct_H*/
