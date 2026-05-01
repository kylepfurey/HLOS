// .h
// OS Beep Sound Functions
// by Kyle Furey

#ifndef HLOS_BEEP_H
#define HLOS_BEEP_H

#include "types.h"

/** Programmable Interval Timer channel 2 (audio) port. */
#define PIT_CHANNEL2_PORT 0x42

/** Default audio speaker port. */
#define SPEAKER_PORT 0x61

/** Square wave sound setting. */
#define SQUARE_WAVE_NUM 0xB6

/** Audio speaker flags. */
typedef enum speaker_flags {
    SPEAKER_FLAGS_ENABLE = 3,
    SPEAKER_FLAGS_CLEAR = 252,
} speaker_flags_t;

#ifdef __cplusplus
extern "C" {
#endif

/** Beeps the computer. */
void beep();

/** Beeps for the given frequency in hertz and duration in milliseconds. */
void freq(uint_t hz, uint_t ms);

#ifdef __cplusplus
}
#endif

#endif // HLOS_BEEP_H
