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

const int NO_FREE_SPACE = 33;
const int ALL_FREE_SPACE_FILLED = 22;
const int FREE_SPACE_REMAINING = 11;

struct free_list_s {
    Header head;
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

    if (size <= 0) {
        return NULL;
    }

    if (FreeList.head == NULL) {
        Header block = new_block(pasize);

        if (block == NULL) {
            return NULL;
        }

        int status = NULL;
        Header remainder = split(block, pasize,asize,&status, NULL);
        if (status == FREE_SPACE_REMAINING) {
            FreeList.head = remainder;
        }

        return ((void*)block+HEAD_SIZE);

    } else {
        void* who_points = NULL;
        Header block = find_block(&who_points,asize);

        if (block == NULL) { // No block big enough to fit
            block = new_block(pasize);
            if (block == NULL) {
                return NULL;
            }
            int status = NULL;
            void* points_where = NULL;
            Header remainder = split(block, pasize, asize, &status, &points_where);
            if (status == FREE_SPACE_REMAINING){
                remainder->next = FreeList.head;
                FreeList.head = remainder;
            } else if (status == ALL_FREE_SPACE_FILLED){
                //do nothing. free space is not added anywhere. memory is requested and used immediately. nothing remains
            }
            return  ((void*)block+HEAD_SIZE);
        } else { // Found block that can fit

            int status = NULL;
            void* points_where = NULL;
            Header remainder = split(block, block->size, asize, &status, &points_where);
            if (status == FREE_SPACE_REMAINING){
                if (who_points != NULL) {
                    ((Header)who_points)->next = remainder;
                }
                remainder->next = ((Header)points_where);
//                if (who_points == NULL && points_where == NULL)
//                    FreeList.head = remainder;
                if (who_points == NULL)
                    FreeList.head = remainder;
            } else if (status == ALL_FREE_SPACE_FILLED){
                if (who_points != NULL){
                    ((Header)who_points)->next = ((Header)points_where);
                } else {
                    FreeList.head = ((Header)points_where);
                }
            }

            return ((void*)block+HEAD_SIZE);
        }
    }
}

void list_remove_used(){
    Header prev = NULL;
    Header curr = FreeList.head;
    while (curr != NULL){

        if (curr->magic_number == 123456){
            prev->next = NULL;
            break;
        }

        prev = curr;
        curr = curr->next;
    }
}

void merge(){

    Header prev = NULL;
    Header curr = FreeList.head;

    while(curr != NULL){

        if (prev != NULL && ( (prev+prev->size) == curr)) {
            Header merged = prev;
            merged->size = prev->size + curr->size;
            merged->next = curr->next;
        }

        prev = curr;
        curr = curr->next;
    }
}

Header find_block(void** who_points, size_t asize){
    Header curr = FreeList.head;
    Header prev = NULL;

    if (FreeList.head == NULL)
        return NULL;

    while (curr->next != NULL) {
        if (curr->size >= asize)
            return curr;
        prev = curr;
        curr = curr->next;
    }
    *who_points = prev;
    if (curr->size >= asize)
        return curr;
    else
        return NULL;
}

Header new_block(size_t pasize){

    void* mem = alloc_pages(pasize);
    if (mem == NULL)
        return NULL;
    Header h = mem;
    h->size = pasize;
    h->next = NULL;
    return h;
}

Header split(void* origin, size_t origin_size, size_t asize, int* status, void** points_where){

    Header remainder = NULL;
    Header origin_mod = origin;
    if (points_where != NULL)
        *points_where = origin_mod->next;
    origin_mod->size = asize-HEAD_SIZE;
    origin_mod->magic_number = 123456;

    if (origin_size-asize > 0){
        remainder = origin + asize;
        remainder->size = origin_size-asize;
        remainder->next = NULL;
        *status = FREE_SPACE_REMAINING;
    } else if (origin_size-asize == 0)
        *status = ALL_FREE_SPACE_FILLED;
    else
        *status = NO_FREE_SPACE;

    return remainder;
}

void* alloc_pages(size_t pasize){
    void* alloc_mem = mem_map(pasize);
    return alloc_mem;
}

/*
 * mm_free - Freeing a block does nothing.
 */
void mm_free(void* ptr)
{
    /* You must implement this. If a page is completely free, use
     * mem_unmap() from driver/memlib.c to return it to the OS.
     */

//    if (ptr == NULL) {
//        return;
//    }
//
//    Header ptr_h = ptr-HEAD_SIZE;
//    if (ptr_h == NULL)
//        return;
//
//    if (ptr_h->magic_number == 123456){
//        size_t size = ptr_h->size;
//        size_t asize = ALIGN(size);
//        mem_unmap(ptr, asize);
//        merge();
//    } else {
//        return;
//    }
}
//
//int main(int argc, char* args[]){
//    mem_init();
//    mm_malloc(10);
//}
