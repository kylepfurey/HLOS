// .c
// OS String Functions
// by Kyle Furey

#include "hlos.h"

/** Returns whether the given character is a letter. */
bool_t isletter(char_t c) {
    return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z');
}

/** Returns whether the given character is a number. */
bool_t isnumber(char_t c) {
    return (c >= '0' && c <= '9');
}

/** Returns whether the given character is whitespace. */
bool_t isspace(char_t c) {
    return (c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == '\v' || c == '\f');
}

/** Returns whether the given character is an uppercase letter. */
bool_t isupper(char_t c) {
    return (c >= 'A' && c <= 'Z');
}

/** Returns whether the given character is a lowercase letter. */
bool_t islower(char_t c) {
    return (c >= 'a' && c <= 'z');
}

/** Returns the uppercase version of the given character. */
char_t upper(char_t c) {
    if (islower(c)) {
        return c - ('a' - 'A');
    }
    return c;
}

/** Returns the lowercase version of the given character. */
char_t lower(char_t c) {
    if (isupper(c)) {
        return c + ('a' - 'A');
    }
    return c;
}

/** Returns the length of a string. */
uint_t strlen(string_t str) {
    assert(str != NULL, "strlen() - str was NULL!");
    uint_t len = 0;
    while (str[len] != '\0') {
        ++len;
    }
    return len;
}

/** Returns the length of the smaller string. */
uint_t strmin(string_t left, string_t right) {
    assert(left != NULL, "strmin() - str was NULL!");
    assert(right != NULL, "strmin() - right was NULL!");
    uint_t len = 0;
    while (left[len] != '\0' && right[len] != '\0') {
        ++len;
    }
    return len;
}

/** Generates a unique hash code from the given string. */
uint_t strhash(string_t str) {
    assert(str != NULL, "strhash() - str was NULL!");
    // FNV-1a
    uint_t hash = 2166136261; // FNV offset
    for (uint_t i = 0; str[i] != '\0'; ++i) {
        hash ^= (byte_t) str[i];
        hash *= 16777619; // FNV prime
    }
    return hash;
}

/** Safely copies <src> into <dest> with proper null-termination. Returns <dest>. */
char_t *strcopy(char_t *dest, string_t src) {
    assert(dest != NULL, "strcopy() - dest was NULL!");
    assert(src != NULL, "strcopy() - src was NULL!");
    char_t *returned = dest;
    if (dest < src) {
        while (*src != '\0') {
            *dest++ = *src++;
        }
        *dest = '\0';
    } else if (dest > src) {
        const char_t *end = src;
        while (*end != '\0') {
            ++end;
        }
        char_t *d = dest + (end - src);
        *d-- = '\0';
        while (end != src) {
            --end;
            *d-- = *end;
        }
    }
    return returned;
}

/**
 * Compares <left> and <right> character by character.
 * Returns a comparison enum of <left> to <right> for the first differing character.
 */
comparison_t strcompare(string_t left, string_t right) {
    assert(left != NULL, "strcompare() - left was NULL!");
    assert(right != NULL, "strcompare() - right was NULL!");
    uint_t i = 0;
    while (left[i] != '\0' && right[i] != '\0') {
        if (left[i] < right[i]) {
            return LESS_THAN;
        }
        if (left[i] > right[i]) {
            return GREATER_THAN;
        }
        ++i;
    }
    if (left[i] == '\0' && right[i] != '\0') {
        return LESS_THAN;
    }
    if (left[i] != '\0' && right[i] == '\0') {
        return GREATER_THAN;
    }
    return EQUAL_TO;
}

/** Safely sets each character in <str> to <c>. Returns <str>. */
char_t *strset(char_t *str, char_t c) {
    assert(str != NULL, "strset() - str was NULL!");
    for (uint_t i = 0; str[i] != '\0'; ++i) {
        str[i] = c;
    }
    return str;
}

