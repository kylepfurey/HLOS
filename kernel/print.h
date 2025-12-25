// .h
// OS Printing Functions
// by Kyle Furey

#ifndef HLOS_PRINT_H
#define HLOS_PRINT_H

#include "types.h"

/** Combines a foreground and background color for the Video Graphics Array. */
#define VGA_COLOR(fg, bg) (((bg) << 4) | ((fg) & 15))

/** Returns the current index of the VGA. */
#define VGA_POS (VGA.column + (VGA.row * VGA_WIDTH))

/** The number of spaces that equal one \t tab. */
#define TAB_WIDTH 4

/** Video Graphics Array sizes. */
typedef enum VGA_size {
    VGA_WIDTH = 80,
    VGA_HEIGHT = 25,
    VGA_SIZE = VGA_WIDTH * VGA_HEIGHT,
} VGA_size_t;

/** Video Graphics Array colors. */
typedef enum VGA_color {
    VGA_COLOR_BLACK = 0x0,
    VGA_COLOR_BLUE = 0x1,
    VGA_COLOR_GREEN = 0x2,
    VGA_COLOR_CYAN = 0x3,
    VGA_COLOR_RED = 0x4,
    VGA_COLOR_MAGENTA = 0x5,
    VGA_COLOR_BROWN = 0x6,
    VGA_COLOR_LIGHT_GRAY = 0x7,
    VGA_COLOR_DARK_GRAY = 0x8,
    VGA_COLOR_LIGHT_BLUE = 0x9,
    VGA_COLOR_LIGHT_GREEN = 0xA,
    VGA_COLOR_LIGHT_CYAN = 0xB,
    VGA_COLOR_LIGHT_RED = 0xC,
    VGA_COLOR_LIGHT_MAGENTA = 0xD,
    VGA_COLOR_YELLOW = 0xE,
    VGA_COLOR_WHITE = 0xF,
} VGA_color_t;

/** Represents a single character in the Video Graphics Array. */
typedef struct VGA_char {
    /** The value of this character. */
    char_t character;

    /**
     * The foreground and background color of this character.
     * The low 4 bits represent the foreground color.
     * The high 4 bits represent the background color.
     */
    byte_t color;
} VGA_char_t;

/** Data for the state of the Video Graphics Array. */
typedef struct VGA {
    /** The address of the Video Graphics Array. */
    volatile VGA_char_t *const array;

    /** The current column of the Video Graphics Array. */
    byte_t column;

    /** The current row of the Video Graphics Array. */
    byte_t row;

    /** The current color of the Video Graphics Array. */
    byte_t color;
} VGA_t;

/** The Video Graphics Array. */
extern VGA_t VGA;

/** Prints a message to the Video Graphics Array. */
void print(string_t msg);

/** Prints a character to the Video Graphics Array. */
void printchar(char_t c);

/** Prints an integer to the Video Graphics Array. */
void printint(int_t num);

/** Clears the Video Graphics Array. */
void clear();

/** Sets the current color of the Video Graphics Array. */
void color(VGA_color_t fg, VGA_color_t bg);

/**
 * Sets the current column and row of the Video Graphics Array.
 * Returns the character at that position.
 */
char_t pos(byte_t col, byte_t row);

#endif // HLOS_PRINT_H
