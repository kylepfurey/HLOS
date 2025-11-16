// .h
// OS Input Reading Functions
// by Kyle Furey

#ifndef HLOS_READ_H
#define HLOS_READ_H

#include "print.h"

/** Keyboard data port. */
#define KEYBOARD_DATA_PORT 0x60

/** Keyboard status port. */
#define KEYBOARD_STATUS_PORT 0x64

/** Indicates the keyboard is busy. */
#define KEYBOARD_STATUS_BUSY 0x2

/** Reboots the computer when sent to the KEYBOARD_STATUS_PORT. */
#define KEYBOARD_REBOOT 0xFE

/** Advanced Configuration and Power Interface port. */
#define ACPI_PORT 0x604

/** Shuts down the computer when sent to the ACPI_PORT. */
#define ACPI_SHUTDOWN 0x2000

/** The maximum length of a received input string (including the null-terminator). */
#define MAX_INPUT_LEN VGA_SIZE

/** The number of key states in the circular keyboard queue. */
#define MAX_KEY_STATES 16

/** Each numerical scancode value. */
typedef enum scancode {
    SCANCODE_NONE = 0x0,
    SCANCODE_ESCAPE = 0x1,
    SCANCODE_1 = 0x2,
    SCANCODE_2 = 0x3,
    SCANCODE_3 = 0x4,
    SCANCODE_4 = 0x5,
    SCANCODE_5 = 0x6,
    SCANCODE_6 = 0x7,
    SCANCODE_7 = 0x8,
    SCANCODE_8 = 0x9,
    SCANCODE_9 = 0xA,
    SCANCODE_0 = 0xB,
    SCANCODE_MINUS = 0xC,
    SCANCODE_EQUALS = 0xD,
    SCANCODE_BACKSPACE = 0xE,
    SCANCODE_TAB = 0xF,
    SCANCODE_Q = 0x10,
    SCANCODE_W = 0x11,
    SCANCODE_E = 0x12,
    SCANCODE_R = 0x13,
    SCANCODE_T = 0x14,
    SCANCODE_Y = 0x15,
    SCANCODE_U = 0x16,
    SCANCODE_I = 0x17,
    SCANCODE_O = 0x18,
    SCANCODE_P = 0x19,
    SCANCODE_LEFT_BRACKET = 0x1A,
    SCANCODE_RIGHT_BRACKET = 0x1B,
    SCANCODE_ENTER = 0x1C,
    SCANCODE_LEFT_CTRL = 0x1D,
    SCANCODE_A = 0x1E,
    SCANCODE_S = 0x1F,
    SCANCODE_D = 0x20,
    SCANCODE_F = 0x21,
    SCANCODE_G = 0x22,
    SCANCODE_H = 0x23,
    SCANCODE_J = 0x24,
    SCANCODE_K = 0x25,
    SCANCODE_L = 0x26,
    SCANCODE_SEMICOLON = 0x27,
    SCANCODE_APOSTROPHE = 0x28,
    SCANCODE_GRAVE = 0x29,
    SCANCODE_LEFT_SHIFT = 0x2A,
    SCANCODE_BACKSLASH = 0x2B,
    SCANCODE_Z = 0x2C,
    SCANCODE_X = 0x2D,
    SCANCODE_C = 0x2E,
    SCANCODE_V = 0x2F,
    SCANCODE_B = 0x30,
    SCANCODE_N = 0x31,
    SCANCODE_M = 0x32,
    SCANCODE_COMMA = 0x33,
    SCANCODE_DOT = 0x34,
    SCANCODE_SLASH = 0x35,
    SCANCODE_RIGHT_SHIFT = 0x36,
    SCANCODE_KEYPAD_MULTIPLY = 0x37,
    SCANCODE_LEFT_ALT = 0x38,
    SCANCODE_SPACE = 0x39,
    SCANCODE_CAPSLOCK = 0x3A,
    SCANCODE_F1 = 0x3B,
    SCANCODE_F2 = 0x3C,
    SCANCODE_F3 = 0x3D,
    SCANCODE_F4 = 0x3E,
    SCANCODE_F5 = 0x3F,
    SCANCODE_F6 = 0x40,
    SCANCODE_F7 = 0x41,
    SCANCODE_F8 = 0x42,
    SCANCODE_F9 = 0x43,
    SCANCODE_F10 = 0x44,
    SCANCODE_NUMLOCK = 0x45,
    SCANCODE_SCROLLLOCK = 0x46,
    SCANCODE_KEYPAD_7 = 0x47,
    SCANCODE_KEYPAD_8 = 0x48,
    SCANCODE_KEYPAD_9 = 0x49,
    SCANCODE_KEYPAD_MINUS = 0x4A,
    SCANCODE_KEYPAD_4 = 0x4B,
    SCANCODE_KEYPAD_5 = 0x4C,
    SCANCODE_KEYPAD_6 = 0x4D,
    SCANCODE_KEYPAD_PLUS = 0x4E,
    SCANCODE_KEYPAD_1 = 0x4F,
    SCANCODE_KEYPAD_2 = 0x50,
    SCANCODE_KEYPAD_3 = 0x51,
    SCANCODE_KEYPAD_0 = 0x52,
    SCANCODE_KEYPAD_DOT = 0x53,
    SCANCODE_F11 = 0x57,
    SCANCODE_F12 = 0x58,
    SCANCODE_LEFT_SUPER = 0x5B,
    SCANCODE_RIGHT_SUPER = 0x5C,
    SCANCODE_MENU = 0x5D,
    SCANCODE_RIGHT_CTRL = 0x81,
    SCANCODE_RIGHT_ALT = 0x82,
    SCANCODE_UP = 0x85,
    SCANCODE_DOWN = 0x86,
    SCANCODE_LEFT = 0x87,
    SCANCODE_RIGHT = 0x88,
    SCANCODE_HOME = 0x89,
    SCANCODE_END = 0x8A,
    SCANCODE_PAGEUP = 0x8B,
    SCANCODE_PAGEDOWN = 0x8C,
    SCANCODE_INSERT = 0x8D,
    SCANCODE_DELETE = 0x8E,
    SCANCODE_KEYPAD_ENTER = 0x8F,
    SCANCODE_RELEASED = 0x80,
    SCANCODE_REMAPPED = 0x81,
    SCANCODE_EXTENDED = 0xE0,
    SCANCODE_EXTENDED2 = 0xE1,
    SCANCODE_COUNT = 128,
} scancode_t;