/** Safely swaps the characters of <left> and <right> with proper null-termination. Returns <left>. */
char_t *strswap(char_t *left, char_t *right) {
    assert(left != NULL, "strswap() - left was NULL!");
    assert(right != NULL, "strswap() - right was NULL!");
    uint_t len = strmin(left, right);
    while (len-- > 0) {
        char_t temp = left[len];
        left[len] = right[len];
        right[len] = temp;
    }
    return left;
}

/** Safely reverses each character in <str>. Returns <str>. */
char_t *strflip(char_t *str) {
    assert(str != NULL, "strflip() - str was NULL!");
    uint_t len = strlen(str);
    uint_t count = len / 2;
    for (uint_t i = 0; i < count; ++i) {
        byte_t temp = str[i];
        str[i] = str[len - 1 - i];
        str[len - 1 - i] = temp;
    }
    return str;
}

/** Returns the index of the first character that matches <c> in <str> (or NOT_FOUND). */
uint_t strfirst(string_t str, char_t c) {
    assert(str != NULL, "strfirst() - str was NULL!");
    for (uint_t i = 0; str[i] != '\0'; ++i) {
        if (str[i] == c) {
            return i;
        }
    }
    return NOT_FOUND;
}

/** Returns the index of to the last character that matches <c> in <str> (or NOT_FOUND). */
uint_t strlast(string_t str, char_t c) {
    assert(str != NULL, "strlast() - str was NULL!");
    uint_t last = NOT_FOUND;
    for (uint_t i = 0; str[i] != '\0'; ++i) {
        if (str[i] == c) {
            last = i;
        }
    }
    return last;
}

/** Returns the first index of where <src> was found in <dest> (or NOT_FOUND). */
uint_t strfind(string_t dest, string_t src) {
    assert(dest != NULL, "strfind() - dest was NULL!");
    assert(src != NULL, "strfind() - src was NULL!");
    if (*src == '\0') {
        return NOT_FOUND;
    }
    for (uint_t i = 0; dest[i] != '\0'; ++i) {
        uint_t j = 0;
        while (src[j] != '\0' && dest[i + j] != '\0' && dest[i + j] == src[j]) {
            ++j;
        }
        if (src[j] == '\0') {
            return i;
        }
    }
    return NOT_FOUND;
}

/** Safely appends the characters in <src> to <dest> with proper null-termination. Returns <dest>. */
char_t *strappend(char_t *dest, string_t src) {
    assert(dest != NULL, "strappend() - dest was NULL!");
    assert(src != NULL, "strappend() - src was NULL!");
    uint_t len = strlen(dest);
    uint_t j = 0;
    while (src[j] != '\0') {
        dest[len + j] = src[j];
        ++j;
    }
    dest[len + j] = '\0';
    return dest;
}

/** Safely appends <c> at the end of <str> with proper null-termination. Returns <str>. */
char_t *strpush(char_t *str, char_t c) {
    assert(str != NULL, "strpush() - str was NULL!");
    uint_t len = strlen(str);
    str[len++] = c;
    str[len] = '\0';
    return str;
}

/** Safely removes <num> characters from the end of <str> with proper null-termination. Returns <str>. */
char_t *strpop(char_t *str, uint_t num) {
    assert(str != NULL, "strpop() - str was NULL!");
    uint_t len = strlen(str);
    if (num >= len) {
        *str = '\0';
    } else {
        str[len - num] = '\0';
    }
    return str;
}

/** Safely removes <num> characters from the front of <str> with proper null-termination. Returns <str>. */
char_t *strpull(char_t *str, uint_t num) {
    assert(str != NULL, "strpull() - str was NULL!");
    uint_t len = strlen(str);
    if (num >= len) {
        *str = '\0';
    } else if (num > 0) {
        copy(str, str + num, len - num + 1);
    }
    return str;
}

