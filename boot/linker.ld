/**
 * .ld
 * OS Linker Script
 * by Kyle Furey
 */

OUTPUT_FORMAT("binary")					/** Output as binary */
ENTRY(kernel_main)						/** Set kernel_main() entry point */

SECTIONS {
	__stack_end = 0x7000;				/** End of stack memory (grows downward) */
	__stack_start = 0x7C00;				/** Start of stack memory (grows downward) */
	__boot_start = 0x7C00;				/** Bootloader expected at 0x7C00 */
	__boot_end = . + 512;
	. = 0x8000;							/** Load kernel immediately after bootloader */
	__kernel_start = .;
	.text : { *(.text) }				/** Source code */
	.rodata : { *(.rodata) }			/** Read-only data */
	.data : { *(.data) }				/** Initialized data */
	.bss : {							/** Uninitialized data */
		__bss_start = .;
		*(.bss)
		*(COMMON)
		__bss_end = .;
	}
	__kernel_end = .;
	.page_directory 0x10000 : {			/** Reserve page directory after kernel */
		__page_directory_start = .;
		. = . + 0x1000;					/** Reserve 4 kilobytes */
		__page_directory_end = .;
	}
	.page_table 0x11000 : {				/** Reserve page table after page directory */
		__page_table_start = .;
		. = . + 0x1000;					/** Reserve 4 kilobytes */
		__page_table_end = .;
	}
	. = 0x12000;						/** Reserve kernel heap after page table */
	__kernel_heap_start = .;
	.heap (NOLOAD) : {
		. = . + 16M;					/** Reserve 16 megabytes for the kernel heap */
	}
	__kernel_heap_end = .;
	__heap_start = .;					/** Rest is user heap memory */
}
