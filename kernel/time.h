// .h
// OS Time and Sleep Functions
// by Kyle Furey

#ifndef HLOS_TIME_H
#define HLOS_TIME_H

#include "types.h"

/** Programmable Interval Timer command port. */
#define PIT_CMD_PORT 0x43

/** Programmable Interval Timer command number. */
#define PIT_CMD_NUM 0x36

/** Programmable Interval Timer channel 0 (time). */
#define PIT_CHANNEL0_PORT 0x40

/** The frequency of the Programmable Interval Timer. */
#define PIT_FREQ 1193182

/** The number of Programmable Interval Timer interrupts per second. */
#define PIT_SPEED_HZ 1000

/** Calculates the value used to set the speed of the Programmable Interval Timer. */
#define PIT_DIVISOR(hz) (PIT_FREQ / (hz))

/** The time in milliseconds of a Programmable Interval Timer tick. */
#define PIT_TICK_MS (1000 / PIT_SPEED_HZ)

/** CMOS clock output port. */
#define CMOS_WRITE_PORT 0x70

/** CMOS clock input port. */
#define CMOS_READ_PORT 0x71

/** Converts a binary-coded decimal value to a regular binary value. */
#define BCD(bcd) ((((bcd) >> 4) * 10) + ((bcd) & 0xF))

/** CMOS clock registers. */
typedef enum CMOS_register {
    CMOS_REG_SECOND = 0x0,
    CMOS_REG_MINUTE = 0x2,
    CMOS_REG_HOUR = 0x4,
    CMOS_REG_DAY = 0x7,
    CMOS_REG_MONTH = 0x8,
    CMOS_REG_YEAR = 0x9,
    CMOS_REG_CENTURY = 0x32,
    CMOS_REG_STATUSA = 0xA,
    CMOS_REG_STATUSB = 0xB,
} CMOS_register_t;

/** CMOS clock flags. */
typedef enum CMOS_flag {
    CMOS_FLAGS_UPDATE = 0x80,
    CMOS_FLAGS_BCD = 0x4,
    CMOS_FLAGS_24HR = 0x2,
    CMOS_FLAGS_PM = 0x80,
} CMOS_flags_t;

/** Numerical values that represent the current date and time. */
typedef struct date {
    /** The 4-digit year. */
    ushort_t year;

    /** 1 of 12 months. */
    byte_t month;

    /** 1 day of 28-31 days of the month. */
    byte_t day;

    /** 1 of 24 hours of the day. */
    byte_t hour;

    /** 1 of 60 minutes of the hour. */
    byte_t minute;

    /** 1 of 60 seconds of the hour. */
    byte_t second;
} date_t;

/** The number of ticks since the kernel started. */
extern volatile uint_t ticks;

/** The number of ticks to increment each interrupt by the Programmable Interval Timer. */
extern uint_t speed;

/** Returns the total number of milliseconds since the kernel booted. */
uint_t time();

/** Pauses the kernel for about the given number of milliseconds. */
void sleep(uint_t ms);

/** Returns the current date and time from the CMOS clock. */
date_t date();

/**
 * Converts the given date to a string, optionally with 24 hr time.
 * The returned string is reused for all conversions.
 */
string_t datestr(date_t date, bool_t hr24);

/** Increments the kernel timer. */
void tick();

#endif // HLOS_TIME_H
