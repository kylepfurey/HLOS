// .h
// OS File IO Functions
// by Kyle Furey

#ifndef HLOS_FILE_H
#define HLOS_FILE_H

#include "types.h"

/** Advanced Technology Attachment master drive command. */
#define ATA_MASTER_DRIVE 0xE0

/** Advanced Technology Attachment slave drive command. */
#define ATA_SLAVE_DRIVE 0xF0

/** Advanced Technology Attachment read command. */
#define ATA_READ 0x20

/** Advanced Technology Attachment write command. */
#define ATA_WRITE 0x30

/** Advanced Technology Attachment busy status. */
#define ATA_BUSY 0x80

/** Advanced Technology Attachment request status. */
#define ATA_REQUEST 0x8

/** The size in bytes of a sector. */
#define SECTOR_SIZE 512

/** The maximum size of a file (not including the file header). */
#define MAX_FILE_SIZE 1000000

/** Indicates the entire file will be read. */
#define ENTIRE_FILE ((uint_t)-1)

/** Advanced Technology Attachment ports. */
typedef enum ATA_port {
    ATA_PRIMARY_PORT = 0,
    ATA_SECONDARY_PORT = 1,
    ATA_PORT_PRIMARY_DATA = 0x1F0,
    ATA_PORT_PRIMARY_ERROR = 0x1F1,
    ATA_PORT_PRIMARY_FEATURE = 0x1F1,
    ATA_PORT_PRIMARY_SECTOR_COUNT = 0x1F2,
    ATA_PORT_PRIMARY_LBA_LOW = 0x1F3,
    ATA_PORT_PRIMARY_LBA_MID = 0x1F4,
    ATA_PORT_PRIMARY_LBA_HIGH = 0x1F5,
    ATA_PORT_PRIMARY_DRIVE = 0x1F6,
    ATA_PORT_PRIMARY_CMD = 0x1F7,
    ATA_PORT_PRIMARY_STATUS = 0x1F7,
    ATA_PORT_SECONDARY_DATA = 0x170,
    ATA_PORT_SECONDARY_ERROR = 0x171,
    ATA_PORT_SECONDARY_FEATURE = 0x171,
    ATA_PORT_SECONDARY_SECTOR_COUNT = 0x172,
    ATA_PORT_SECONDARY_LBA_LOW = 0x173,
    ATA_PORT_SECONDARY_LBA_MID = 0x174,
    ATA_PORT_SECONDARY_LBA_HIGH = 0x175,
    ATA_PORT_SECONDARY_DRIVE = 0x176,
    ATA_PORT_SECONDARY_CMD = 0x177,
    ATA_PORT_SECONDARY_STATUS = 0x177,
} ATA_port_t;

/**
 * Reads the file at the given path and offset for the given size.
 * Returns a string containing the file's data or NULL if no file was found.
 */
string_t fileread(string_t path, uint_t offset, uint_t size);

/**
 * Writes the given data to the given file path.
 * Returns whether an existing file was overwritten.
 */
bool_t filewrite(string_t path, string_t data);

/**
 * Appends the given data to the given file path.
 * Returns whether an existing file was appended to.
 */
bool_t fileappend(string_t path, string_t data);

/**
 * Moves the file from <start> to <end>.
 * Returns whether an existing file was overwritten at <end>.
 */
bool_t filemove(string_t start, string_t end);

/** Deletes the file at the given path and returns whether a file was erased. */
bool_t filedelete(string_t path);

/** Returns whether a file exists and writes its size into <size>. */
bool_t filesize(string_t path, uint_t *size);

/** Reads <num> number of 512-byte sectors at <sec> into <str>. */
char_t *secread(ATA_port_t port, byte_t drive, uint_t sec, uint_t num, char_t *str);

/** Writes <str> into <num> number of 512-byte sectors at <sec>. */
string_t secwrite(ATA_port_t port, byte_t drive, uint_t sec, uint_t num, string_t str);

#endif // HLOS_FILE_H
