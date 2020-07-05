#include <linux/Yimin_struct.h>
#include <linux/module.h>

DEFINE_MUTEX(Yimin_mutex); //statically define a mutex
EXPORT_SYMBOL(Yimin_mutex);

struct Yimin_struct Yimin_mm_limits = {
    .mm_entries = {{0}}
};
EXPORT_SYMBOL(Yimin_mm_limits);
