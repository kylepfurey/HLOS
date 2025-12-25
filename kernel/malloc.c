// .c
// OS Memory Allocation Functions
// by Kyle Furey

#include "hlos.h"

/** The global Page Directory. */
volatile page_directory_t *page_directory = (volatile page_directory_t *) &__page_directory_start;

/** The start of the global Page Tables. */
volatile page_table_t *page_tables = (volatile page_table_t *) &__page_tables_start;

/** The next free memory page. */
page_t *free_page = NULL;

/** The next free memory block. */
block_t *free_block = NULL;

/**
 * Allocates the given number of bytes on the heap.
 * Returns a pointer to the new memory or NULL if allocation failed.
 * The allocated memory will always be at least the size of <size>.
 */
void *malloc(uint_t size) {
    assert(size != 0, "malloc() - size was 0!");
    size = (size + (MALLOC_ALIGN - 1)) & ~(MALLOC_ALIGN - 1); // Round up to 4-byte alignment
    while (true) {
        block_t *prev = NULL;
        block_t *curr = free_block;
        while (curr != NULL) {
            if (curr->size >= size) {
                if (curr->size >= size + sizeof(block_t) + MALLOC_ALIGN) {
                    block_t *new_block = (block_t *) ((byte_t *) curr + sizeof(block_t) + size);
                    new_block->size = curr->size - size - sizeof(block_t);
                    new_block->next = curr->next;
                    curr->size = size;
                    if (prev) {
                        prev->next = new_block;
                    } else {
                        free_block = new_block;
                    }
                } else {
                    if (prev != NULL) {
                        prev->next = curr->next;
                    } else {
                        free_block = curr->next;
                    }
                }
                return ((byte_t *) curr) + sizeof(block_t);
            }
            prev = curr;
            curr = curr->next;
        }
        page_t *page = pagealloc();
        if (page == NULL) {
            return NULL;
        }
        map(
            page,
            page,
            PDE_FLAGS_DEFAULT,
            PTE_FLAGS_DEFAULT
        );
        block_t *new_block = (block_t *) page;
        new_block->size = PAGE_SIZE - sizeof(block_t);
        new_block->next = free_block;
        free_block = new_block;
    }
}

/** Allocates and zeroes out an array of the given size and count on the heap. */
void *calloc(uint_t count, uint_t size) {
    uint_t total = count * size;
    if (count != 0 && total / count != size) {
        return NULL; // Overflow detected
    }
    byte_t *alloc = (byte_t *) malloc(total);
    if (alloc == NULL) {
        return NULL; // malloc() can safely fail
    }
    set(alloc, 0, total);
    return alloc;
}

/** Reallocates the given heap memory to the given size. */
void *realloc(void *mem, uint_t size) {
    if (mem == NULL) {
        return malloc(size);
    }
    if (size == 0) {
        free(mem);
        return NULL;
    }
    size = (size + (MALLOC_ALIGN - 1)) & ~(MALLOC_ALIGN - 1); // Round up to 4-byte alignment
    block_t *block = (block_t *) ((byte_t *) mem - sizeof(block_t));
    uint_t old_size = block->size;
    if (block->size >= size) {
        if (block->size >= size + sizeof(block_t) + MALLOC_ALIGN) {
            block_t *new_block = (block_t *) ((byte_t *) block + sizeof(block_t) + size);
            new_block->size = block->size - size - sizeof(block_t);
            new_block->next = free_block;
            free_block = new_block;
            block->size = size;
        }
        return mem;
    }
    void *alloc = malloc(size);
    if (alloc == NULL) {
        return NULL;
    }
    copy(alloc, mem, min((int_t) old_size, (int_t) size));
    free(mem);
    return alloc;
}

/** Releases the given memory from the heap. */
void free(void *mem) {
    if (mem == NULL) {
        return; // free(NULL) is safe
    }
    block_t *block = (block_t *) (((byte_t *) mem) - sizeof(block_t));
    block->next = free_block;
    free_block = block;
}

/** Maps the given physical memory address to the given virtual memory address with the given flags. */
void map(void *phys, void *virt, PDE_flags_t dir_flags, PTE_flags_t table_flags) {
    assert(phys != NULL, "map() - phys was NULL!");
    assert(virt != NULL, "map() - virt was NULL!");
    assert(((uint_t) phys & (PAGE_SIZE - 1)) == 0, "map() - phys was not aligned to PAGE_SIZE!");
    assert(((uint_t) virt & (PAGE_SIZE - 1)) == 0, "map() - virt was not aligned to PAGE_SIZE!");
    uint_t directory_index = PAGE_DIRECTORY_INDEX(virt);
    uint_t table_index = PAGE_TABLE_INDEX(virt);
    page_table_t *table;
    if ((page_directory->entries[directory_index] & PDE_FLAGS_PRESENT) == 0) {
        table = (page_table_t *) pagealloc();
        assert(table != NULL, "map() - Could not allocate a memory page!");
        set(table, 0, PAGE_SIZE);
        page_directory->entries[directory_index] = NEW_PAGE_ENTRY(table, dir_flags);
    } else {
        table = (page_table_t *) PAGE_ENTRY_ADDRESS(page_directory->entries[directory_index]);
    }
    table->entries[table_index] = NEW_PAGE_ENTRY(phys, table_flags);
    invlpg(virt);
}

/** Unmaps the given virtual memory address. */
void unmap(void *virt) {
    assert(virt != NULL, "unmap() - virt was NULL!");
    uint_t directory_index = PAGE_DIRECTORY_INDEX(virt);
    uint_t table_index = PAGE_TABLE_INDEX(virt);
    assert(
        (page_directory->entries[directory_index] & PDE_FLAGS_PRESENT) != 0,
        "unmap() - Attempting to free an invalid page!"
    );
    page_table_t *table = (page_table_t *) PAGE_ENTRY_ADDRESS(page_directory->entries[directory_index]);
    assert(
        (table->entries[table_index] & PTE_FLAGS_PRESENT) != 0,
        "unmap() - Attempting to free an invalid page!"
    );
    if ((table->entries[table_index] & PTE_FLAGS_PRESENT) != 0) {
        pagefree((page_t *) PAGE_ENTRY_ADDRESS(table->entries[table_index]));
    }
    table->entries[table_index] = 0;
    invlpg(virt);
    for (uint_t i = 0; i < PTE_SIZE; ++i) {
        if ((table->entries[i] & PTE_FLAGS_PRESENT) != 0) {
            return;
        }
    }
    pagefree((page_t *) PAGE_ENTRY_ADDRESS(page_directory->entries[directory_index]));
    page_directory->entries[directory_index] = 0;
}

/** Allocates a new page of memory. */
page_t *pagealloc() {
    if (free_page == NULL) {
        return NULL;
    }
    page_t *page = free_page;
    free_page = page->next;
    set(page, 0, PAGE_SIZE);
    return page;
}

/** Deallocates a page of memory. */
void pagefree(page_t *page) {
    assert(page != NULL, "pagefree() - page was NULL!");
    assert(((uint_t) page & (PAGE_SIZE - 1)) == 0, "pagefree() - page was not aligned to PAGE_SIZE!");
    page->next = free_page;
    free_page = page;
}

/** Asserts on a page fault. */
void crash(uint_t err) {
    panic("Dereferencing invalid virtual address!");
}
