// .c
// OS Initialization Function
// by Kyle Furey

#include "init.h"
#include "lib.h"
#include "assembly.h"
#include "interrupt.h"
#include "time.h"
#include "malloc.h"

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
    set(&__page_directory_start, 0, &__page_table_end - &__page_directory_start);
    // TODO: Let the kernel RUN again!
    //enable_paging();
}

/** Initializes the kernel. */
void init() {
    set(&__bss_start, 0, &__bss_end - &__bss_start);
    PIT_init();
    keyboard_init();
    heap_init();
}
