# OperatingSystemPrj1 -- Android Kernel Hacking

# Problem 1 - Ptree Module

## `real parent` vs. `parent` in `task_struct`

From <u>\include\linux\sched.h</u>

```C
 struct task_struct {
    // ...
    /* real parent process */
    struct task_struct __rcu *real_parent; 

    /* recipient of SIGCHLD, wait4() reports */
    struct task_struct __rcu *parent; 
}
```

From the comment, we can know that `parent` is the recipient of `SIGCHLD` .

So what is the `SIGCHLD` ?

From https://docs.oracle.com/cd/E19455-01/806-4750/signals-7/index.html

>When a child process stops or terminates, `SIGCHLD` is sent to the parent process. The default response to the signal is to ignore it. The signal can be caught and the exit status from the child process can be obtained by immediately calling [wait(2)](https://docs.oracle.com/docs/cd/E19455-01/806-0626/6j9vgh6an/index.html) and [wait3(3C)](https://docs.oracle.com/docs/cd/E19455-01/806-0627/6j9vhfnam/index.html). This allows zombie process entries to be removed as quickly as possible. 

Ok, so basically, the `real_parent` is the process that `fork()` its children.  And in normal cases, the `real_parent` is `parent`.

But sometimes the parent terminates before its children, leaving the children processes becoming zombies.

Then some process need to  take over these zombies (becomes their `parent` ) because the definition of `parent` shows that there must be one process responsible for **receiving SIGHLD** of these zombies.

And from experiments of running code we can know that the `pstree` only shows the `read_parent` .

## Kernel Space and User Space

All the variables excepts parameters of the system call in the module are defined in the kernel space.

```C
int ptree(struct prinfo* buf, int *nr){
}
```

***`buf` and `nr` are in user space***

The memory of kernel space and user space cannot visit each other directly. So in order to modify `buf ` and `nr`. We need to create two counterparts in kernel space (in the code) and use this functions to achieve the modification.

```C
int copy_to_user(void __user *to, const void *from, unsigned long n)
```

## How to Find `pid_t first_child_pid`;

I searched in <u>\include\linux\list.h</u>

```C
static inline int list_empty(const struct list_head *head)
{
	return head->next == head;
}
```

And in `task_struct`:

```C
	struct list_head children;	/* list of my children */
```

which means it needs `list_empty(&(tar->children))` which will return 1 if there is no children

And to fetch the youngest children, we need to locate the first element of the linked list whose head node is `tar->children`, and I search in the `list.h`:

```C
/**
 * list_first_entry - get the first element from a list
 * @ptr:	the list head to take the element from.
 * @type:	the type of the struct this is embedded in.
 * @member:	the name of the list_struct within the struct.
 *
 * Note, that list is expected to be not empty.
 */
#define list_first_entry(ptr, type, member) \
	list_entry((ptr)->next, type, member)
```

Cool!  So this means we have 2 choices:

+ `list_first_entry(&(task->children), struct task_struct, siblings)->pid`
+ `list_entry((task->children).next, struct task_struct, siblings)->pid`

## Choose `sibling` or `children`?

### Background:

I want to carry out a DFS iteration in the process tree:

+ DFS(root)
  + iterating root's sibling
  + for every sibling `node` do `DFS(node)`

### Problem Encountered

Here is part of my DFS function:

```C
struct list_head* iterator;
struct task_struct* next_node; 
//iterating through all the children of the root
list_fot_each(iterator, &(root->children)){
    next_node = list_entry(iterator, struct task_struct, children);
    DFS(next_node, buf, nr);
}
```

There is an error caused by:

```C
next_node = list_entry(iterator, struct task_struct, children);
```

### Solution

The third parameter for `list_entry()` stands for **"the variable name of `iterator` in `task_struct`"**. But unfortunately, `list_fot_each(iterator, &(root->children))`makes it confusing about the identification of `iterator` in its `task_struct`.**Is it `children` or `sibling`?**

```C
struct list_head children;	/* list of my children */
struct list_head sibling;	/* linkage in my parent's children list */
```

The answer is **sibling**. The reasons are as the following.

#### Intuitive Reason

We just need to realize that when DFS is iterating in the for loop --- `list_for_each(next_node...)`, it is actually iterating through the **siblings** of the `next_node`. So the correct code is:

```C
struct task_struct* next_node; 
next_node = list_entry(iterator, struct task_struct, sibling);
```

#### More Convincing Reason

We can find the definition of `list_for_each` in <u>list.h</u>:

```C
#define list_for_each(pos, head) \
		for (pos = (head)->next; pos != (head); pos = pos->next)
```

so the original loop can be written as:

```C
for(iterator = (root->children).next; iterator !=(root->children).next; iterator = iterator->next) {
    next_node = list_entry(iterator, struct task_struct, sibling);
    DFS(next_node, buf, nr);
}
```

It's obvious that `iterator = interator->next` **means moving to the next sibling** (according to DFS algorithm)!

## Why `execl("ptree_test", NULL)` Can't Work?

```C
else if (child == 0) //in the child process
	{
		printf("518030910188-Child is %d\n", getpid());
		if(execl("./ptree_test",  NULL) == -1){
            //!exception
            printf("calling ptree_test failed in child process!\n");
        }
		_exit(0);
	}
```

Well, this is a problem I encountered in problem 3. We need to run the function we implemented in problem 2 --- `ptree_test`. It turns out that the `ptree_test` can not work properly(giving no ouput) but the exception doesn't happen either.

### Solution

Back to the definition of `execl( )`:

```C
int execl(const char *path, const char * arg..)
````

And:

+ path is the path of program called
+ following parameters are `argv[]`
+ **`argv[0]` by default is the place saving the name of the program**

So it seems that the problem is caused by the missing program name.
But why? Why this hinders `ptree_test` from printing anything?

We can find the definition in `unistd.h`

```C
/* Execute PATH with all arguments after PATH until
   a NULL pointer and environment from `environ'.  */
extern int execl (const char *__path, const char *__arg, ...)
     __THROW __nonnull ((1, 2));
```

I think it's reasonable to defer that `__THROW __nonnull ((1, 2));` means the first two parameters can not be null.


# Problem 4 - BBC

## Can't use `pthread_cancel` in Android

### Reason

Due to safety concerns, Android bionic doesn't support `pthread_cancel`

### Solution

Using `pthread_kill` instead, but note that ` int pthread_kill(pthread_t thread, int sig);` just send the signal to the thread asynchronously. So we need a signal handler to manually kill the thread.

## Is it safe to use `pthread_mutex_lock` without `pthread_mutex_init` ?

According to an answer in StackOverflow:

https://stackoverflow.com/questions/31663981/is-it-safe-to-call-pthread-mutex-lock-before-pthread-mutex-init
It is not always safe:

+ If the `mutex_t` is set as a global variable, then it's by default set as 0, which is considered to be safe.
+ If the `mutex_t` is in the stack or heap, then it has a random value, which may lead to unwanted behaviors.

