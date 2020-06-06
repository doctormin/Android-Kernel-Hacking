#ifndef	_Yimin_struct_H
#define _Yimin_struct_H

struct Yimin_struct {
    unsigned long mm_entries[200][2];
};

struct Yimin_struct Yimin_mm_limits = {
    .mm_entries = {{0}}
};



#endif /*_Yimin_struct_H*/
