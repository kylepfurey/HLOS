// .c
// OS Beep Sound Functions
// by Kyle Furey

#include "hlos.h"

/** Beeps the computer. */
void beep() {
    freq(800, 200);
}

/** Beeps for the given frequency and duration. */
void freq(uint_t hz, uint_t ms) {
    if (hz == 0 || ms == 0) {
        return;
    }
    uint_t divisor = PIT_DIVISOR(hz);
    out(PIT_CMD_PORT, SQUARE_WAVE_NUM);
    out(PIT_CHANNEL2_PORT, (byte_t) (divisor & 255));
    out(PIT_CHANNEL2_PORT, (byte_t) ((divisor >> 8) & 255));
    byte_t speaker = in(SPEAKER_PORT);
    if (!(speaker & SPEAKER_FLAGS_ENABLE)) {
        out(SPEAKER_PORT, speaker | SPEAKER_FLAGS_ENABLE);
    }
    sleep(ms);
    speaker = in(SPEAKER_PORT) & SPEAKER_FLAGS_CLEAR;
    out(SPEAKER_PORT, speaker);
}
