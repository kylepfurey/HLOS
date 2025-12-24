// .h
// OS Initialization Function
// by Kyle Furey

#ifndef HLOS_INIT_H
#define HLOS_INIT_H

#include "types.h"

/** The last byte of the 16-bit stack. */
extern byte_t __stack16_end;

/** The byte after the start of the 16-bit stack. */
extern byte_t __stack16_start;

/** The first byte of the bootloader. */
extern byte_t __boot_start;

/** The byte after the end of the bootloader. */
extern byte_t __boot_end;

/** The first byte of the kernel. */
extern byte_t __kernel_start;

/** The first byte of uninitialized data. */
extern byte_t __bss_start;

/** The byte after the end of uninitialized data. */
extern byte_t __bss_end;

/** The byte after the end of the kernel. */
extern byte_t __kernel_end;

/** The last byte of the 32-bit stack. */
extern byte_t __stack32_end;

/** The byte after the start of the 32-bit stack. */
extern byte_t __stack32_start;

/** The first byte of the Interrupt Descriptor Table. */
extern byte_t __IDT_start;

/** The byte after the end of the Interrupt Descriptor Table. */
extern byte_t __IDT_end;

/** The first byte of the Video Graphics Array. */
extern byte_t __VGA_start;

/** The byte after the end of the Video Graphics Array. */
extern byte_t __VGA_end;

/** The first byte of the memory directory table. */
extern byte_t __page_directory_start;

/** The byte after the end of the memory directory table. */
extern byte_t __page_directory_end;

/** The first byte of the memory page tables. */
extern byte_t __page_tables_start;

/** The byte after the end of the memory page tables. */
extern byte_t __page_tables_end;

/** The first byte of the kernel heap memory. */
extern byte_t __kernel_heap_start;

/** The byte after the end of the kernel heap memory. */
extern byte_t __kernel_heap_end;

/** The first byte of the user heap memory. */
extern byte_t __heap_start;

/** Initializes the kernel. */
void init();

#endif // HLOS_INIT_H
