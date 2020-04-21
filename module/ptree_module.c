#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/sched.h>
#include <linux/unistd.h>

#define VERSION "0.0.1"
#define NAME "Yimin-ptree"
#define __NR_ptree 356

MODULE_AUTHOR("Yimin Zhao <ym-zhao@qq.com>");
MODULE_DESCRIPTION("This module inplements the ptree instruction for Android");
MODULE_LICENSE("Dual BSD/GPL");

#define MAXBUFFER 3000 //This is the max length for (prinfo*) buf

struct prinfo {
  pid_t parent_pid;        // process id of parent
  pid_t pid;               // process id
  pid_t first_child_pid;   // pid of youngest child (earliest created child)
  pid_t next_sibling_pid;  // pid of older sibling (later created sibling)
  long state;              // current state of process
  long uid;                // user id of process owner
  char comm[64];           // name of program executed
};

void get_prinfo(struct prinfo* tar, struct task_struct* src){
  tar->parent_id        = (src->parent) ? src->parent->pid : 0;
  tar->pid              = src->pid;
  /*reference:
   *in list.h:   int list_empty(const struct list_head *head)
   *in list.h:   #define list_first_entry(ptr, type, member) list_entry((ptr)->next, type, member)
   *in sched.h:  struct list_head children; 
   */
  tar->first_child_pid  = list_empty(&(src->children)) ? 0 : list_first_entry(&(src->children), struct task_struct, siblings)->pid;
  tar->next_sibling_pid = list_empty(&(src->sibling)) ? 0 : list_entry((src->sibling).next, struct task_struct, siblings)->pid;
  tar->state            = src->state
  tar->uid              = src->cred->uid
  /*char *get_task_comm(char *buf, struct task_struct *tsk)*/
  get_task_comm(tar->comm, src)
}

void DFS(struct task_struct* root, struct prinfo* buf, int nr){

}

static int ptree(struct prinfo* buf, int* nr) {
  //allocate space for parameters (in the kernel space)
  int nr_in_kernel; //nr_in_kernel is the counterpart for nr in kernel space
  struct prinfo* buf_in_kernel =  (struct prinfo *)kmalloc(MAXBUFFER * sizeof(struct prinfo), GFP_KERNEL); //buf_in_kernel is the counterpart for buf in kernel space
  
  read_lock(&tasklist_lock);
  DFS(init_task, buf_in_kernel, nr_in_kernel);//main procedure: do DFS search and link the buf_in_kernel in DFS order
  read_unlock(&tasklist_lock);
  /* `copy_to_user` will copy `the second parameter` in the kernel space into `the first parameter` in the user space
   * @the first parameter: void *
   * @thr second parameter: void *
   * return 0 if succeed 
   * return >0 if failed 
  */
  if(copy_to_user(buf, buf_in_kernel, MAXBUFFER * sizeof(struct printfo))){
    //the copy procedure failed
    printk(KERN_ERR "failed in function copy_to_user(buf_in_kernel -> buf)");
    return -1;
  }
  if(copy_to_user(nr, &nr_in_kernel, sizeof(int))){
    //the copy procedure failed
    printk(KERN_ERR "failed in function copy_to_user(nr_in_kernel -> nr)");
    return -1;
  }

  kfree(buf_in_kernel);
  return 0;
}


static int (*oldcall)(void);  // function pointer
static int ptree_init(void) {
  // syscall points to the syscall table
  long* syscall = (long*)0xc000d8c4;
  oldcall = (int (*)(void))(syscall[__NR_ptree]);  // preserve the original sys call
  syscall[__NR_ptree] = (unsigned long)ptree;
  printk(KERN_INFO "%s: version %s loaded successfully\n", NAME, VERSION);
  return 0;
}

static void ptree_exit(void) {
  long* syscall = (long*)0xc000d8c4;
  syscall[__NR_ptree] = (unsigned long)oldcall;  // retrieve the original sys call
  printk(KERN_INFO "%s: version %s unloaded successfully\n", NAME, VERSION);
}

module_init(ptree_init);
module_exit(ptree_exit);