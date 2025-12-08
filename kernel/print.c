// .c
// OS Printing Functions
// by Kyle Furey

#include "print.h"
#include "lib.h"
#include "malloc.h"

/** The Video Graphics Array. */
VGA_t VGA = {
    (volatile VGA_char_t * const) 0xB8000,
    0,
    0,
    VGA_COLOR(VGA_COLOR_LIGHT_GRAY, VGA_COLOR_BLACK),
};

/** Prints a message to the Video Graphics Array. */
void print(string_t msg) {
    assert(msg != NULL, "print() - msg was NULL!");
    while (*msg != '\0') {
        if (*msg == '\n') {
            VGA.column = 0;
            ++VGA.row;
        } else if (*msg == '\r') {
            VGA.column = 0;
        } else if (*msg == '\t') {
            byte_t tab = TAB_WIDTH - (VGA.column % TAB_WIDTH);
            for (uint_t i = 0; i < tab; ++i) {
                printchar(' ');
            }
        } else {
            uint_t i = VGA.row * VGA_WIDTH + VGA.column;
            VGA.array[i].character = *msg;
            VGA.array[i].color = VGA.color;
            ++VGA.column;
            if (VGA.column >= VGA_WIDTH) {
                VGA.column = 0;
                ++VGA.row;
            }
        }
        ++msg;
        if (VGA.row >= VGA_HEIGHT) {
            clear();
        }
    }
}

/** Prints a character to the Video Graphics Array. */
void printchar(char_t c) {
    static char_t buffer[2] = {0};
    *buffer = c;
    print(buffer);
}

/** Prints an integer to the Video Graphics Array. */
void printint(int_t num) {
    print(intstr(num));
}

/** Clears the Video Graphics Array. */
void clear() {
    for (uint_t i = 0; i < VGA_SIZE; ++i) {
        VGA.array[i].character = ' ';
        VGA.array[i].color = VGA.color;
    }
    VGA.column = 0;
    VGA.row = 0;
}

/** Sets the current color of the Video Graphics Array. */
void color(VGA_color_t fg, VGA_color_t bg) {
    VGA.color = VGA_COLOR(fg, bg);
}

/**
 * Sets the current column and row of the Video Graphics Array.
 * Returns the character at that position.
 */
char_t pos(byte_t col, byte_t row) {
    VGA.column = col % VGA_WIDTH;
    VGA.row = row % VGA_HEIGHT;
    return VGA.array[VGA.row * VGA_WIDTH + VGA.column].character;
}
