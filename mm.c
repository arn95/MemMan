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

#define LOG_APAGE_SIZE 12
#define APAGE_SIZE (1 << LOG_APAGE_SIZE)

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

const int NEED_NEW_BLOCK = 123;
const int LARGEST_BLOCK_FITS = 9866;

typedef struct header* Header;

struct free_list_s {
    Header head;
} FreeList;

// Custom function declaration

/** requests page-aligned memory **/
void* fl_alloc(size_t pasize);

/** deallocates page-aligned memory **/
void fl_dealloc();

/** Splits the block into to be used and free memory
 * returns: remaining free mem
 * pass-by-ref:
 *      status = (NO_FREE_SPACE | ALL_FREE_SPACE_FILLED | FREE_SPACE_REMAINING)
 *      points_where = a reference to where the original block pointed
 * **/
Header fl_split_block(void*, size_t , size_t, int*, void**);

/** embeds a header to the page-aligned memory **/
Header fl_new_block(size_t);

/** coalesces the blocks in the free list **/
void fl_coalesce();

/** merges two headers together if they're eligible **/
Header fl_merge_headers(Header, Header);

/** approriately connects the eligible headers together with the rest of the list **/
Header fl_connect_headers(Header, Header, Header, Header);

/** applies mergesort recursively on free list, sorting it in descending order of free space size **/
void fl_mergesort(Header*);

/** merges 2 linked list halves **/
Header fl_mergesort_merge(Header, Header);

/** splits the given linked list into 2 halves **/
void fl_mergesort_split(Header, Header*, Header*);

/*
 * mm_init - initialize the malloc package.
 */
int mm_init(void)
{
    // no pages allocated yet.
    FreeList.head = NULL; // head is also largest block
    return 0;
}

/*
 * mm_malloc - Allocate a block by using bytes from current_avail,
 *     grabbing a new page if necessary.
 */
void *mm_malloc(size_t size) {

    if (size <= 0) { // cant request negative size
        return NULL;
    }

    size_t asize = ALIGN(size+HEAD_SIZE);
    size_t pasize = PAGE_ALIGN(asize);

    if (FreeList.head == NULL) { // list is empty
        Header block = fl_new_block(pasize);

        if (block == NULL) {
            return NULL;
        }

        int status = 0;
        Header remainder = fl_split_block(block,pasize,asize,&status, NULL);
        if (status == FREE_SPACE_REMAINING) {
            FreeList.head = remainder;
            FreeList.head->next = NULL;
        } else if (status == ALL_FREE_SPACE_FILLED){
            FreeList.head = NULL;
        }

        return ((void*)block+HEAD_SIZE);

    } else { //list has at least 1 header
        int situation = 0;
        int status = 0;
        void* curr_pointer_next = NULL;
        Header block = NULL;

        if (FreeList.head->size >= asize){
            situation = LARGEST_BLOCK_FITS;
        } else {
            situation = NEED_NEW_BLOCK;
        }

        switch(situation){
            case LARGEST_BLOCK_FITS : {

                block = (void*)FreeList.head; //largest block

                Header remainder = fl_split_block(FreeList.head, FreeList.head->size, asize, &status , &curr_pointer_next);
                if (FREE_SPACE_REMAINING == status){
                    FreeList.head = remainder;
                    remainder->next = ((Header)curr_pointer_next);
                } else if (ALL_FREE_SPACE_FILLED == status){
                    FreeList.head = ((Header)curr_pointer_next);
                }

                return ((void*)block+HEAD_SIZE);

            } //break;
            case NEED_NEW_BLOCK : {

                block = fl_new_block(pasize);
                if (block == NULL) {
                    return NULL;
                }

                Header remainder = fl_split_block(block, pasize, asize, &status, NULL);
                if (status == FREE_SPACE_REMAINING){ //always inserted in the beginning of the list
                    remainder->next = FreeList.head;
                    FreeList.head = remainder;
                }

                return ((void*)block+HEAD_SIZE);
            } //break;

            default: {
                return NULL;
            }
        }
    }
}

/*
 * mm_free - Freeing a block does nothing.
 */
void mm_free(void* ptr) {
    /* You must implement this. If a page is completely free, use
     * mem_unmap() from driver/memlib.c to return it to the OS.
     */

    if (ptr == NULL) {
        return;
    }

    Header ptr_h = ptr-HEAD_SIZE;
    if (ptr_h == NULL)
        return;

    if (ptr_h->magic_number == 123456){
        ptr_h->next = FreeList.head;
        FreeList.head = ptr_h;
        fl_coalesce();
        fl_dealloc();
        fl_mergesort(&FreeList.head);
    } else {
        return;
    }
}

