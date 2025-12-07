// .h
// OS Event and Coroutine Functions
// by Kyle Furey

#ifndef HLOS_EVENT_H
#define HLOS_EVENT_H

#include "types.h"

/** The maximum number of events that can be bound at once. */
#define MAX_EVENTS 16

/** The maximum number of coroutines that can be bound at once. */
#define MAX_COROUTINES 16

/** The maximum number of coroutines that can be processed at once. */
#define MAX_COROUTINES_PROCESSED 2

/** A handle representing a scheduled coroutine. */
typedef uint_t handle_t;

/** An asynchronous coroutine. */
typedef struct coroutine {
    /** The current handle for this coroutine. */
    handle_t handle;

    /** When this coroutine will be invoked. */
    uint_t schedule;

    /** The function to call. */
    void (*callback)(void *);

    /** A pointer to arguments for this coroutine. */
    void *args;
} coroutine_t;

/** Each bound event function. */
extern void (*volatile events[MAX_EVENTS])(void *);

/** Each bound coroutine. */
extern volatile coroutine_t coroutines[MAX_COROUTINES];

/** Sets the event callback at the given index. */
void event(uint_t index, void (*event)(void *));

/** Invokes the event callback at the given index with the given args. */
void invoke(uint_t index, void *args);

/**
 * Sets a coroutine callback to be invoked after the given number of milliseconds.
 * <coroutine> is a function pointer with a void pointer to arguments as a parameter and no return type.
 * <args> is automatically passed to the function pointer when the coroutine starts.
 * Returns a handle for the scheduled coroutine.
 */
handle_t coro(uint_t ms, void (*coroutine)(void *), void *args);

/** Attempts to cancel the scheduled coroutine with the given handle. */
bool_t cancel(handle_t handle);

/** Processes coroutines. */
void async();

#endif // HLOS_EVENT_H
