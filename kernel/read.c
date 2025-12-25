// .c
// OS Input Reading Functions
// by Kyle Furey

#include "hlos.h"

/** A mapping of each scancode to a keyboard character. */
const char_t SCANCHAR[SCANCODE_COUNT] = {
    '\0', '\0', '1', '2', '3', '4', '5', '6',
    '7', '8', '9', '0', '-', '=', '\0', '\t',
    'q', 'w', 'e', 'r', 't', 'y', 'u', 'i',
    'o', 'p', '[', ']', '\n', '\0', 'a', 's',
    'd', 'f', 'g', 'h', 'j', 'k', 'l', ';',
    '\'', '`', '\0', '\\', 'z', 'x', 'c', 'v',
    'b', 'n', 'm', ',', '.', '/', '\0', '*',
    '\0', ' ', '\0', '\0', '\0', '\0', '\0', '\0',
    '\0', '\0', '\0', '\0', '\0', '\0', '\0', '7',
    '8', '9', '-', '4', '5', '6', '+', '1',
    '2', '3', '0', '.',
};

/** A mapping of each scancode to a shifted keyboard character. */
const char_t SCANSHIFT[SCANCODE_COUNT] = {
    '\0', '\0', '!', '@', '#', '$', '%', '^',
    '&', '*', '(', ')', '_', '+', '\0', '\t',
    'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I',
    'O', 'P', '{', '}', '\n', '\0', 'A', 'S',
    'D', 'F', 'G', 'H', 'J', 'K', 'L', ':',
    '"', '~', '\0', '|', 'Z', 'X', 'C', 'V',
    'B', 'N', 'M', '<', '>', '?', 0, '*',
    '\0', ' ', '\0', '\0', '\0', '\0', '\0', '\0',
    '\0', '\0', '\0', '\0', '\0', '\0', '\0', '7',
    '8', '9', '-', '4', '5', '6', '+', '1',
    '2', '3', '0', '.',
};

/** The state of the user's keyboard. */
volatile keyboard_t keyboard = {0};

/**
 * Reads an input string from the user, appended to <start>.
 * The maximum string length is min(<len>, MAX_INPUT_LEN).
 * The returned string is reused for all conversions.
 */
