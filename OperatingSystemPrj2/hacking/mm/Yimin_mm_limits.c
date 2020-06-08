#include <linux/Yimin_struct.h>

DEFINE_MUTEX(Yimin_mutex); //statically define a mutex

struct Yimin_struct Yimin_mm_limits = {
    .mm_entries = {{0}}
};
