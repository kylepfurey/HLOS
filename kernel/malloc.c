// .c
// OS Memory Allocation Functions
// by Kyle Furey

#include "malloc.h"
#include "lib.h"
#include "assembly.h"

/** The global Page Directory. */
volatile page_directory_t *page_directory = (volatile page_directory_t *) &page_directory;

/** The global Page Table. */
volatile page_table_t *page_table = (volatile page_table_t *) &page_table;

/**
 * Maps the given physical memory address to the given virtual memory address with the given flags.
 * Returns whether memory was successfully mapped.
 */
bool_t map(void *phys, void *virt, PTE_flags_t flags) {
    assert(phys != NULL, "map() - phys was NULL!");
    assert(virt != NULL, "map() - virt was NULL!");
    assert(((uint_t) phys & (PAGE_SIZE - 1)) == 0, "map() - phys was not aligned to PAGE_SIZE!");
    assert(((uint_t) virt & (PAGE_SIZE - 1)) == 0, "map() - virt was not aligned to PAGE_SIZE!");
    // TODO
    return false;
}

/** Unmaps the given virtual memory address. */
void unmap(void *virt) {
    assert(virt != NULL, "unmap() - virt was NULL!");
    // TODO
}

/**
 * Allocates the given number of bytes on the heap.
 * Returns a pointer to the new memory or NULL if allocation failed.
 * The allocated memory will always be at least the size of <size>.
 */
void *malloc(uint_t size) {
    size = (size + 3) / 4 * 4; // Round up to 4-byte alignment
    // TODO
    return NULL;
}

/** Allocates and zeroes out an array of the given size and count on the heap. */
void *calloc(uint_t count, uint_t size) {
    uint_t total = count * size;
    byte_t *mem = (byte_t *) malloc(total);
    if (mem == NULL) {
        return NULL; // malloc can safely fail
    }
    for (uint_t i = 0; i < total; ++i) {
        mem[i] = 0;
    }
    return mem;
}

/** Reallocates the given heap memory to the given size. */
void *realloc(void *mem, uint_t size) {
    if (mem == NULL) {
        return NULL; // realloc can safely fail
    }
    size = (size + 3) / 4 * 4; // Round up to 4-byte alignment
    // TODO
    return NULL;
}

/** Releases the given memory from the heap. */
void free(void *mem) {
    if (mem == NULL) {
        return; // free(NULL) is safe
    }
    // TODO
}
