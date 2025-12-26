/**
 * .ld
 * OS Linker Script
 * by Kyle Furey
 */

OUTPUT_FORMAT("binary")								/** Output as binary */
ENTRY(kernel_main)									/** Set kernel_main() entry point */

SECTIONS {
	/** 16-BIT STACK */
	__stack16_end = 			0x7000;				/** End of 16-bit stack memory (grows downward) */
	__stack16_start = 			0x7C00;				/** Start of 16-bit stack memory (grows downward) */

	/** BOOTLOADER */
	__boot_start = 				0x7C00;				/** Bootloader expected at 0x7C00 */
	__boot_end = 				0x7E00;				/** Bootloader is one sector wide */

	/** KERNEL */
	. = 						0x8000;				/** Load kernel at page after bootloader */
	__kernel_start = .;
	.text : { 										/** Source code */
		*(.text*)
	}
	. = 						ALIGN(4K);
	.rodata : { 									/** Read-only data */
		*(.rodata*)
	}
	. = 						ALIGN(4K);
	.data : { 										/** Initialized data */
		*(.data*)
	}
	. = 						ALIGN(4K);
	.bss (NOLOAD) : {								/** Uninitialized data */
		__bss_start = .;
		*(.bss*)
		*(COMMON)
		__bss_end = .;
	}
	__kernel_end = ALIGN(4K);

	/** 32-BIT STACK */
	__stack32_end = 			__kernel_end;		/** End of 32-bit stack memory (grows downward) */
	__stack32_start = 			0x80000;			/** Start of 32-bit stack memory (grows downward) */

	/** RAM DATA STRUCTURES */
	__IDT_start = 				0x80000;			/** Interrupt Descriptor Table */
	__IDT_end = 				0x89000;
	__PBA_start = 				0xA0000;			/** Pixel Buffer Array */
	__PBA_end = 				0xB09FF;
	__VGA_start = 				0xB8000;			/** Video Graphics Array */
	__VGA_end = 				0xB9000;

	/** PAGE DIRECTORY */
	__page_directory_start = 	0x100000;			/** Reserve page directory safely after RAM data structures */
	__page_directory_end = 		0x101000;

	/** PAGE TABLES */
	__page_tables_start = 		0x104000;			/** Reserve page tables after page directory */
	__page_tables_end = 		0x200000;

	/** KERNEL HEAP */
	__kernel_heap_start = 		0x200000;			/** Reserve kernel heap after page tables */
	__kernel_heap_end = 		0x2000000;

	/** USER HEAP */
	__heap_start = 				0x2000000;			/** Rest is user heap memory */
}
