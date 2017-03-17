/*
 * mm-naive.c - The least memory-efficient malloc package.
 *
 * In this naive approach, a block is allocated by allocating a
 * new page as needed.  A block is pure payload. There are no headers or
 * footers.  Blocks are never coalesced or reused. The current_avail
 * pointer represents the next free space for a payload.
 *
 * NOTE TO STUDENTS: Replace this header comment with your own header
 * comment that gives a high level description of your solution.
 */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>

#include "mm.h"
#include "driver/memlib.h"

/* always use 16-byte alignment */
#define ALIGNMENT 16

/* rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(size) (((size) + (ALIGNMENT-1)) & ~(ALIGNMENT-1))

/* rounds up to the nearest multiple of mem_pagesize() */
#define PAGE_ALIGN(size) (((size) + (mem_pagesize()-1)) & ~(mem_pagesize()-1))

/* Use this structure to represent a free or used block.
 * This is not currently used by the naive solution, but you
 * should use it in yours. */
struct header {
  size_t size; /* Does not include header space */
   union {
    struct header* next; /* for free blocks */
    int magic_number; /* for used blocks */
   };
};
#define HEAD_SIZE sizeof(struct header)

/* You can have global data, but not arrays!
 * These are for the naive solution, you can replace them
 * with your own. */
void *current_avail = NULL;
int current_avail_size = 0;

/*
 * mm_init - initialize the malloc package.
 */
int mm_init(void)
{
  /* Initially there is no space available. */
  current_avail = NULL;
  current_avail_size = 0;

  return 0;
}

/*
 * mm_malloc - Allocate a block by using bytes from current_avail,
 *     grabbing a new page if necessary.
 */
void *mm_malloc(size_t size)
{
  /* The payloads must be aligned on 16 byte boundaries. */
  int newsize = ALIGN(size);
  void *p;

  /* If there isn't enough space, find out how many pages you need.
   * Use mem_map to get new pages from the heap. */
  if (current_avail_size < newsize) {
    current_avail_size = PAGE_ALIGN(newsize);
    current_avail = mem_map(current_avail_size);
    if (current_avail == NULL)
      return NULL;
  }

  /* The payload will be at the next available address, then shift the
   * available pointer forward based on the size of the allocation and
   * decrease the available space. */
  p = current_avail;
  current_avail += newsize;
  current_avail_size -= newsize;

  return p;
}

/*
 * mm_free - Freeing a block does nothing.
 */
void mm_free(void *ptr)
{
  /* You must implement this. If a page is completely free, use
   * mem_unmap() from driver/memlib.c to return it to the OS.
   */
}
