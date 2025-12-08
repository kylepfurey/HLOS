// .h
// OS Initialization Function
// by Kyle Furey

#ifndef HLOS_INIT_H
#define HLOS_INIT_H

#include "types.h"

/** The first byte of the bootloader. */
extern byte_t __boot_start;

/** The byte after the end of the bootloader. */
extern byte_t __boot_end;

/** The first byte of the kernel. */
extern byte_t __kernel_start;

/** The byte after the end of the kernel. */
extern byte_t __kernel_end;

/** The first byte of uninitialized data. */
extern byte_t __bss_start;

/** The byte after the end of uninitialized data. */
extern byte_t __bss_end;

/** The first byte of the memory directory table. */
extern byte_t __page_directory_start;

/** The byte after the end of the memory directory table. */
extern byte_t __page_directory_end;

/** The first byte of the memory page table. */
extern byte_t __page_table_start;

/** The byte after the end of the memory page table. */
extern byte_t __page_table_end;

/** The first byte of the kernel heap memory. */
extern byte_t __kernel_heap_start;

/** The byte after the end of the kernel heap memory. */
extern byte_t __kernel_heap_end;

/** The first byte of the user heap memory. */
extern byte_t __heap_start;

/** Initializes the kernel. */
void init();

#endif // HLOS_INIT_H
