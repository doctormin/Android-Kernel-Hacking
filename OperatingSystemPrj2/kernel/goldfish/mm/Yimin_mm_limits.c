/*
 * This file defines and initialize Yimin's global variable 
 * `struct Yimin_MMLimits Yimin_mm_limits` 
 */

#include <linux/Yimin_struct.h>

/*
 * Initial Yimin_MMLimits struct
 */
    

struct Yimin_struct Yimin_mm_limits = {
    .mm_entries = {{0}}
};

EXPORT_SYMBOL(Yimin_mm_limits)