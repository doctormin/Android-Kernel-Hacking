#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/sched.h>
#include <linux/unistd.h>

#define VERSION "0.0.1"
#define NAME "Yimin-ptree"
#define __NR_ptree 356

MODULE_AUTHOR("Yimin Zhao <ym-zhao@qq.com");
MODULE_DESCRIPTION("This module inplements the ptree instruction for Android");
MODULE_LICENSE("Dual BSD/GPL");

struct prinfo {
    pid_t parent_pid; // process id of parent
    pid_t pid; // process id
    pid_t first_child_pid; // pid of youngest child
    pid_t next_sibling_pid; // pid of older sibling
    long state; // current state of process
    long uid; // user id of process owner
    char comm[64]; // name of program executed
};

static int ptree(struct prinfo* buf, int* nr)
{ calling ptree is shown in /ptree/testcript.txt


}


static int (*oldcall) (void); //function pointer
static int ptree_init(void)
{
  //syscall points to the syscall table
  long *syscall = (long*)0xc000d8c4;
  oldcall = (int(*)(void))(syscall[__NR_ptree]); //preserve the original sys call
  syscall[__NR_ptree] = (unsigned long)ptree;
  printk(KERN_INFO "%s: version %s loaded successfully\n", NAME, VERSION);
  return 0;
}


static void ptree_exit(void)
{
  long *syscall = (long*)0xc000d8c4;
  syscall[__NR_ptree] = (unsigned long)oldcall; //retrieve the original sys call
  printk(KERN_INFO "%s: version %s unloaded successfully\n", NAME, VERSION);
}


module_init(ptree_init);
module_exit(ptree_exit);