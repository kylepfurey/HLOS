// .c
// OS Initialization Function
// by Kyle Furey

#include "hlos.h"

/** Initializes the Pixel Buffer Array color palette. */
static void PBA_init() {
#if PIXEL_RENDERING
    for (uint_t i = 0; i < 256; ++i) {
        byte_t red = ((i >> 5) & 0x7) * 8;
        byte_t green = ((i >> 2) & 0x7) * 8;
        byte_t blue = ((i & 3) << 1) * 8;
        out(PBA_INDEX_PORT, (byte_t) i);
        out(PBA_COLOR_PORT, red);
        out(PBA_COLOR_PORT, green);
        out(PBA_COLOR_PORT, blue);
    }
    fill(COLOR(51, 77, 77));
    render();
#endif
}

/** Initializes the Programmable Interval Timer. */
static void PIT_init() {
    out(PIT_CMD_PORT, PIT_CMD_NUM);
    uint_t divisor = PIT_DIVISOR(PIT_SPEED_HZ);
    out(PIT_CHANNEL0_PORT, (byte_t) (divisor & 255));
    out(PIT_CHANNEL0_PORT, (byte_t) ((divisor >> 8) & 255));
    IDT_bind(PIC_TIMER, timer_interrupt, IDT_KERNEL_SELECTOR, IDT_KERNEL_FLAGS);
    IRQ_enable(IRQ_TIMER);
}

/** Initializes the keyboard. */
static void keyboard_init() {
    IDT_bind(PIC_KEYBOARD, keyboard_interrupt, IDT_KERNEL_SELECTOR, IDT_KERNEL_FLAGS);
}

/** Initializes memory paging, virtual addressing, and the heap. */
static void heap_init() {
    // Clear page directory
    set(&__page_directory_start, 0, &__page_directory_end - &__page_directory_start);
    // Initialize free page tables
    free_table = (page_t *) &__page_tables_start;
    page_t *current = free_table;
    page_t *next = (page_t *) (((byte_t *) current) + PAGE_SIZE);
    while (next < (page_t *) &__page_tables_end) {
        current->next = next;
        current = next;
        next = (page_t *) (((byte_t *) current) + PAGE_SIZE);
    }
    current->next = NULL;
    // Map all kernel virtual addresses
    for (byte_t *page = (byte_t *) 0; page < &__kernel_heap_end; page += PAGE_SIZE) {
        map(
            page,
            page,
            PDE_FLAGS_PRESENT | PDE_FLAGS_WRITABLE,
            PTE_FLAGS_PRESENT | PTE_FLAGS_WRITABLE
        );
    }
    // Initialize heap pages and allocator
    free_block = (block_t *) &__kernel_heap_start;
    free_block->size = (uint_t) (&__kernel_heap_end - &__kernel_heap_start - sizeof(block_t));
    free_block->next = NULL;
    free_page = NULL; // TODO: Initialize user heap
    // Enable paging
    IDT_bind(PAGE_FAULT, page_fault_interrupt, IDT_KERNEL_SELECTOR, IDT_KERNEL_FLAGS);
    enable_paging();
}

/** Mounts FAT32. */
static void FAT32_init() {
    clear();
    color(VGA_COLOR_YELLOW, VGA_COLOR_BLACK);
    print("Mounting drive . . .");
    if (!mount(ATA_PRIMARY_PORT, ATA_MASTER_DRIVE, FAT32_START)) {
        clear();
        color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK);
        print("Formatting empty drive to FAT32 . . .");
        color(VGA_COLOR_GREEN, VGA_COLOR_BLACK);
        assert(
            format(
                ATA_PRIMARY_PORT,
                ATA_MASTER_DRIVE,
                FAT32_START,
                1,
                FAT32_SIZE,
                false
            ),
            "FAT32_init() - Formatting failed!"
        );
        mount(ATA_PRIMARY_PORT, ATA_MASTER_DRIVE, FAT32_START);
        string_t welcome = "Welcome to HLOS!\n";
        assert(
            fileappend("hlos.txt", strlen(welcome), welcome),
            "FAT32_init() - Could not create test file!"
        );
        print(" Formatting successful!\n\n");
        color(VGA_COLOR_YELLOW, VGA_COLOR_BLACK);
        print("Rebooting . . .");
        reboot(1000);
    }
    color(VGA_COLOR_LIGHT_GRAY, VGA_COLOR_BLACK);
}

/** Initializes the kernel. */
void init() {
    set(&__bss_start, 0, &__bss_end - &__bss_start);
    PBA_init();
    PIT_init();
    keyboard_init();
    heap_init();
    FAT32_init();
}
