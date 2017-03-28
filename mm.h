#include <stdio.h>

extern int mm_init (void);
extern void *mm_malloc (size_t size);
extern void mm_free (void *ptr);

// Custom func

void* alloc_page(size_t);
void* split(void*, size_t);
void* try_merge(void*, void*);
int is_used(void*);
void* try_occupy(void*, size_t, size_t, int*);
