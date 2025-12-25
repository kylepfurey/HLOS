// .h
// OS External Assembly Functions
// by Kyle Furey

#ifndef HLOS_ASSEMBLY_H
#define HLOS_ASSEMBLY_H

#include "types.h"

/** The x86 return instruction. */
#define RET 0xC3

/** Pauses the CPU. */
extern void hlt();

/** Disables external interrupts. */
extern void cli();

/** Enables external interrupts. */
extern void sti();

/** Reads a byte from the given IO port. */
extern byte_t in(ushort_t port);

/** Writes a byte to the given IO port. */
extern void out(ushort_t port, byte_t num);

/** Reads a short from the given IO port. */
extern ushort_t in2(ushort_t port);

/** Writes a short to the given IO port. */
extern void out2(ushort_t port, ushort_t num);

/** Jumps to and begins executing the given memory address until it returns. */
extern void call(const void *addr);

// INTERRUPTS

/** The callback for the timer interrupt. */
extern void timer_interrupt();

/** The callback for the keyboard interrupt. */
extern void keyboard_interrupt();

/** The callback for a page fault interrupt. */
extern void page_fault_interrupt();

// HEAP

/** Enables memory paging and virtual addressing. */
extern void enable_paging();

/** Forces the CPU to refresh a memory page. */
extern void invlpg(const void *addr);

#endif // HLOS_ASSEMBLY_H
