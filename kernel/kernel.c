// .c
// OS Kernel Entry Point
// by Kyle Furey

#include "hlos.h"

/** Each selection. */
typedef enum selection {
    SELECTION_NONE,
    SELECTION_FILES,
    SELECTION_TEXTEDIT,
    SELECTION_SHUTDOWN,
    SELECTION_FILES_OPEN,
    SELECTION_FILES_SAVE,
    SELECTION_FILES_DELETE,
    SELECTION_FILES_FORMAT,
    SELECTION_FILES_QUIT,
} selection_t;

/** Opening sequence. */
void opening();

/** Selection sequence. */
selection_t selection();

/** Coroutine for the clock. */
void clock(void *);

/** Text-editing sequence. */
string_t textedit(string_t text);

/** The entry point of the HLOS kernel. */
void kernel_main() {
    init();
    sleep(1000);
    opening();
    clock(NULL);
    char_t text[VGA_SIZE];
    strcopy(text, "int main() {\n\treturn 0;\n}\n");
    while (true) {
        switch (selection()) {
            default:
                break;
            case SELECTION_FILES_OPEN:
                break;
            case SELECTION_FILES_FORMAT:
                break;
            case SELECTION_TEXTEDIT:
                strcopy(text, textedit(text));
                break;
            case SELECTION_SHUTDOWN:
                color(VGA_COLOR_GREEN, VGA_COLOR_BLACK);
                print("\n\nGoodbye!");
                shutdown(1000);
                break;
        }
    }
}

/** Opening sequence. */
void opening() {
    clear();
    beep();
    color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
    print("\n\t\t");
    string_t welcome = "Welcome to the\n\n";
    sleep(1000);
    if (keyboard.index != 0) return;
    while (*welcome != '\0') {
        printchar(*welcome++);
        sleep(75);
        if (keyboard.index != 0) return;
    }
    color(VGA_COLOR_BROWN, VGA_COLOR_BLACK);
    sleep(300);
    if (keyboard.index != 0) return;
    print(" High");
    sleep(300);
    if (keyboard.index != 0) return;
    print("-Level ");
    sleep(300);
    if (keyboard.index != 0) return;
    print("Operating ");
    sleep(300);
    if (keyboard.index != 0) return;
    print("System!");
    color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
    welcome = " High-Level Operating System!";
    uint_t length = strlen(welcome);
    VGA_color_t rainbow[] = {
        VGA_COLOR_LIGHT_RED,
        VGA_COLOR_LIGHT_BLUE,
        VGA_COLOR_LIGHT_CYAN,
        VGA_COLOR_LIGHT_GREEN,
        VGA_COLOR_LIGHT_MAGENTA,
        VGA_COLOR_YELLOW,
        VGA_COLOR_BROWN,
        VGA_COLOR_RED,
    };
    uint_t size = ARRAY_SIZE(rainbow);
    uint_t index = (uint_t) -1;
    sleep(750);
    if (keyboard.index != 0) return;
    print("\n\n\t\t\tYaay!");
    VGA.row -= 2;
    uint_t t = time() + 2000;
    while (time() < t) {
        for (uint_t i = 0; i < length; ++i) {
            char_t c = pos(i, VGA.row);
            color(rainbow[(index + i) % size], VGA_COLOR_BLACK);
            printchar(c);
            sleep(1);
            if (keyboard.index != 0) return;
        }
        --index;
    }
}

/** Selection sequence. */
selection_t selection() {
restart:
    while (true) {
        clear();
        color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
        print(datestr(date(), false));
        color(VGA_COLOR_BROWN, VGA_COLOR_BLACK);
        print("\n\nHLOS\n\n");
        color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
        print("1. FILES\n");
        print("2. TEXTEDIT\n");
        print("3. SHUTDOWN\n");
        print("\n> ");
        char_t *select = (char_t *) read(9, false, NULL);
        strlower(select);
        if (strcompare(select, "files") == EQUAL_TO) {
        files:
            while (true) {
                clear();
                color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
                print(datestr(date(), false));
                color(VGA_COLOR_BROWN, VGA_COLOR_BLACK);
                print("\n\nHLOS/FILES\n\n");
                color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
                print("1. OPEN\n");
                print("2. SAVE\n");
                print("3. DELETE\n");
                print("4. FORMAT\n");
                print("5. QUIT\n");
                print("\n> ");
                select = (char_t *) read(7, false, NULL);
                strlower(select);
                if (strcompare(select, "open") == EQUAL_TO) {
                    return SELECTION_FILES_OPEN;
                } else if (strcompare(select, "save") == EQUAL_TO) {
                    return SELECTION_FILES_SAVE;
                } else if (strcompare(select, "delete") == EQUAL_TO) {
                    return SELECTION_FILES_DELETE;
                } else if (strcompare(select, "format") == EQUAL_TO) {
                    return SELECTION_FILES_FORMAT;
                } else if (strcompare(select, "quit") == EQUAL_TO) {
                    goto restart;
                } else {
                    int_t index;
                    if (strint(select, &index)) {
                        switch (index + SELECTION_FILES_OPEN - 1) {
                            default:
                                break;
                            case SELECTION_FILES_OPEN:
                                return SELECTION_FILES_OPEN;
                            case SELECTION_FILES_SAVE:
                                return SELECTION_FILES_SAVE;
                            case SELECTION_FILES_DELETE:
                                return SELECTION_FILES_DELETE;
                            case SELECTION_FILES_FORMAT:
                                return SELECTION_FILES_FORMAT;
                            case SELECTION_FILES_QUIT:
                                goto restart;
                        }
                    }
                }
            }
        } else if (strcompare(select, "textedit") == EQUAL_TO) {
            return SELECTION_TEXTEDIT;
        } else if (strcompare(select, "shutdown") == EQUAL_TO) {
            return SELECTION_SHUTDOWN;
        } else {
            int_t index;
            if (strint(select, &index)) {
                switch (index) {
                    default:
                        break;
                    case SELECTION_FILES:
                        goto files;
                    case SELECTION_TEXTEDIT:
                        return SELECTION_TEXTEDIT;
                    case SELECTION_SHUTDOWN:
                        return SELECTION_SHUTDOWN;
                }
            }
        }
    }
}

/** Coroutine for the clock. */
void clock(void *) {
    coro(1000, clock, NULL);
    byte_t col = VGA.column;
    byte_t row = VGA.row;
    byte_t color = VGA.color;
    VGA.column = 0;
    VGA.row = 0;
    VGA.color = VGA_COLOR(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
    print(datestr(date(), false));
    VGA.column = col;
    VGA.row = row;
    VGA.color = color;
}

/** Text-editing sequence. */
string_t textedit(string_t text) {
    clear();
    color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
    print(datestr(date(), false));
    color(VGA_COLOR_BROWN, VGA_COLOR_BLACK);
    print("\n\nHLOS/TEXTEDIT\n");
    color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
    print("Ctrl+Enter to exit\n\n");
    uint_t row = VGA.row;
    color(VGA_COLOR_GREEN, VGA_COLOR_BLACK); // Matrix style
    while (true) {
        VGA.column = 0;
        VGA.row = row;
        text = (char_t *) read(VGA_SIZE, true, text);
        key_state_t key;
        if ((keyboard.queue[keyboard.index].flags & KEY_FLAGS_CTRL) != 0) {
            return text;
        }
    }
}
