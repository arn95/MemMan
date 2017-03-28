#include <stdio.h>

extern int mm_init (void);
extern void *mm_malloc (size_t size);
extern void mm_free (void *ptr);

// Custom func

typedef struct header* Header;

void* alloc_pages(size_t pasize);
Header split(void*, size_t , size_t, int*, void**);
Header new_block(size_t);
Header find_block(void**, size_t);
Header merge(Header, Header);
void merge_terse();
void dealloc_pages();

