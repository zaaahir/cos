#include "memory/memory.h"
#include "memory/kheap.h"

void *operator new(size_t size) { return kmalloc(size, 0); }
void *operator new[](size_t size) { return kmalloc(size, 0); }
void operator delete(void *p) { kfree(p); }
void operator delete[](void *p) { kfree(p); }