/** Keyboard scancode flags. */
typedef enum key_flags {
    KEY_FLAGS_PRESSED = 1,
    KEY_FLAGS_SHIFT = 2,
    KEY_FLAGS_CTRL = 4,
    KEY_FLAGS_ALT = 8,
    KEY_FLAGS_SUPER = 16,
    KEY_FLAGS_CAPSLOCK = 32,
    KEY_FLAGS_NUMLOCK = 64,
    KEY_FLAGS_SCROLLLOCK = 128,
} key_flags_t;

/** The state of a single key press. */
typedef struct key_state {
    /** The scancode value of this key state. */
    byte_t code;

    /** Flags used to define this key state. */
    byte_t flags;

    /** An accumulated number used to compare the ages of different key states. */
    ushort_t age;
} key_state_t;

/** The state of the user's keyboard. */
typedef struct keyboard {
    /** The index of the most recent key state in the queue. */
    ushort_t index;

    /** A circular queue of recent key states. */
    key_state_t queue[MAX_KEY_STATES];
} keyboard_t;

/** A mapping of each scancode to a keyboard character. */
extern const char_t SCANCHAR[SCANCODE_COUNT];

/** A mapping of each scancode to a shifted keyboard character. */
extern const char_t SCANSHIFT[SCANCODE_COUNT];

/** The current state of the user's keyboard. */
extern volatile keyboard_t keyboard;

/**
 * Reads an input string from the user, appended to <start>.
 * The maximum string length is min(<len>, MAX_INPUT_LEN).
 * The returned string is reused for all conversions.
 */
string_t read(uint_t len, bool_t ctrl_chars, string_t start);

/** Reads a single character from the user. */
char_t readchar();

/**
 * Reads an integer from the user.
 * If the user inputted a valid number, <num> is set to it.
 * Returns whether the read was successful.
 */
bool_t readint(int_t *num);

/**
 * Checks for keyboard input. Returns whether any new input has occurred.
 * <key> will be set to the latest keyboard input.
 * <c> will be set to the keyboard's corresponding character (or \0 if none).
 */
bool_t scan(key_state_t *key, char_t *c);

/** Updates the state of the keyboard. */
void key();

#endif // HLOS_READ_H
