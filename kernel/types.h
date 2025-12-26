// .h
// OS Type Declarations
// by Kyle Furey

#ifndef HLOS_TYPES_H
#define HLOS_TYPES_H

#include "extern.h"

/** A true or false value. */
typedef bool bool_t;

/** An 8-bit signed integral number. */
typedef int8_t sbyte_t;

/** An 8-bit unsigned integral number. */
typedef uint8_t byte_t;

/** A 16-bit signed integral number. */
typedef int16_t short_t;

/** A 16-bit unsigned integral number. */
typedef uint16_t ushort_t;

/** A 32-bit signed integral number. */
typedef int32_t int_t;

/** A 32-bit unsigned integral number. */
typedef uint32_t uint_t;

/** A 32-bit floating-point decimal number. */
typedef float float_t;

/** An 8-bit ASCII character. */
typedef char char_t;

/** A null-terminated immutable sequence of characters. */
typedef const char_t *string_t;

#endif // HLOS_TYPES_H
