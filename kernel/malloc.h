// .h
// OS Memory Allocation Functions
// by Kyle Furey

#ifndef HLOS_MALLOC_H
#define HLOS_MALLOC_H

#include "types.h"

/** The alignment of dynamically aligned memory. */
#define MALLOC_ALIGN 4

/** The size of a memory page. */
#define PAGE_SIZE 4096

/** Builds a new Page Entry using the given memory page address and flags. */
#define NEW_PAGE_ENTRY(addr, flags) ((page_entry_t)((((uint_t)addr) & 0xFFFFF000) | (flags)))

/** Retrieves the set flags of a memory page from the given Page Entry. */
#define PAGE_ENTRY_FLAGS(entry) (((uint_t)entry) & 0xFFF)

/** Retrieves the address of a memory page from the given Page Entry. */
#define PAGE_ENTRY_ADDRESS(entry) (((uint_t)entry) & 0xFFFFF000)

/** Returns the index of a Page Directory Entry's address. */
#define PAGE_DIRECTORY_INDEX(addr) ((((uint_t)addr) >> 22) & 0x3FF)

/** Returns the index of a Page Table Entry's address. */
#define PAGE_TABLE_INDEX(addr) ((((uint_t)addr) >> 12) & 0x3FF)

/** The Interrupt Descriptor Table entry for page faults. */
#define PAGE_FAULT 14

/** Page Directory Entry flags. */
typedef enum PDE_flags {
    PDE_FLAGS_PRESENT = 1,
    PDE_FLAGS_WRITABLE = 2,
    PDE_FLAGS_USER_MODE = 4,
    PDE_FLAGS_WRITE_THROUGH = 8,
    PDE_FLAGS_CACHE_DISABLE = 16,
    PDE_FLAGS_ACCESSED = 32,
    PDE_FLAGS_4MB = 64,
    PDE_FLAGS_GLOBAL = 128,
    PDE_FLAGS_CUSTOM1 = 256,
    PDE_FLAGS_CUSTOM2 = 512,
    PDE_FLAGS_CUSTOM3 = 1024,
    PDE_FLAGS_CUSTOM4 = 2048,
    PDE_FLAGS_DEFAULT = PDE_FLAGS_PRESENT | PDE_FLAGS_WRITABLE | PDE_FLAGS_USER_MODE,
    PDE_SIZE = 1024,
} PDE_flags_t;

/** Page Table Entry flags. */
typedef enum PTE_flags {
    PTE_FLAGS_PRESENT = 1,
    PTE_FLAGS_WRITABLE = 2,
    PTE_FLAGS_USER_MODE = 4,
    PTE_FLAGS_WRITE_THROUGH = 8,
    PTE_FLAGS_CACHE_DISABLE = 16,
    PTE_FLAGS_ACCESSED = 32,
    PTE_FLAGS_DIRTY = 64,
    PTE_FLAGS_ATTRIBUTE_TABLE = 128,
    PTE_FLAGS_GLOBAL = 256,
    PTE_FLAGS_CUSTOM1 = 512,
    PTE_FLAGS_CUSTOM2 = 1024,
    PTE_FLAGS_CUSTOM3 = 2048,
    PTE_FLAGS_DEFAULT = PTE_FLAGS_PRESENT | PTE_FLAGS_WRITABLE | PTE_FLAGS_USER_MODE,
    PTE_SIZE = 1024,
} PTE_flags_t;

/** A single Page Directory Entry or Page Table Entry containing settings for a memory page and its address. */
typedef uint_t page_entry_t;

/** An array of each Page Directory Entry used to locate virtual memory pages in the Page Table.  */
typedef struct page_directory {
    /** Each page entry in the Page Directory. */
    page_entry_t entries[PDE_SIZE];
} page_directory_t;

/** An array of each Page Table Entry used to physical memory pages. */
typedef struct page_table {
    /** Each page entry in the Page Table. */
    page_entry_t entries[PTE_SIZE];
} page_table_t;

/** A linked list of all allocated memory pages. */
typedef struct page {
    /** The address of the next memory page. */
    struct page *next;
} page_t;

/** A linked list of allocated memory blocks. */
typedef struct block {
    /** The size of this memory block. */
    uint_t size;

    /** The address of the next memory block. */
    struct block *next;
} block_t;

/** The global Page Directory. */
extern volatile page_directory_t *page_directory;

/** The start of all allocated Page Tables. */
extern volatile page_table_t *page_tables;

/** The next free Page Table. */
extern page_t *free_table;

/** The next free memory page. */
extern page_t *free_page;

/** The next free memory block. */
extern block_t *free_block;

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Allocates the given number of bytes on the heap.
 * Returns a pointer to the new memory or NULL if allocation failed.
 * The allocated memory will always be at least the size of <size>.
 */
void *malloc(uint_t size);

/** Allocates and zeroes out an array of the given size and count on the heap. */
void *calloc(uint_t count, uint_t size);

/** Reallocates the given heap memory to the given size. */
void *realloc(void *mem, uint_t size);

/** Releases the given memory from the heap. */
void free(void *mem);

/** Maps the given physical memory address to the given virtual memory address with the given flags. */
void map(void *phys, void *virt, PDE_flags_t dir_flags, PTE_flags_t table_flags);

/** Unmaps the given virtual memory address. */
void unmap(void *virt);

/** Allocates a new page of memory for either a Page Table or for the heap. */
page_t *pagealloc(bool_t table);

/** Deallocates a page of memory. */
void pagefree(page_t *page);

/** Asserts on a page fault. */
void crash(uint_t err);

#ifdef __cplusplus
}
#endif

#endif // HLOS_MALLOC_H