/** Safely copies <len> characters in <src> to <dest> with proper null-termination. Returns <dest>. */
char_t *substr(char_t *dest, string_t src, uint_t len) {
    assert(dest != NULL, "substr() - dest was NULL!");
    assert(src != NULL, "substr() - src was NULL!");
    uint_t i = 0;
    while (i < len && src[i] != '\0') {
        dest[i] = src[i];
        ++i;
    }
    dest[i] = '\0';
    return dest;
}

/** Converts all characters in <str> to uppercase. Returns <str>. */
char_t *strupper(char_t *str) {
    assert(str != NULL, "strupper() - str was NULL!");
    uint_t len = 0;
    while (str[len] != '\0') {
        str[len] = upper(str[len]);
        ++len;
    }
    return str;
}

/** Converts all characters in <str> to lowercase. Returns <str>. */
char_t *strlower(char_t *str) {
    assert(str != NULL, "strlower() - str was NULL!");
    uint_t len = 0;
    while (str[len] != '\0') {
        str[len] = lower(str[len]);
        ++len;
    }
    return str;
}

/** Safely removes leading, trailing, and duplicate whitespace characters in <str> with proper null-termination. Returns <str>. */
char_t *strnormal(char_t *str) {
    assert(str != NULL, "strnormal() - str was NULL!");
    uint_t read = 0;
    uint_t write = 0;
    char_t last = '\0';
    while (isspace(str[read])) {
        ++read;
    }
    while (str[read] != '\0') {
        if (isspace(str[read])) {
            if (last != str[read]) {
                str[write++] = str[read];
                last = str[read];
            }
        } else {
            str[write++] = str[read];
            last = str[read];
        }
        ++read;
    }
    if (write > 0 && isspace(str[write - 1])) {
        --write;
    }
    str[write] = '\0';
    return str;
}

/**
 * Transforms the given string using the given <trans> function. Returns <str>.
 * <trans> is a function that inputs a pointer to the start of the string, a pointer to the
 * current character, and returns whether the character should be inserted back into the string.
 */
char_t *strtrans(char_t *str, bool_t (*trans)(char_t *, char_t *)) {
    assert(str != NULL, "strtrans() - str was NULL!");
    assert(trans != NULL, "strtrans() - trans was NULL!");
    char_t *read = str;
    char_t *write = str;
    while (*read != '\0') {
        char_t *current = read;
        if (trans(str, current)) {
            *write = *read;
            ++write;
        }
        ++read;
    }
    *write = '\0';
    return str;
}

/**
 * Converts a string into an integer if possible.
 * <num> is set to the integer if <str> represents a valid integer.
 * Returns whether the string was successfully converted.
 */
bool_t strint(string_t str, int_t *num) {
    assert(str != NULL, "strint() - str was NULL!");
    while (isspace(*str)) {
        ++str;
    }
    int_t sign = 1;
    if (*str == '+' || *str == '-') {
        if (*str == '-') {
            sign = -1;
        }
        ++str;
    }
    if (!isnumber(*str)) {
        return false;
    }
    int_t value = 0;
    while (isnumber(*str)) {
        int_t digit = *str - '0';
        if (value > (2147483647 - digit) / 10) {
            return false;
        }
        value = value * 10 + digit;
        ++str;
    }
    if (num != NULL) {
        *num = sign * value;
    }
    return true;
}

/**
 * Converts an integer into a string.
 * The returned string is reused for all conversions.
 */
string_t intstr(int_t num) {
    static char_t buffer[12] = {0}; // Longest string: -2147483647\0
    char_t *current = &buffer[11];
    bool_t positive = (num >= 0);
    uint_t magnitude = positive ? (uint_t) num : (uint_t) (-(num + 1)) + 1;
    do {
        *--current = (char_t) ('0' + (magnitude % 10));
        magnitude /= 10;
    } while (magnitude > 0);
    if (!positive) {
        *--current = '-';
    }
    return current;
}
