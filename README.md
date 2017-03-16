# Assignment3

This file provides information on what is required for your code, as well as how to use the provided framework.

# Information provided by the student

What works:

What doesn't work:

Honor code:

# Allocator functions

Your dynamic storage allocator consists of the following three functions, which are declared in "mm.h" and defined in "mm.c":
```c
  int   mm_init(void);  
  void *mm_malloc(size_t size);  
  void  mm_free(void *ptr);  
```
The "mm.c" file implements a relatively simple, but still functional, malloc implementation. You will use this as a starting point. You will need to modify these files so that they meet the following requirements: 

* mm_init: Before calling mm_malloc or mm_free, the application program (i.e., the trace-driven driver program that you will use to evaluate your implementation) calls mm_init to perform any necessary initialization, such as allocating the initial heap area. The return value should be -1 if there was a problem in performing the initialization, 0 otherwise.

* mm_malloc: The mm_malloc function returns a pointer to an allocated block payload of at least *size* bytes, where *size* is less than 2^32. The entire allocated block should lie within the heap region and should not overlap with any other allocated block.

* mm_free: The mm_free function frees the block pointed to by ptr. It returns nothing. This routine is only guaranteed to work when the passed pointer (ptr) was returned by an earlier call to mm_malloc and has not yet been freed.

These semantics match the corresponding libc malloc and free functions.

## Beyond Correctness
Beyond correctness, your goal is to produce an allocator that performs well in time and space. The mm_malloc and mm_free functions should work as quickly as possible, and the total amount of memory used by your allocator should stay as close as possible to the amount of memory needed to hold the payload of mm_malloc calls not yet balanced by mm_free calls.

## Testing of these functions
The mm_init function will be called once per benchmark run, so it can be called multiple times in the same run of mdriver. Your mm_init function should reset your implementation to its initial state in each case.

The test driver will compare your implementation to the version of malloc supplied in the standard C library (libc). Since the libc malloc always returns payload pointers that are aligned to 16 bytes, your malloc implementation should do likewise and always return 16-byte aligned pointers.

You can use the make file to compile your code. See information below on the driver program for how to run your program.

# Support Functions
The "memlib.c" module provides a thin wrapper on the operating system’s virtual-memory system. Your allocator will need to use these functions to obtain pages of memory from which it can allocate memory. When mdriver runs your allocator, it resets the memory system (i.e., frees all pages) before calling mm_init to start a new benchmark run.

You can invoke the following functions from memlib.c:

```size_t mem_pagesize(void);```

Returns the miminum granularity of allocation via mem_map and deallocation via mem_unmap.
You can assume that the allocation granularity is a power of 2 and at least 4096.

```void *mem_map(size_t amt);```

Returns a pointer to the start of a group of newly allocated, contiguous pages that occupy exactly *amt* bytes.

The given *amt* must be a multiple of the value produced by mem_pagesize, and the returned address is always a multiple of the value produced by mem_pagesize. Separate calls to mem_map may not return pages that are contiguous to pages from previous calls.

The result is NULL if memory cannot be allocated.

```void mem_unmap(void *addr, size_t amt);```

Releases back to the operating system a group of contiguous pages that start at *addr* and covers exactly *amt* bytes.

The given *addr* must be page-aligned (i.e., it must be a multiple of the value produced by mem_pagesize), and *amt* must be a multiple of the value produced by mem_pagesize. The given *addr* must also identify pages formerly allocated with mem_map and since released by mem_unmap. A single mem_unmap can release pages allocated by multiple previous mem_map calls, and multiple different mem_unmap calls can release pages allocated by a single mem_map call—as long as each individual page’s allocation is balanced by exactly one deallocation.

```size_t mem_heapsize(void);```

Returns the total size of all currently allocated pages (i.e., mapped and not yet freed).

While your allocator will obviously need to call mem_map to obtain memory for allocation, space-efficiency for this assignment also means using mem_unmap to avoid retaining pages that are not needed as allocated blocks are freed.

# Trace-based Driver Program

The driver program "mdriver.c" tests your "mm.c" implementation for correctness, space utilization, and throughput. The driver program is controlled by a set of trace files that are included in your repository. Each trace file contains a sequence of allocate, free, and reallocate (i.e., allocate plus free) directions that instruct the driver to call your mm_malloc and mm_free functions in some sequence. The driver and the trace files are the same ones used for grading, although there may be slight modifications.

The provided makefile combines "mdriver.c" with your "mm.c" to create mdriver, which accepts the following command line arguments:

* -t ‹tracedir› — Look for the default trace files in directory ‹tracedir› instead of the default directory that is defined in config.h.

* -f ‹tracefile› — Use one particular ‹tracefile› for testing instead of the default set of tracefiles.

* -h — Print a summary of the command line arguments.

* -l — Run and measure libc malloc in addition to the "mm.c" malloc implementation.

* -v — Verbose output, printing a performance breakdown for each tracefile in a compact table.

* -V — More verbose output, printing additional diagnostic information as each trace file is processed. This flags is useful during debugging to determine which trace file is causing your malloc implementation to fail.

You may find it useful to see the shape of memory use created by a trace file. Run

  ```$ racket plot.rkt ‹tracefile›```

to see a plot of allocated memory over time for ‹tracefile›.

# Throughput: The average number of operations completed per second

The driver program summarizes the performance of your allocator by computing a performance index, P, which is a weighted sum of space utilization U, instantaneous space utilization Ui, and throughput T relative to a baseline through Tlibc:

```P = 0.3U + 0.3Ui + 0.4T/Tlibc```

The value of Tlibc is 7500K operations/second. 

Observing that both memory and CPU cycles are expensive system resources, this formula encourages balanced optimization of both memory utilization and throughput. Ideally, the performance index will reach 100%. Since each metric will contribute at most a fraction to the performance index, you should not go to extremes to optimize either the memory utilization or the throughput only. To receive a good score, you must achieve a balance between utilization and throughput. Note that the performance index overall favors space utilization over throughput.

An overall P of 0.52 is considered OK, and an overall P of 0.75 is considered excellent.

# Final Tips
* Start early! It is possible to write an efficient malloc package with a few pages of code. However, an allocator could easily be the most difficult and sophisticated code you have written so far.
* Use the mdriver -f option. During initial development, using tiny trace files will simplify debugging and testing. We have included two such trace files, "short1-bal.rep" and "short2-bal.rep", that you can use for initial debugging.
* Use the mdriver -v and -V options. The -v option will give you a detailed summary for each trace file. The -V will also indicate when each trace file is read, which will help you isolate errors.
* Use the mdriver -l options to warm up the processor and cache before running your functions.
* Compile with gcc -g and use a debugger. A debugger will help you isolate and identify out of bounds memory references.
* Understand what was discussed in class and is in the book.
* Encapsulate your pointer arithmetic in functions or C preprocessor macros. Pointer arithmetic in memory managers is confusing and error-prone because of all the casting that is necessary. You can reduce the complexity significantly by writing macros for your pointer operations. 
* If you use macros, put each use of a macro argument in parentheses within the macro definition, and always put parentheses around the right-hand side of a macro definition. Otherwise, it’s easy to write macros that parse differently than you expect when the macro is textually expanded.
* Complete your implementation in stages. Get a basic implementation working, and then modify it to improve performance
* Use a profiler. You may find the gprof tool helpful for optimizing performance.
* Keep in mind that mmap (especially) takes time, so you’ll want to make as few calls to mem_map as possible while achieving good utilization.