Header fl_merge_headers(Header h1, Header h2){

    Header merged = NULL;

    if (h1 == NULL || h2 == NULL)
        return NULL;

    if ( ((void*)h1+(h1->size+HEAD_SIZE)) == h2 ) {
        merged = h1;
        merged->size = h1->size + h2->size;
    }

    return merged;
}

Header fl_connect_headers(Header prev, Header curr, Header prev_other, Header other) {

    Header merged = NULL;
    merged = fl_merge_headers(curr, other); //merging on curr
    if (merged != NULL){
        if (prev_other == NULL){
            curr->next = other->next;
        } else {
            prev_other->next = other->next;
        }
        return merged;
    }

    merged = fl_merge_headers(other, curr); //merging on other
    if (merged != NULL){
        if (prev == NULL){
            if (prev_other == NULL){
                FreeList.head = other;
            } else {
                FreeList.head = curr->next;
            }
        } else {
            if (prev_other == NULL){
                prev->next = other;
            } else {
                prev->next = curr->next;
            }
        }
        return merged;
    }

    return NULL;
}

void fl_coalesce(){
    Header curr = FreeList.head;
    Header prev = NULL;
    Header other = NULL;
    Header prev_other = NULL;

    if (curr == NULL)
        return;

    while (curr != NULL){
        other = curr->next;
        while (other != NULL){

            fl_connect_headers(prev, curr, prev_other, other);

            prev_other = other;
            other = other->next;
        }

        prev = curr;
        curr = curr->next;
        prev_other = NULL;
    }
}

Header fl_new_block(size_t pasize){

    void* mem = fl_alloc(pasize);
    if (mem == NULL)
        return NULL;
    Header h = mem;
    h->size = pasize;
    h->next = NULL;
    return h;
}

Header fl_split_block(void* origin, size_t origin_size, size_t asize, int* status, void** points_where){

    Header remainder = NULL;
    Header origin_mod = origin;
    if (points_where != NULL) // the user might supply a null param
        *points_where = origin_mod->next;
    origin_mod->size = asize-HEAD_SIZE;
    origin_mod->magic_number = 123456;

    if (origin_size-asize > 0){
        remainder = origin + asize;
        remainder->size = origin_size-asize;
        *status = FREE_SPACE_REMAINING;
    } else if (origin_size-asize == 0)
        *status = ALL_FREE_SPACE_FILLED;
    else
        *status = NO_FREE_SPACE;

    return remainder;
}

void* fl_alloc(size_t pasize){
    void* alloc_mem = mem_map(pasize);
    return alloc_mem;
}

void fl_dealloc(){
    Header prev = NULL;
    Header curr = FreeList.head;

    if (curr == NULL)
        return;

    while (curr != NULL){

        if ( !(((uintptr_t)curr) & (APAGE_SIZE - 1)) && !( (curr->size+HEAD_SIZE) & (APAGE_SIZE - 1)) ){ // check idea borrowed from mdriver.c
            if (prev != NULL) {
                prev->next = curr->next;
            } else {
                FreeList.head = curr->next;
            }

            mem_unmap(curr, (curr->size+HEAD_SIZE));

            if (prev == NULL){
                curr = FreeList.head;
                continue;
            }
        }

        prev = curr;
        curr = curr->next;
    }
}

void fl_mergesort(Header* head) {

    Header h1;
    Header h2;

    if (*head == NULL || (*head)->next == NULL)
        return;

    fl_mergesort_split(*head, &h1, &h2);

    fl_mergesort(&h1); //recursive sort the 2 halves
    fl_mergesort(&h2);

    //merge
    *head = fl_mergesort_merge(h1, h2);
}

Header fl_mergesort_merge(Header h1, Header h2) {
    Header to_ret = NULL;

    if (h1 == NULL)
        return(h2);
    else if (h2 == NULL)
        return(h1);

    if (h1->size >= h2->size) { // set for descending order
        to_ret = h1;
        to_ret->next = fl_mergesort_merge(h1->next, h2);
    } else {
        to_ret = h2;
        to_ret->next = fl_mergesort_merge(h1, h2->next);
    }
    return(to_ret);
}

void fl_mergesort_split(Header head, Header *h1, Header *h2) {
    Header max;
    Header mid;
    if ( head==NULL || head->next==NULL ){
        *h1 = head;
        *h2 = NULL;
    }
    else {
        mid = head; // mid will be halfway when max is at last element
        max = head->next;

        while (max != NULL) {
            max = max->next;

            if (max != NULL) {
                mid = mid->next;
                max = max->next;
            }
        }

        *h1 = head;
        *h2 = mid->next;
        mid->next = NULL;
    }
}
