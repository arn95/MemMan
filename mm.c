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

const int NO_FREE_SPACE = 23;
const int ALL_FREE_SPACE_FILLED = 85;
const int FREE_SPACE_REMAINING = 33;

struct free_list_s {
    struct header* head;
} FreeList;

/*
 * mm_init - initialize the malloc package.
 */
int mm_init(void)
{
    // no pages allocated yet.
    FreeList.head = NULL;
}

/*
 * mm_malloc - Allocate a block by using bytes from current_avail,
 *     grabbing a new page if necessary.
 */
void *mm_malloc(size_t size)
{

    size_t asize = ALIGN(size+HEAD_SIZE); //allocation needed
    size_t pasize = PAGE_ALIGN(asize); //page allocation given

    //for the first page
    if (FreeList.head == NULL){

        void* alloc_mem = alloc_page(pasize);

        if (alloc_mem == NULL)
            return NULL;

        struct header* space = alloc_mem;
        space->size = pasize;
        space->next = NULL;

        struct header* used_space = space;
        used_space->size = size;
        used_space->magic_number = 123456;

        struct header* free_space = space + HEAD_SIZE + size;
        free_space->size = pasize-size-HEAD_SIZE;
        free_space->next = NULL;

        FreeList.head = free_space;

        return (used_space+HEAD_SIZE);

    } else {

        //look up free space

        struct header* prev_header = NULL;
        struct header* current_header = FreeList.head;
        struct header* next_header = FreeList.head->next;

        int space_found = 0;

        while (current_header != NULL){

            int status;
            struct header* mem_free = try_occupy(current_header, size, asize, &status);

            switch(status){
                case FREE_SPACE_REMAINING : {

                    if (prev_header != NULL)
                        prev_header->next = mem_free;
                    else
                        FreeList.head = mem_free;

                    mem_free->next = next_header;

                    space_found = 1;

                } break;
                case NO_FREE_SPACE: {

                    // let the loop go to check another node for free space
                    space_found = 0;


                } break;
                case ALL_FREE_SPACE_FILLED: {

                    if (prev_header != NULL)
                        prev_header->next = next_header;
                    else
                        FreeList.head = next_header;

                    space_found = 1;

                } break;
                default: {

                }
            }

            if (space_found == 1) {
                break;
            }

            prev_header = current_header;
            current_header = current_header->next;
            if (current_header != NULL)
                next_header = current_header->next;
        }

        if (space_found == 1)
            return current_header;
        else {
            // we need to alloc a new page

            void* alloc_mem = alloc_page(pasize);

            if (alloc_mem == NULL)
                return NULL;

            struct header* space = alloc_mem;
            space->size = pasize;
            space->next = NULL;

            struct header* used_space = space;
            used_space->size = size;
            used_space->magic_number = 123456;

            struct header* free_space = space+HEAD_SIZE+size;
            free_space->size = pasize-size-HEAD_SIZE;
            free_space->next = FreeList.head;

            FreeList.head = free_space;

            return (used_space+HEAD_SIZE);
        }

    }
}

void* try_merge(void* mem1, void* mem2){

    struct header* h1 = mem1;
    struct header* h2 = mem2;

    if ((h1+h1->size) == h2){
        struct header* merged = h1;
        merged->size = (h1->size + h2->size);

        //connect the chunk with the rest of the list
        merged->next = h2->next;

        return merged;
    } else {
        return NULL;
    }
}

void* try_occupy(void* mem, size_t size, size_t asize, int* status){

    struct header* space = mem;
    size_t free_before = space->size;

    if (free_before < size){

        *status = NO_FREE_SPACE;
        return NULL; // not enough free space to occupy

    } else if (free_before == size) {

        struct header* to_use = mem;
        to_use->size = free_before;
        to_use->next = NULL;

        struct header* used_space = to_use;
        used_space->size = size;
        used_space->magic_number = 123456;

        mem = (used_space+HEAD_SIZE);

        *status = ALL_FREE_SPACE_FILLED;
        return NULL; // fits exactly so no free space left

    } else {

        struct header* to_use = mem;
        to_use->size = free_before;
        to_use->next = NULL;

        struct header* used_space = to_use;
        used_space->size = size;
        used_space->magic_number = 123456;

        mem = (used_space+HEAD_SIZE);

        struct header* free_space = used_space+HEAD_SIZE+size;
        free_space->size = free_before-size-HEAD_SIZE;

        *status = FREE_SPACE_REMAINING;
        return free_space; // free space left after filling

    }
}

int is_used(void* mem){
    struct header* mem1 = mem;
    if (mem == NULL)
        return -1;
    if (mem1->magic_number == 123456)
        return 1;
    else
        return 0;
}

void* alloc_page(size_t pasize){
    void* alloc_mem = mem_map(pasize);
    return alloc_mem;
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
//
//int main(int argc, char* args[]){
//    mem_init();
//    mm_malloc(10);
//}
