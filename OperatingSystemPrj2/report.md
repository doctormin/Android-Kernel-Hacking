
## Adding Source File (.c) to Kernel
***Background***

When I am implementing global variable in the kernel, I add these  2 files:
```
goldfish
├── include
│   └── linux
│       └── Yimin_struct.h
└── mm
    └── Yimin_mm_limits.c

```
- `Yimin_struct.h` is used to define a struct which is the type of my global variable
- `Yimin_mm_limits.c` is used to initialize my global variable

But when I try to access my globle variable in my syscall defined in `goldfish/arch/arm/kernel/sys_arm.c` 
the error **Undefined Varible** comes out.

***Reason and Solution***

My c file (`Yimin_mm_limits.c`) is not linked with other files of the OS in after compliation stage. So even though I declare `extern struct Yimin_struct Yimin_mm_limits` in the syscall funtion. The variable can still not be found.

To solve this, all I need to do is adding the following line to the `Makefile` in `goldfish/mm` where my c file locates.
```
obj-y += Yimin_mm_limits.o
```




## About `EXPORT_SYMBOL()` and Symbol Table

### Symbol Table
***definition***

 - Kernel symbol table is nothing but a look-up table between symbol names and their addresses in memory. 
 - When a module is loaded into Kernel memory using insmod or modprobe utility, any symbol exported by the module becomes part of the Kernel symbol table. 
  
***More Information***
- This kernel symbol table is loaded into memory as part of the kernel boot process
  

### `EXPORT_SYMBOL()` & `EXPORT_SYMBOL_GPL()`

- `EXPORT_SYMBOL()`
  - Exports a given symbol to all loadable modules

- `EXPORT_SYMBOL_GPL()`
  - Exports a given symbol to only those modules that have a *GPL-compatible license*





## The Implementation of OOM-Killer
***<center>gfp.h</center>***

```C
static inline struct page *
__alloc_pages(gfp_t gfp_mask, unsigned int order,
		struct zonelist *zonelist)
{
	return __alloc_pages_nodemask(gfp_mask, order, zonelist, NULL);
}
```
***<center>page_alloc.c</center>***

```C
struct page *
__alloc_pages_nodemask(gfp_t gfp_mask, unsigned int order,
			struct zonelist *zonelist, nodemask_t *nodemask)
{
	enum zone_type high_zoneidx = gfp_zone(gfp_mask);
	struct zone *preferred_zone;
	struct page *page = NULL;
	int migratetype = allocflags_to_migratetype(gfp_mask);
	unsigned int cpuset_mems_cookie;

	gfp_mask &= gfp_allowed_mask;

	lockdep_trace_alloc(gfp_mask);

	might_sleep_if(gfp_mask & __GFP_WAIT);

	if (should_fail_alloc_page(gfp_mask, order))
		return NULL;

	/*
	 * Check the zones suitable for the gfp_mask contain at least one
	 * valid zone. It's possible to have an empty zonelist as a result
	 * of GFP_THISNODE and a memoryless node
	 */
	if (unlikely(!zonelist->_zonerefs->zone))
		return NULL;

retry_cpuset:
	cpuset_mems_cookie = get_mems_allowed();

	/* The preferred zone is used for statistics later */
	first_zones_zonelist(zonelist, high_zoneidx,
				nodemask ? : &cpuset_current_mems_allowed,
				&preferred_zone);
	if (!preferred_zone)
		goto out;

	/* First allocation attempt */
	page = get_page_from_freelist(gfp_mask|__GFP_HARDWALL, nodemask, order,
			zonelist, high_zoneidx, ALLOC_WMARK_LOW|ALLOC_CPUSET,
			preferred_zone, migratetype);
	if (unlikely(!page))
		page = __alloc_pages_slowpath(gfp_mask, order,
				zonelist, high_zoneidx, nodemask,
				preferred_zone, migratetype);

	trace_mm_page_alloc(page, order, gfp_mask, migratetype);

out:
	/*
	 * When updating a task's mems_allowed, it is possible to race with
	 * parallel threads in such a way that an allocation can fail while
	 * the mask is being updated. If a page allocation is about to fail,
	 * check if the cpuset changed during allocation and if so, retry.
	 */
	if (unlikely(!put_mems_allowed(cpuset_mems_cookie) && !page))
		goto retry_cpuset;

	return page;
}
```


# Feature

### Adding a mutex for our global varibal
In `Yimin_struct.h`

```C
#include <linux/mutex.h>

DEFINE_MUTEX(Yimin_mutex); //statically define a mutex

```

It should be noted that:
 + `DEFINE_MUTEX(name)` is a static definition
 + `mutex_init(&name)` is the dynamic initialization