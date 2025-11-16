// .c
// OS Multithreading Function
// by Kyle Furey

#include "thread.h"
#include "lib.h"

/**
 * Executes the given function asynchronously on the next available hardware thread.
 * <func> is a function pointer with a void pointer to arguments as a parameter and no return type.
 * <args> is automatically passed to the function pointer when the thread starts.
 * Returns a handle for the executing thread.
 */
thandle_t thread(void (*func)(void *), void *args) {
    assert(func != NULL, "thread() - func was NULL!");
    // TODO
    return 0;
}
