// .c
// OS Event and Coroutine Functions
// by Kyle Furey

#include "event.h"
#include "lib.h"
#include "assembly.h"
#include "time.h"

/** Each bound event function. */
void (*volatile events[MAX_EVENTS])(void *) = {0};

/** Each bound coroutine. */
volatile coroutine_t coroutines[MAX_COROUTINES] = {0};

/** Sets the event callback at the given index. */
void event(uint_t index, void (*event)(void *)) {
    assert(index < MAX_EVENTS, "event() - index was >= MAX_EVENTS!");
    events[index] = event;
}

/** Invokes the event callback at the given index with the given args. */
void invoke(uint_t index, void *args) {
    assert(index < MAX_EVENTS, "event() - index was >= MAX_EVENTS!");
    void (*event)(void *) = events[index];
    if (event != NULL) {
        event(args);
    }
}

/**
 * Sets a coroutine callback to be invoked after the given number of milliseconds.
 * <coroutine> is a function pointer with a void pointer to arguments as a parameter and no return type.
 * <args> is automatically passed to the function pointer when the coroutine starts.
 * Returns a handle for the scheduled coroutine.
 */
chandle_t coro(uint_t ms, void (*coroutine)(void *), void *args) {
    assert(coroutine != NULL, "coro() - coroutine was NULL!");
    uint_t schedule = time() + ms;
    cli();
    volatile coroutine_t *coro = NULL;
    chandle_t handle;
    for (uint_t i = 0; i < MAX_COROUTINES; ++i) {
        if (coroutines[i].handle % MAX_COROUTINES != i) {
            coroutines[i].handle = i;
        }
        if (coroutines[i].callback == NULL) {
            coro = &coroutines[i];
            handle = coro->handle;
            break;
        }
    }
    assert(coro != NULL, "coro() - Number of concurrent coroutines >= MAX_COROUTINES!");
    coro->schedule = schedule;
    coro->callback = coroutine;
    coro->args = args;
    sti();
    return handle;
}

/** Attempts to cancel the scheduled coroutine with the given handle. */
bool_t cancel(chandle_t handle) {
    cli();
    volatile coroutine_t *coro = &coroutines[handle % MAX_COROUTINES];
    bool_t result;
    if (handle == coro->handle) {
        coro->handle += MAX_COROUTINES;
        coro->schedule = 0;
        coro->callback = NULL;
        coro->args = NULL;
        result = true;
    } else {
        result = false;
    }
    sti();
    return result;
}

/** Processes coroutines. */
void async() {
    static uint_t index = 0;
    uint_t time = ticks * PIT_TICK_MS;
    uint_t count = 0;
    for (uint_t i = 0; i < MAX_COROUTINES && count < MAX_COROUTINES_PROCESSED; ++i) {
        if (coroutines[index].callback != NULL && time >= coroutines[index].schedule) {
            coroutines[index].callback(coroutines[index].args);
            coroutines[index].handle += MAX_COROUTINES;
            coroutines[index].schedule = 0;
            coroutines[index].callback = NULL;
            coroutines[index].args = NULL;
            ++count;
        }
        index = (index + 1) % MAX_COROUTINES;
    }
}
