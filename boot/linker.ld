/**
 * .ld
 * OS Linker Script
 * by Kyle Furey
 */

OUTPUT_FORMAT("binary")					/** Output as binary */
ENTRY(kernel_main)						/** Set kernel_main() entry point */

SECTIONS {
	__boot_start = 0x7C00;				/** Bootloader expected at 0x7C00 */
	__boot_end = 0x7C00 + 512;
	. = 0x1000;							/** Load kernel immediately after bootloader */
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
	.page_table 0x11000 : {				/** Reserve page table after page directory*/
		__page_table_start = .;
		. = . + 0x1000;					/** Reserve 4 kilobytes */
		__page_table_end = .;
	}
	__heap_start = 0x100000;			/** Rest is heap memory */
}
