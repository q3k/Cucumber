#ifndef __KERNEL_H__
#define __KERNEL_H__

// Macros for using standard structures in kernel code
#define free(x) kfree(x)
#define malloc(x) kmalloc(x)
#define seed(x) kseed(x)
#define rand(x) krand(x)

#endif