string_t read(uint_t len, bool_t ctrl_chars, string_t start) {
    static char_t buffer[MAX_INPUT_LEN] = {0};
    char_t *current = buffer;
    char_t *end = buffer;
    key_state_t key;
    char_t c;
    uint_t size = 0;
    byte_t startcol = VGA.column;
    byte_t startrow = VGA.row;
    uint_t blinktime = time() + 500;
    bool_t blinking = false;
    static bool_t insert = true;
    if (start != NULL) {
        uint_t startlen = strlen(start);
        assert(startlen < min(len, MAX_INPUT_LEN), "read() - start length > len!");
        strcopy(buffer, start);
        current = buffer + startlen;
        end = current;
        goto render;
    }
    while (true) {
        if (scan(&key, &c)) {
            if (blinking) {
                blinking = false;
                byte_t fg = VGA.color & 15;
                byte_t bg = (VGA.color >> 4) & 15;
                VGA.color = VGA_COLOR(bg, fg);
                VGA.array[VGA_POS].color = VGA.color;
            }
            blinktime = time();
            if (c != '\0') {
                if (c == '\n' && (key.flags & KEY_FLAGS_SHIFT) == 0) {
                    break;
                } else if ((!ctrl_chars && (c == '\t' || c == '\n')) ||
                           (key.flags & KEY_FLAGS_CTRL) != 0) {
                    continue;
                }
                uint_t append;
                switch (c) {
                    case '\t':
                        append = TAB_WIDTH - (VGA.column % TAB_WIDTH);
                        break;
                    case '\n':
                        append = VGA_WIDTH - VGA.column;
                        break;
                    default:
                        append = 1;
                        break;
                }
                if (insert) {
                    if (end - buffer >= min(len, MAX_INPUT_LEN) - 1 ||
                        size + append >= VGA_SIZE) {
                        continue;
                    }
                    copy(current + 1, current, end - current);
                    *current++ = c;
                    ++end;
                } else if (current != end) {
                    *current++ = c;
                } else {
                    if (end - buffer >= min(len, MAX_INPUT_LEN) - 1 ||
                        size + append >= VGA_SIZE) {
                        continue;
                    }
                    *current++ = c;
                    ++end;
                }
            } else {
                bool_t ctrl = (key.flags & KEY_FLAGS_CTRL) != 0;
                char_t first;
                byte_t targetcol;
                byte_t targetrow;
                byte_t col;
                byte_t row;
                uint_t count;
                uint_t index;
                switch (key.code) {
                    case SCANCODE_INSERT:
                        insert = !insert;
                        break;
                    case SCANCODE_BACKSPACE:
                        if (current == buffer) {
                            break;
                        }
                        first = *(current - 1);
                        do {
                            if (current == buffer) {
                                break;
                            }
                            c = *(current - 1);
                            if (ctrl && first != '\n' &&
                                (c == '\n' ||
                                 isletter(c) != isletter(first) ||
                                 isnumber(c) != isnumber(first) ||
                                 isspace(c) != isspace(first))) {
                                break;
                            }
                            --current;
                            copy(current, current + 1, end - current);
                            --end;
                        } while (ctrl &&
                                 (first != '\n' || c != '\n') &&
                                 isletter(c) == isletter(first) &&
                                 isnumber(c) == isnumber(first) &&
                                 isspace(c) == isspace(first));
                        break;
                    case SCANCODE_RIGHT:
                        first = *current;
                        do {
                            if (current == end) {
                                break;
                            }
                            c = *current;
                            if (ctrl && first != '\n' &&
                                (c == '\n' ||
                                 isletter(c) != isletter(first) ||
                                 isnumber(c) != isnumber(first) ||
                                 isspace(c) != isspace(first))) {
                                break;
                            }
                            ++current;
                        } while (ctrl &&
                                 (first != '\n' || c != '\n') &&
                                 isletter(c) == isletter(first) &&
                                 isnumber(c) == isnumber(first) &&
                                 isspace(c) == isspace(first));
                        break;
                    case SCANCODE_DELETE:
                        first = *current;
                        do {
                            if (current == end) {
                                break;
                            }
                            c = *current;
                            if (ctrl && first != '\n' &&
                                (c == '\n' ||
                                 isletter(c) != isletter(first) ||
                                 isnumber(c) != isnumber(first) ||
                                 isspace(c) != isspace(first))) {
                                break;
                            }
                            copy(current, current + 1, end - current);
                            --end;
                        } while (ctrl &&
                                 (first != '\n' || c != '\n') &&
                                 isletter(c) == isletter(first) &&
                                 isnumber(c) == isnumber(first) &&
                                 isspace(c) == isspace(first));
                        break;
                    case SCANCODE_LEFT:
                        if (current == buffer) {
                            break;
                        }
                        first = *(current - 1);
                        do {
                            if (current == buffer) {
                                break;
                            }
                            c = *(current - 1);
                            if (ctrl && first != '\n' &&
                                (c == '\n' ||
                                 isletter(c) != isletter(first) ||
                                 isnumber(c) != isnumber(first) ||
                                 isspace(c) != isspace(first))) {
                                break;
                            }
                            --current;
                        } while (ctrl &&
                                 (first != '\n' || c != '\n') &&
                                 isletter(c) == isletter(first) &&
                                 isnumber(c) == isnumber(first) &&
                                 isspace(c) == isspace(first));
                        break;
                    case SCANCODE_UP:
                        if (VGA.row == 0) {
                            current = buffer;
                            break;
                        }
                        targetcol = VGA.column;
                        targetrow = VGA.row - 1;
                        col = startcol;
                        row = startrow;
                        count = current - buffer;
                        index = 0;
                        for (uint_t i = 0; i < count; ++i) {
                            if (row == targetrow) {
                                if (col <= targetcol || ctrl) {
                                    index = i;
                                } else {
                                    break;
                                }
                            }
                            switch (buffer[i]) {
                                case '\t':
                                    col += TAB_WIDTH - (col % TAB_WIDTH);
                                    if (col >= VGA_WIDTH) {
                                        col = 0;
                                        ++row;
                                    }
                                    break;
                                case '\n':
                                    col = 0;
                                    ++row;
                                    break;
                                default:
                                    ++col;
                                    if (col >= VGA_WIDTH) {
                                        col = 0;
                                        ++row;
                                    }
                                    break;
                            }
                        }
                        current = buffer + index;
                        break;
                    case SCANCODE_DOWN:
                        if (VGA.row == VGA_HEIGHT - 1) {
                            current = end;
                            break;
                        }
                        targetcol = VGA.column;
                        targetrow = VGA.row + 1;
                        col = startcol;
                        row = startrow;
                        count = end - buffer + 1;
                        index = count;
                        for (uint_t i = 0; i < count; ++i) {
                            if (row == targetrow) {
                                if (col <= targetcol || ctrl) {
                                    index = i;
                                } else {
                                    break;
                                }
                            }
                            switch (buffer[i]) {
                                case '\t':
                                    col += TAB_WIDTH - (col % TAB_WIDTH);
                                    if (col >= VGA_WIDTH) {
                                        col = 0;
                                        ++row;
                                    }
                                    break;
                                case '\n':
                                    col = 0;
                                    ++row;
                                    break;
                                default:
                                    ++col;
                                    if (col >= VGA_WIDTH) {
                                        col = 0;
                                        ++row;
                                    }
                                    break;
                            }
                        }
                        current = buffer + min((int_t) index, (int_t) (end - buffer));
                        break;
                    case SCANCODE_PAGEUP:
                    case SCANCODE_HOME:
                        current = buffer;
                        break;
                    case SCANCODE_PAGEDOWN:
                    case SCANCODE_END:
                        current = end;
                        break;
                    default:
                        break;
                }
            }
            *end = '\0';
        render:
            uint_t begin = startcol + startrow * VGA_WIDTH;
            for (uint_t i = begin; i < min((uint_t) (begin + size), VGA_SIZE); ++i) {
                VGA.array[i].character = ' ';
            }
            VGA.column = startcol;
            VGA.row = startrow;
            print(buffer);
            if (begin > VGA_POS) {
                clear();
                startcol = 0;
                startrow = 0;
                print(buffer);
            }
            size = min(VGA_POS - (startcol + startrow * VGA_WIDTH), VGA_SIZE);
            VGA.column = startcol;
            VGA.row = startrow;
            uint_t count = current - buffer;
            for (uint_t i = 0; i < count; ++i) {
                switch (buffer[i]) {
                    case '\t':
                        VGA.column += TAB_WIDTH - (VGA.column % TAB_WIDTH);
                        if (VGA.column >= VGA_WIDTH) {
                            VGA.column = 0;
                            ++VGA.row;
                        }
                        break;
                    case '\n':
                        VGA.column = 0;
                        ++VGA.row;
                        break;
                    default:
                        ++VGA.column;
                        if (VGA.column >= VGA_WIDTH) {
                            VGA.column = 0;
                            ++VGA.row;
                        }
                        break;
                }
            }
        } else if (time() > blinktime) {
            byte_t fg = VGA.color & 15;
            byte_t bg = (VGA.color >> 4) & 15;
            VGA.color = VGA_COLOR(bg, fg);
            VGA.array[VGA_POS].color = VGA.color;
            blinking = !blinking;
            blinktime = time() + 500;
        }
    }
    *end = '\0';
    return buffer;
}

