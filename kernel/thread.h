// .h
// OS Multithreading Function
// by Kyle Furey

#ifndef HLOS_THREAD_H
#define HLOS_THREAD_H

#include "types.h"

/** A handle representing an executing thread.  */
typedef uint_t thandle_t;

/**
 * Executes the given function asynchronously on the next available hardware thread.
 * <func> is a function pointer with a void pointer to arguments as a parameter and no return type.
 * <args> is automatically passed to the function pointer when the thread starts.
 * Returns a handle for the executing thread.
 */
thandle_t thread(void (*func)(void *), void *args);

#endif // HLOS_THREAD_H
