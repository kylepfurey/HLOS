// .c
// OS Library Functions
// by Kyle Furey

#include "hlos.h"

/** Halts the kernel. */
void pause() {
    while (true) {
        hlt();
    }
}

/** Shuts down the kernel after the given number of milliseconds. */
void shutdown(uint_t ms) {
    sleep(ms);
    while ((in(KEYBOARD_STATUS_PORT) & KEYBOARD_STATUS_BUSY) != 0) {
        hlt();
    }
    cli();
    clear();
    out2(ACPI_PORT, ACPI_SHUTDOWN);
    pause();
}

/** Reboots the kernel after the given number of milliseconds. */
void reboot(uint_t ms) {
    sleep(ms);
    while ((in(KEYBOARD_STATUS_PORT) & KEYBOARD_STATUS_BUSY) != 0) {
        hlt();
    }
    cli();
    clear();
    out(KEYBOARD_STATUS_PORT, KEYBOARD_REBOOT);
    pause();
}

/** Crashes the kernel with an error if the given condition is false. */
void assert(bool_t cond, string_t err) {
    if (!cond) {
        cli();
        for (uint_t i = 0; i < MAX_COROUTINES; ++i) {
            coroutines[i].callback = NULL;
        }
        pos(0, 0);
        color(VGA_COLOR_WHITE, VGA_COLOR_RED);
        print("ASSERTION FAILED: ");
        if (err != NULL) {
            print(err);
        }
        printchar('\n');
        pause();
    }
}

/** Enables stepping through the kernel via keyboard input. */
void step(string_t msg) {
    // Freeze VGA in place
    cli();
    byte_t col = VGA.column;
    byte_t row = VGA.row;
    byte_t old_color = VGA.color;
    VGA_char_t array[VGA_SIZE];
    for (uint_t i = 0; i < VGA_SIZE; ++i) {
        array[i] = VGA.array[i];
    }
    // Print message
    pos(0, 0);
    color(VGA_COLOR_BLACK, VGA_COLOR_YELLOW);
    print("BREAKPOINT: ");
    if (msg != NULL) {
        print(msg);
    }
    printchar('\n');
    // Restore VGA
    VGA.column = col;
    VGA.row = row;
    VGA.color = old_color;
    // Await input
    sti();
    char_t c = '\0';
    do {
        scan(NULL, &c);
    } while (c == '\0');
    // Restore VGA
    cli();
    for (uint_t i = 0; i < VGA_SIZE; ++i) {
        VGA.array[i] = array[i];
    }
    // Continue
    sti();
}

/** Allows raw memory to be easily inspected as a string. */
string_t dump(uint_t size, void *addr) {
    assert(addr != NULL, "dump() - addr was NULL!");
    size = min((int_t) size, VGA_SIZE);
    static char_t buffer[VGA_SIZE];
    if (size <= sizeof(int_t)) {
        int_t *num = (int_t *) addr;
        strcopy(buffer, intstr(*num));
        return buffer;
    }
    copy(buffer, (char_t *) addr, size);
    for (uint_t i = 0; i < size; ++i) {
        if (buffer[i] == '\0') {
            buffer[i] = ' ';
        }
    }
    buffer[size] = '\0';
    return buffer;
}

/** Returns the lesser number. */
int_t min(int_t a, int_t b) {
    return a < b ? a : b;
}

/** Returns the greater number. */
int_t max(int_t a, int_t b) {
    return a > b ? a : b;
}

/** Returns a positive version of the given number. */
int_t abs(int_t num) {
    if (num >= 0) {
        return num;
    }
    return -num;
}

/** Generates a unique hash code from the given data. */
uint_t hash(const void *data, uint_t size) {
    assert(data != NULL, "hash() - data was NULL!");
    // FNV-1a
    byte_t *ptr = (byte_t *) data;
    uint_t hash = 2166136261u; // FNV offset
    for (uint_t i = 0; i < size; ++i) {
        hash ^= ptr[i];
        hash *= 16777619u; // FNV prime
    }
    return hash;
}

/** Copies <size> number of bytes from <src> into <dest> without overlap. Returns <dest>. */
void *copy(void *dest, const void *src, uint_t size) {
    assert(dest != NULL, "copy() - dest was NULL!");
    assert(src != NULL, "copy() - src was NULL!");
    byte_t *destbytes = (byte_t *) dest;
    const byte_t *srcbytes = (const byte_t *) src;
    if (destbytes == srcbytes || size == 0) {
        return dest;
    }
    if (destbytes < srcbytes || destbytes >= srcbytes + size) {
        for (uint_t i = 0; i < size; ++i) {
            destbytes[i] = srcbytes[i];
        }
    } else {
        for (uint_t i = size; i > 0; --i) {
            destbytes[i - 1] = srcbytes[i - 1];
        }
    }
    return dest;
}

