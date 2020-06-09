#include <linux/Yimin_struct.h>

DEFINE_MUTEX(Yimin_mutex); //statically define a mutex

struct Yimin_struct Yimin_mm_limits = {
    .mm_entries = {{0}}
};

unsigned long find_mm_limit(unsigned long uid)
{
    int i = 0;
    for(i = 0; i < 200; i++)
    {
        if(Yimin_mm_limits.mm_entries[i][0] == uid && Yimin_mm_limits.mm_entries[i][2])
        {
            return Yimin_mm_limits.mm_entries[i][1];
        }
    }
    return -1;
}