/** Reads a single character from the user. */
char_t readchar() {
    char_t c;
    while (true) {
        if (scan(NULL, &c) && c != '\0') {
            break;
        }
    }
    printchar(c);
    return c;
}

/**
 * Reads an integer from the user.
 * If the user inputted a valid number, <num> is set to it.
 * Returns whether the read was successful.
 */
bool_t readint(int_t *num) {
    return strint(read(12, false, NULL), num);
}

/**
 * Checks for keyboard input. Returns whether any new input has occurred.
 * <key> will be set to the latest keyboard input.
 * <c> will be set to the keyboard's corresponding character (or \0 if none).
 */
bool_t scan(key_state_t *key, char_t *c) {
    ushort_t index = keyboard.index;
    ushort_t age = keyboard.queue[keyboard.index].age;
    hlt();
    cli();
    volatile key_state_t *state = &keyboard.queue[keyboard.index];
    if (index == keyboard.index && age == keyboard.queue[index].age) {
        sti();
        return false;
    }
    if ((state->flags & KEY_FLAGS_PRESSED) == 0) {
        sti();
        return false;
    }
    if (key != NULL) {
        *key = *state;
    }
    if (c != NULL) {
        if (state->code < SCANCODE_REMAPPED) {
            *c = SCANCHAR[state->code];
            if (isletter(*c)) {
                if (((state->flags & KEY_FLAGS_SHIFT) != 0) ^
                    ((state->flags & KEY_FLAGS_CAPSLOCK) != 0)) {
                    *c = SCANSHIFT[state->code];
                }
            } else if ((state->flags & KEY_FLAGS_SHIFT) != 0) {
                *c = SCANSHIFT[state->code];
            }
        } else {
            *c = '\0';
        }
    }
    sti();
    return true;
}