/**
 * Compares <left> and <right> byte by byte for <size>.
 * Returns a comparison enum of <left> to <right> for the first differing byte.
 */
comparison_t compare(const void *left, const void *right, uint_t size) {
    assert(left != NULL, "compare() - left was NULL!");
    assert(right != NULL, "compare() - right was NULL!");
    const byte_t *leftbytes = (const byte_t *) left;
    const byte_t *rightbytes = (const byte_t *) right;
    for (uint_t i = 0; i < size; ++i) {
        if (leftbytes[i] < rightbytes[i]) {
            return LESS_THAN;
        }
        if (leftbytes[i] > rightbytes[i]) {
            return GREATER_THAN;
        }
    }
    return EQUAL_TO;
}

/** Sets each byte of <dest> to <num> for <size>. Returns <dest>. */
void *set(void *dest, byte_t num, uint_t size) {
    assert(dest != NULL, "set() - dest was NULL!");
    byte_t *bytes = (byte_t *) dest;
    while (size-- > 0) {
        bytes[size] = num;
    }
    return dest;
}

/** Swaps each byte of <left> and <right> for <size>. Returns <left>. */
void *swap(void *left, void *right, uint_t size) {
    assert(left != NULL, "swap() - left was NULL!");
    assert(right != NULL, "swap() - right was NULL!");
    byte_t *leftbytes = (byte_t *) left;
    byte_t *rightbytes = (byte_t *) right;
    while (size-- > 0) {
        byte_t temp = leftbytes[size];
        leftbytes[size] = rightbytes[size];
        rightbytes[size] = temp;
    }
    return left;
}

/** Reverses <size> number of bytes in <src>. Returns <src>. */
void *flip(void *src, uint_t size) {
    assert(src != NULL, "flip() - src was NULL!");
    byte_t *bytes = (byte_t *) src;
    uint_t count = size / 2;
    for (uint_t i = 0; i < count; ++i) {
        byte_t temp = bytes[i];
        bytes[i] = bytes[size - 1 - i];
        bytes[size - 1 - i] = temp;
    }
    return src;
}

/** Finds the first matching byte equal to <num> for <size> and returns its index (or NOT_FOUND). */
uint_t first(const void *src, byte_t num, uint_t size) {
    assert(src != NULL, "first() - src was NULL!");
    const byte_t *bytes = (const byte_t *) src;
    for (uint_t i = 0; i < size; ++i) {
        if (bytes[i] == num) {
            return i;
        }
    }
    return NOT_FOUND;
}

/** Finds the last matching byte equal to <num> for <size> and returns its index (or NOT_FOUND). */
uint_t last(const void *src, byte_t num, uint_t size) {
    assert(src != NULL, "last() - src was NULL!");
    const byte_t *bytes = (const byte_t *) src;
    uint_t i = size;
    while (i-- > 0) {
        if (bytes[i] == num) {
            return i;
        }
    }
    return NOT_FOUND;
}

/**
 * Returns the index of the first byte where the value of <src> of size <size2>
 * is located within <dest> within <size1> bytes (or NOT_FOUND).
 */
uint_t find(const void *dest, uint_t size1, const void *src, uint_t size2) {
    assert(dest != NULL, "find() - dest was NULL!");
    assert(src != NULL, "find() - src was NULL!");
    if (size2 == 0 || size2 > size1) {
        return NOT_FOUND;
    }
    const byte_t *destbytes = (const byte_t *) dest;
    const byte_t *srcbytes = (const byte_t *) src;
    for (uint_t i = 0; i <= size1 - size2; ++i) {
        uint_t j = 0;
        while (j < size2 && destbytes[i + j] == srcbytes[j]) {
            ++j;
        }
        if (j == size2) {
            return i;
        }
    }
    return NOT_FOUND;
}

/** Copies <src> of size <size2> right after <dest> at <size1>. Returns <dest>. */
void *append(void *dest, uint_t size1, const void *src, uint_t size2) {
    assert(dest != NULL, "append() - dest was NULL!");
    assert(src != NULL, "append() - src was NULL!");
    return copy((byte_t *) dest + size1, src, size2);
}
