// .h
// OS Random Number Functions
// by Kyle Furey

#ifndef HLOS_RNG_H
#define HLOS_RNG_H

#include "types.h"

/** The seed for random number generation. */
extern uint_t seed;

#ifdef __cplusplus
extern "C" {
#endif

/** Returns a pseudo-randomly generated number. */
uint_t rng();

/** Returns a pseudo-randomly generated number within the given range. */
uint_t rngrange(uint_t min, uint_t max);

#ifdef __cplusplus
}
#endif

#endif // HLOS_RNG_H