/** Updates the state of the keyboard. */
void key() {
    static bool_t extended = false;
    scancode_t code = (scancode_t) in(KEYBOARD_DATA_PORT);
    if (code == SCANCODE_NONE) {
        return;
    }
    if (code == SCANCODE_EXTENDED) {
        extended = true;
        return;
    } else if (code == SCANCODE_EXTENDED2) {
        extended = false;
        return;
    }
    bool_t pressed = (code & SCANCODE_RELEASED) == 0;
    code &= ~SCANCODE_RELEASED;
    volatile key_state_t *state = &keyboard.queue[keyboard.index];
    byte_t flags = state->flags;
    if (extended) {
        extended = false;
        if ((flags & KEY_FLAGS_NUMLOCK) == 0) {
            switch (code) {
                case SCANCODE_LEFT_CTRL:
                    code = SCANCODE_RIGHT_CTRL;
                    break;
                case SCANCODE_LEFT_ALT:
                    code = SCANCODE_RIGHT_ALT;
                    break;
                case SCANCODE_KEYPAD_8:
                    code = SCANCODE_UP;
                    break;
                case SCANCODE_KEYPAD_2:
                    code = SCANCODE_DOWN;
                    break;
                case SCANCODE_KEYPAD_4:
                    code = SCANCODE_LEFT;
                    break;
                case SCANCODE_KEYPAD_6:
                    code = SCANCODE_RIGHT;
                    break;
                case SCANCODE_KEYPAD_7:
                    code = SCANCODE_HOME;
                    break;
                case SCANCODE_KEYPAD_1:
                    code = SCANCODE_END;
                    break;
                case SCANCODE_KEYPAD_9:
                    code = SCANCODE_PAGEUP;
                    break;
                case SCANCODE_KEYPAD_3:
                    code = SCANCODE_PAGEDOWN;
                    break;
                case SCANCODE_KEYPAD_0:
                    code = SCANCODE_INSERT;
                    break;
                case SCANCODE_KEYPAD_DOT:
                    code = SCANCODE_DELETE;
                    break;
                case SCANCODE_ENTER:
                    code = SCANCODE_KEYPAD_ENTER;
                    break;
                default:
                    break;
            }
        }
    }
    if (pressed) {
        switch (code) {
            case SCANCODE_LEFT_SHIFT:
            case SCANCODE_RIGHT_SHIFT:
                flags |= KEY_FLAGS_SHIFT;
                break;
            case SCANCODE_LEFT_CTRL:
            case SCANCODE_RIGHT_CTRL:
                flags |= KEY_FLAGS_CTRL;
                break;
            case SCANCODE_LEFT_ALT:
            case SCANCODE_RIGHT_ALT:
                flags |= KEY_FLAGS_ALT;
                break;
            case SCANCODE_LEFT_SUPER:
            case SCANCODE_RIGHT_SUPER:
                flags |= KEY_FLAGS_SUPER;
                break;
            case SCANCODE_CAPSLOCK:
                flags ^= KEY_FLAGS_CAPSLOCK;
                break;
            case SCANCODE_NUMLOCK:
                flags ^= KEY_FLAGS_NUMLOCK;
                break;
            case SCANCODE_SCROLLLOCK:
                flags ^= KEY_FLAGS_SCROLLLOCK;
                break;
            default:
                break;
        }
    } else {
        switch (code) {
            case SCANCODE_LEFT_SHIFT:
            case SCANCODE_RIGHT_SHIFT:
                flags &= ~KEY_FLAGS_SHIFT;
                break;
            case SCANCODE_LEFT_CTRL:
            case SCANCODE_RIGHT_CTRL:
                flags &= ~KEY_FLAGS_CTRL;
                break;
            case SCANCODE_LEFT_ALT:
            case SCANCODE_RIGHT_ALT:
                flags &= ~KEY_FLAGS_ALT;
                break;
            case SCANCODE_LEFT_SUPER:
            case SCANCODE_RIGHT_SUPER:
                flags &= ~KEY_FLAGS_SUPER;
                break;
            default:
                break;
        }
    }
    keyboard.index = (keyboard.index + 1) % MAX_KEY_STATES;
    state = &keyboard.queue[keyboard.index];
    state->code = code;
    if (pressed) {
        state->flags = flags | KEY_FLAGS_PRESSED;
    } else {
        state->flags = flags & ~KEY_FLAGS_PRESSED;
    }
    ++state->age;
}
