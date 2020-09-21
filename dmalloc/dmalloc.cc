#define DMALLOC_DISABLE 1
#include "dmalloc.hh"
#include <unordered_map>
#include <map>
#include <cassert>
#include <cstring>

dmalloc_stats my_stats;
static std::unordered_map<uintptr_t, size_t> my_allocs;
static std::map<uintptr_t, size_t> double_free_detect;
static std::map<uintptr_t, long> dmalloc_line;

/**
 * dmalloc(sz,file,line)
 *      malloc() wrapper. Dynamically allocate the requested amount `sz` of memory and 
 *      return a pointer to it 
 * 
 * @arg size_t sz : the amount of memory requested 
 * @arg const char *file : a string containing the filename from which dmalloc was called 
 * @arg long line : the line number from which dmalloc was called 
 * 
 * @return a pointer to the heap where the memory was reserved
 */
void* dmalloc(size_t sz, const char* file, long line) {
    (void) file, (void) line;   // avoid uninitialized variable warnings
    // Your code here.
  
    size_t sz_bound = sz;
    if (sz_bound < 0xffffffffffffffff)
        sz_bound = (sz_bound + 0x10) & 0xffffffffffffffff;

    uintptr_t *ptr = (uintptr_t *)base_malloc(sz_bound);
    if (ptr == NULL) {
        my_stats.nfail += 1;
        my_stats.fail_size = sz;
        return ptr;
    }

    if (my_stats.heap_min == 0)
        my_stats.heap_min = (uintptr_t)ptr;

    if ((uintptr_t)ptr < my_stats.heap_min)
        my_stats.heap_min = (uintptr_t)ptr;
    
    if ((uintptr_t)ptr > my_stats.heap_max)
        my_stats.heap_max = (uintptr_t)ptr + (uintptr_t)sz;

    my_stats.nactive += 1;
    my_stats.ntotal += 1; 
    my_stats.total_size += sz;
    my_stats.active_size += sz;

    uintptr_t *ptr_bound = (uintptr_t *)((uintptr_t)ptr + sz);
    *((char *)ptr_bound) = '\xCC';

    my_allocs[reinterpret_cast<uintptr_t>(ptr)] = sz;
    double_free_detect[reinterpret_cast<uintptr_t>(ptr)] = 2;
    dmalloc_line[reinterpret_cast<uintptr_t>(ptr)] = line;

    return ptr;
}

/**
 * dfree(ptr, file, line)
 *      free() wrapper. Release the block of heap memory pointed to by `ptr`. This should 
 *      be a pointer that was previously allocated on the heap. If `ptr` is a nullptr do nothing. 
 * 
 * @arg void *ptr : a pointer to the heap 
 * @arg const char *file : a string containing the filename from which dfree was called 
 * @arg long line : the line number from which dfree was called 
 */
void dfree(void* ptr, const char* file, long line) {
    (void) file, (void) line;   // avoid uninitialized variable warnings
    // Your code here.
   
    if (ptr != NULL && ((uintptr_t)ptr < my_stats.heap_min 
                || (uintptr_t)ptr > my_stats.heap_max)) {
        fprintf(stderr, "MEMORY BUG???: invalid free of pointer ???, not in heap");
        abort();
    }

    if (ptr != NULL && double_free_detect[reinterpret_cast<uintptr_t>(ptr)] == 1) {
        fprintf(stderr, "MEMORY BUG???: invalid free of pointer %p, double free", ptr);
        abort();
    }

    if (ptr != NULL && double_free_detect[reinterpret_cast<uintptr_t>(ptr)] == 0) {
        fprintf(stderr, "MEMORY BUG: %s:%lu: invalid free of pointer %p, not allocated",
                file, line, ptr);
        abort();
    }
    
    if (ptr != NULL) {
        size_t sz = my_allocs[reinterpret_cast<uintptr_t>(ptr)];
        uintptr_t *ptr_bound = (uintptr_t *)((uintptr_t)ptr + sz);
        if (*((char *)ptr_bound) != '\xCC') {
            fprintf(stderr, "MEMORY BUG???: detected wild write during free of pointer %p", ptr);
            abort();
        }

        my_stats.nactive -= 1;
        my_stats.active_size -= sz;
        base_free(ptr);
    }
    
    double_free_detect[reinterpret_cast<uintptr_t>(ptr)] = 1;
}

/**
 * dcalloc(nmemb, sz, file, line)
 *      calloc() wrapper. Dynamically allocate enough memory to store an array of `nmemb` 
 *      number of elements with wach element being `sz` bytes. The memory should be initialized 
 *      to zero  
 * 
 * @arg size_t nmemb : the number of items that space is requested for
 * @arg size_t sz : the size in bytes of the items that space is requested for
 * @arg const char *file : a string containing the filename from which dcalloc was called 
 * @arg long line : the line number from which dcalloc was called 
 * 
 * @return a pointer to the heap where the memory was reserved
 */
void* dcalloc(size_t nmemb, size_t sz, const char* file, long line) {
    // Your code here (to fix test014).

    size_t real_size = nmemb * sz;
    if (real_size < nmemb || real_size < sz) {
        my_stats.nfail += 1;
        return NULL; 
    }

    void* ptr = dmalloc(real_size, file, line);
    if (ptr) {
        memset(ptr, 0, nmemb * sz);
    }

    return ptr;
}

/**
 * get_statistics(stats)
 *      fill a dmalloc_stats pointer with the current memory statistics  
 * 
 * @arg dmalloc_stats *stats : a pointer to the the dmalloc_stats struct we want to fill
 */
void get_statistics(dmalloc_stats* stats) {
    // Stub: set all statistics to enormous numbers
    memset(stats, '\x00', sizeof(dmalloc_stats));
    // Your code here.
    stats->nactive = my_stats.nactive;
    stats->active_size = my_stats.active_size;
    stats->ntotal = my_stats.ntotal;
    stats->total_size = my_stats.total_size;
    stats->nfail = my_stats.nfail;
    stats->fail_size = my_stats.fail_size;
    stats->heap_min = my_stats.heap_min;
    stats->heap_max = my_stats.heap_max;
}

/**
 * print_statistics()
 *      print the current memory statistics to stdout       
 */
void print_statistics() {
    dmalloc_stats stats;
    get_statistics(&stats);

    printf("alloc count: active %10llu   total %10llu   fail %10llu\n",
           stats.nactive, stats.ntotal, stats.nfail);
    printf("alloc size:  active %10llu   total %10llu   fail %10llu\n",
           stats.active_size, stats.total_size, stats.fail_size);
}

/**  
 * print_leak_report()
 *      Print a report of all currently-active allocated blocks of dynamic
 *      memory.
 */
void print_leak_report() {
    // Your code here.

    for (auto it = double_free_detect.begin();
            it != double_free_detect.end(); it++) {
        if (it->second == 2) {
            size_t sz = my_allocs[(uintptr_t)it->first];
            long line = dmalloc_line[(uintptr_t)it->first];
            printf("LEAK CHECK: test???.cc:%lu: allocated object %p with size %lu\n",
                    line ,(void *)it->first, sz);
        }
    }
}
