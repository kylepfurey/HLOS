// .h
// OS Interrupt Callback Functions
// by Kyle Furey

#ifndef HLOS_INTERRUPT_H
#define HLOS_INTERRUPT_H

#include "types.h"

/** Master Programmable Interrupt Controller command port. */
#define PIC_MASTER_CMD_PORT 0x20

/** Master Programmable Interrupt Controller data port. */
#define PIC_MASTER_DATA_PORT 0x21

/** Slave Programmable Interrupt Controller command port. */
#define PIC_SLAVE_CMD_PORT 0xA0

/** Slave Programmable Interrupt Controller data port. */
#define PIC_SLAVE_DATA_PORT 0xA1

/** The Interrupt Descriptor Table selector used for interrupts in kernel mode. */
#define IDT_KERNEL_SELECTOR 0x8

/** The Interrupt Descriptor Table flags used for interrupts in kernel mode. */
#define IDT_KERNEL_FLAGS (IDT_FLAGS_PRESENT | IDT_FLAGS_DPL0 | IDT_FLAGS_32BIT_GATE)

/** Programmable Interrupt Controller entry. */
typedef enum PIC_entry {
    // Master PIC entries
    PIC_TIMER = 32,
    PIC_KEYBOARD = 33,
    PIC_CASCADE = 34,
    PIC_COM2 = 35,
    PIC_COM1 = 36,
    PIC_LPT2 = 37,
    PIC_FLOPPY = 38,
    PIC_LPT1 = 39,

    // Slave PIC entries
    PIC_RTC = 40,
    PIC_FREE9 = 41,
    PIC_FREE10 = 42,
    PIC_FREE11 = 43,
    PIC_MOUSE = 44,
    PIC_FPU = 45,
    PIC_PRIMARY_IDE = 46,
    PIC_SECONDARY_IDE = 47,
} PIC_entry_t;

/** Interrupt Request masks. */
typedef enum IRQ_mask {
    // Master PIC IRQs
    IRQ_TIMER = 1,
    IRQ_KEYBOARD = 2,
    IRQ_CASCADE = 4,
    IRQ_COM2 = 8,
    IRQ_COM1 = 16,
    IRQ_LPT2 = 32,
    IRQ_FLOPPY = 64,
    IRQ_LPT1 = 128,

    // Slave PIC IRQs
    IRQ_RTC = 256,
    IRQ_FREE9 = 512,
    IRQ_FREE10 = 1024,
    IRQ_FREE11 = 2048,
    IRQ_MOUSE = 4096,
    IRQ_FPU = 8192,
    IRQ_PRIMARY_IDE = 16384,
    IRQ_SECONDARY_IDE = 32768,
    IRQ_ALL = ((ushort_t) -1),
} IRQ_mask_t;

/** Interrupt Descriptor Table size. */
typedef enum IDT_size {
    IDT_COUNT = 256,
    IDT_ENTRY_SIZE = 8,
    IDT_SIZE = IDT_COUNT * IDT_ENTRY_SIZE,
} IDT_size_t;

/** Interrupt Descriptor Table flags. */
typedef enum IDT_flags {
    IDT_FLAGS_PRESENT = 128,
    IDT_FLAGS_DPL0 = 0,
    IDT_FLAGS_DPL1 = 32,
    IDT_FLAGS_DPL2 = 64,
    IDT_FLAGS_DPL3 = 96,
    IDT_FLAGS_32BIT_GATE = 14,
    IDT_FLAGS_16BIT_GATE = 6,
    IDT_FLAGS_TRAP_GATE = 15,
    IDT_FLAGS_ALL = 255,
} IDT_flags_t;

/** Represents a single entry in the Interrupt Descriptor Table. */
typedef struct IDT_entry {
    /** The low offset of the callback address. */
    ushort_t address_low;

    /** The code segment selector (from GDT). */
    ushort_t selector;

    /** The reserved byte (always zero). */
    byte_t reserved_byte;

    /** Flags for this interrupt. */
    byte_t flags;

    /** The high offset of the callback address. */
    ushort_t address_high;
} IDT_entry_t;

/** Data for the state of the Interrupt Descriptor Table. */
typedef struct IDT {
    /** The address of the Interrupt Descriptor Table. */
    volatile IDT_entry_t *const entries;
} IDT_t;

/** The Interrupt Descriptor Table. */
extern IDT_t IDT;

#ifdef __cplusplus
extern "C" {
#endif

/** Enables hardware with the given Interrupt Request mask. */
void IRQ_enable(IRQ_mask_t mask);

/** Disables hardware with the given Interrupt Request mask. */
void IRQ_disable(IRQ_mask_t mask);

/** Sets and returns an entry in the Interrupt Descriptor Table. */
volatile IDT_entry_t *IDT_bind(PIC_entry_t index, void *callback, ushort_t selector, IDT_flags_t flags);

#ifdef __cplusplus
}
#endif

#endif // HLOS_INTERRUPT_H
