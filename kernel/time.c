// .c
// OS Time and Sleep Functions
// by Kyle Furey

#include "hlos.h"

/** The number of ticks since the kernel started. */
volatile uint_t ticks = 0;

/** The number of ticks to increment each interrupt by the Programmable Interval Timer. */
uint_t speed = 1;

/** Returns the total number of milliseconds since the kernel booted. */
uint_t time() {
    cli();
    uint_t ms = ticks;
    sti();
    return ms * PIT_TICK_MS;
}

/** Pauses the kernel for about the given number of milliseconds. */
void sleep(uint_t ms) {
    ms += time();
    while (time() < ms) {
        hlt();
    }
}

/** Returns the current date and time from the CMOS clock. */
date_t date() {
    date_t current;
    out(CMOS_WRITE_PORT, CMOS_REG_STATUSA);
    while ((in(CMOS_READ_PORT) & CMOS_FLAGS_UPDATE) != 0) {
        hlt();
    }
    cli();
    out(CMOS_WRITE_PORT, CMOS_REG_CENTURY);
    byte_t century = in(CMOS_READ_PORT);
    out(CMOS_WRITE_PORT, CMOS_REG_YEAR);
    current.year = in(CMOS_READ_PORT);
    out(CMOS_WRITE_PORT, CMOS_REG_MONTH);
    current.month = in(CMOS_READ_PORT);
    out(CMOS_WRITE_PORT, CMOS_REG_DAY);
    current.day = in(CMOS_READ_PORT);
    out(CMOS_WRITE_PORT, CMOS_REG_HOUR);
    current.hour = in(CMOS_READ_PORT);
    out(CMOS_WRITE_PORT, CMOS_REG_MINUTE);
    current.minute = in(CMOS_READ_PORT);
    out(CMOS_WRITE_PORT, CMOS_REG_SECOND);
    current.second = in(CMOS_READ_PORT);
    out(CMOS_WRITE_PORT, CMOS_REG_STATUSB);
    byte_t status = in(CMOS_READ_PORT);
    if ((status & CMOS_FLAGS_BCD) == 0) {
        century = BCD(century);
        current.year = BCD(current.year);
        current.month = BCD(current.month);
        current.day = BCD(current.day);
        current.hour = BCD(current.hour);
        current.minute = BCD(current.minute);
        current.second = BCD(current.second);
    }
    if ((status & CMOS_FLAGS_24HR) == 0 && (current.hour & CMOS_FLAGS_PM) != 0) {
        current.hour = ((current.hour & 127) + 12) % 24;
    }
    if (century == 0) {
        current.year += 2000;
    } else {
        current.year += (century * 100);
    }
    sti();
    return current;
}

/**
 * Converts the given date to a string, optionally with 24 hr time.
 * The returned string is reused for all conversions.
 */
string_t datestr(date_t date, bool_t hr24) {
    static char_t buffer[23] = {0}; // Longest string: YYYY-MM-DD HH:MM:SS AM\0
    char_t *current = buffer;
    const char_t *temp = intstr(date.year);
    strcopy(current, temp);
    while (*current) {
        ++current;
    }
    *current++ = '-';
    if (date.month < 10) {
        *current++ = '0';
    }
    temp = intstr(date.month);
    strcopy(current, temp);
    while (*current) {
        ++current;
    }
    *current++ = '-';
    if (date.day < 10) {
        *current++ = '0';
    }
    temp = intstr(date.day);
    strcopy(current, temp);
    while (*current) {
        ++current;
    }
    *current++ = ' ';
    bool_t pm = false;
    if (!hr24) {
        if (date.hour == 0) {
            date.hour = 12;
        } else if (date.hour == 12) {
            pm = true;
        } else if (date.hour > 12) {
            date.hour -= 12;
            pm = true;
        }
    }
    if (date.hour < 10) {
        *current++ = '0';
    }
    temp = intstr(date.hour);
    strcopy(current, temp);
    while (*current) {
        ++current;
    }
    *current++ = ':';
    if (date.minute < 10) {
        *current++ = '0';
    }
    temp = intstr(date.minute);
    strcopy(current, temp);
    while (*current) {
        ++current;
    }
    *current++ = ':';
    if (date.second < 10) {
        *current++ = '0';
    }
    temp = intstr(date.second);
    strcopy(current, temp);
    while (*current) {
        ++current;
    }
    if (!hr24) {
        *current++ = ' ';
        *current++ = pm ? 'P' : 'A';
        *current++ = 'M';
    }
    *current = '\0';
    return buffer;
}

/** Increments the kernel timer. */
void tick() {
    ticks += speed;
    async();
}
