// .h
// OS Library Functions
// by Kyle Furey

#ifndef HLOS_LIB_H
#define HLOS_LIB_H

#include "string.h"

/** Returns the number of elements in an array. */
#define ARRAY_SIZE(array) (sizeof((array)) / sizeof(((array))[0]))

/** An indicator that a value was not found. */
#define NOT_FOUND ((uint_t)-1)

/** Crashes the kernel with an error. */
#define panic(err) assert(false, err)

/** Inserts a breakpoint for a line of code. */
#define bp(code) step(#code); code

/** Converts raw memory into a string. */
#define tostr(mem) dump(sizeof(mem), &mem)

/** Halts the kernel. */
void pause();

/** Shuts down the kernel after the given number of milliseconds. */
void shutdown(uint_t ms);

/** Reboots the kernel after the given number of milliseconds. */
void reboot(uint_t ms);

/** Crashes the kernel with an error if the given condition is false. */
void assert(bool_t cond, string_t err);

/** Enables stepping through the kernel via keyboard input. */
void step(string_t msg);

/** Allows raw memory to be easily inspected as a string. */
string_t dump(uint_t size, void *addr);

/** Returns the lesser number. */
int_t min(int_t a, int_t b);

/** Returns the greater number. */
int_t max(int_t a, int_t b);

/** Returns a positive version of the given number. */
int_t abs(int_t num);

/** Rounds a number. */
int_t round(float_t num);

/** Rounds a number up. */
int_t ceil(float_t num);

/** Rounds a number down. */
int_t floor(float_t num);

/** Rounds a number to the next alignment boundary. */
uint_t align(uint_t num, uint_t align);

/** Generates a unique hash code from the given data. */
uint_t hash(const void *data, uint_t size);

/** Copies <size> number of bytes from <src> into <dest> without overlap. Returns <dest>. */
void *copy(void *dest, const void *src, uint_t size);

/**
 * Compares <left> and <right> byte by byte for <size>.
 * Returns a comparison enum of <left> to <right> for the first differing byte.
 */
comparison_t compare(const void *left, const void *right, uint_t size);

/** Sets each byte of <dest> to <num> for <size>. Returns <dest>. */
void *set(void *dest, byte_t num, uint_t size);

/** Swaps each byte of <left> and <right> for <size>. Returns <left>. */
void *swap(void *left, void *right, uint_t size);

/** Reverses <size> number of bytes in <src>. Returns <src>. */
void *flip(void *src, uint_t size);

/** Finds the first matching byte equal to <num> for <size> and returns its index (or NOT_FOUND). */
uint_t first(const void *src, byte_t num, uint_t size);

/** Finds the last matching byte equal to <num> for <size> and returns its index (or NOT_FOUND). */
uint_t last(const void *src, byte_t num, uint_t size);

/**
 * Returns the index of the first byte where the value of <src> of size <size2>
 * is located within <dest> within <size1> bytes (or NOT_FOUND).
 */
uint_t find(const void *dest, uint_t size1, const void *src, uint_t size2);

/** Copies <src> of size <size2> right after <dest> at <size1>. Returns <dest>. */
void *append(void *dest, uint_t size1, const void *src, uint_t size2);

#endif // HLOS_LIB_H
