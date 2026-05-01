// .h
// OS String Functions
// by Kyle Furey

#ifndef HLOS_STRING_H
#define HLOS_STRING_H

#include "types.h"

/** The maximum length of a string buffer (including the null-terminator). */
#define MAX_STRING_LEN 2000

/** An enumeration of different comparison results. */
typedef enum comparison {
    LESS_THAN = -1,
    EQUAL_TO = 0,
    GREATER_THAN = 1
} comparison_t;

#ifdef __cplusplus
extern "C" {
#endif

/** Returns whether the given character is a letter. */
bool_t isletter(char_t c);

/** Returns whether the given character is a number. */
bool_t isnumber(char_t c);

/** Returns whether the given character is whitespace. */
bool_t isspace(char_t c);

/** Returns whether the given character is an uppercase letter. */
bool_t isupper(char_t c);

/** Returns whether the given character is a lowercase letter. */
bool_t islower(char_t c);

/** Returns the uppercase version of the given character. */
char_t upper(char_t c);

/** Returns the lowercase version of the given character. */
char_t lower(char_t c);

/** Returns the length of a string. */
uint_t strlen(string_t str);

/** Returns the length of the smaller string. */
uint_t strmin(string_t left, string_t right);

/** Generates a unique hash code from the given string. */
uint_t strhash(string_t str);

/** Safely copies <src> into <dest> with proper null-termination. Returns <dest>. */
char_t *strcopy(char_t *dest, string_t src);

/**
 * Compares <left> and <right> character by character.
 * Returns a comparison enum of <left> to <right> for the first differing character.
 */
comparison_t strcompare(string_t left, string_t right);

/** Safely sets each character in <str> to <c>. Returns <str>. */
char_t *strset(char_t *str, char_t c);

/** Safely swaps the characters of <left> and <right> with proper null-termination. Returns <left>. */
char_t *strswap(char_t *left, char_t *right);

/** Safely reverses each character in <str>. Returns <str>. */
char_t *strflip(char_t *str);

/** Returns the index of the first character that matches <c> in <str> (or NOT_FOUND). */
uint_t strfirst(string_t str, char_t c);

/** Returns the index of to the last character that matches <c> in <str> (or NOT_FOUND). */
uint_t strlast(string_t str, char_t c);

/** Returns the first index of where <src> was found in <dest> (or NOT_FOUND). */
uint_t strfind(string_t dest, string_t src);

/** Safely appends the characters in <src> to <dest> with proper null-termination. Returns <dest>. */
char_t *strappend(char_t *dest, string_t src);

/** Safely appends <c> at the end of <str> with proper null-termination. Returns <str>. */
char_t *strpush(char_t *str, char_t c);

/** Safely removes <num> characters from the end of <str> with proper null-termination. Returns <str>. */
char_t *strpop(char_t *str, uint_t num);

/** Safely removes <num> characters from the front of <str> with proper null-termination. Returns <str>. */
char_t *strpull(char_t *str, uint_t num);

/** Safely copies <len> characters in <src> to <dest> with proper null-termination. Returns <dest>. */
char_t *substr(char_t *dest, string_t src, uint_t len);

/** Converts all characters in <str> to uppercase. Returns <str>. */
char_t *strupper(char_t *str);

/** Converts all characters in <str> to lowercase. Returns <str>. */
char_t *strlower(char_t *str);

/** Safely removes leading, trailing, and duplicate whitespace characters in <str> with proper null-termination. Returns <str>. */
char_t *strnormal(char_t *str);

/**
 * Transforms the given string using the given <trans> function. Returns <str>.
 * <trans> is a function that inputs a pointer to the start of the string, a pointer to the
 * current character, and returns whether the character should be inserted back into the string.
 */
char_t *strtrans(char_t *str, bool_t (*trans)(char_t *, char_t *));

/**
 * Converts a string into an integer if possible.
 * <num> is set to the integer if <str> represents a valid integer.
 * Returns whether the string was successfully converted.
 */
bool_t strint(string_t str, int_t *num);

/**
 * Converts an integer into a string.
 * The returned string is reused for all conversions.
 */
string_t intstr(int_t num);

#ifdef __cplusplus
}
#endif

#endif // HLOS_STRING_H
