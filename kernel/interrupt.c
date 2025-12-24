// .c
// OS Interrupt Callback Functions
// by Kyle Furey

#include "hlos.h"

/** The Interrupt Descriptor Table. */
IDT_t IDT = {
    (volatile IDT_entry_t * const) &__IDT_start,
};

/** Enables hardware with the given Interrupt Request mask. */
void IRQ_enable(IRQ_mask_t mask) {
    byte_t master = in(PIC_MASTER_DATA_PORT);
    byte_t slave = in(PIC_SLAVE_DATA_PORT);
    master &= ~(mask & 255);
    slave &= ~((mask >> 8) & 255);
    out(PIC_MASTER_DATA_PORT, master);
    out(PIC_SLAVE_DATA_PORT, slave);
}

/** Disables hardware with the given Interrupt Request mask. */
void IRQ_disable(IRQ_mask_t mask) {
    byte_t master = in(PIC_MASTER_DATA_PORT);
    byte_t slave = in(PIC_SLAVE_DATA_PORT);
    master |= (mask & 255);
    slave |= ((mask >> 8) & 255);
    out(PIC_MASTER_DATA_PORT, master);
    out(PIC_SLAVE_DATA_PORT, slave);
}

/** Sets and returns an entry in the Interrupt Descriptor Table. */
volatile IDT_entry_t *IDT_bind(PIC_entry_t index, void *callback, ushort_t selector, IDT_flags_t flags) {
    volatile IDT_entry_t *entry = &IDT.entries[index % IDT_COUNT];
    cli();
    entry->address_low = (ushort_t) ((uint_t) callback & 0xFFFF);
    entry->selector = selector;
    entry->reserved_byte = 0;
    entry->flags = (byte_t) flags;
    entry->address_high = (ushort_t) (((uint_t) callback >> 16) & 0xFFFF);
    sti();
    return entry;
}
