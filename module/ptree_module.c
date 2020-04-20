#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/sched.h>
#include <linux/unistd.h>
MODULE_LICENSE("Dual BSD/GPL");
#define __NR_ptree 356

struct prinfo {
    pid_t parent_pid; // process id of parent
    pid_t pid; // process id
    pid_t first_child_pid; // pid of youngest child
    pid_t next_sibling_pid; // pid of older sibling
    long state; // current state of process
    long uid; // user id of process owner
    char comm[16]; // name of program executed
};