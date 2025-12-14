// .h
// OS File IO Functions
// by Kyle Furey

#ifndef HLOS_FILE_H
#define HLOS_FILE_H

#include "lib.h"

/** Advanced Technology Attachment master drive command. */
#define ATA_MASTER_DRIVE 0xE0

/** Advanced Technology Attachment slave drive command. */
#define ATA_SLAVE_DRIVE 0xF0

/** Advanced Technology Attachment read command. */
#define ATA_READ 0x20

/** Advanced Technology Attachment write command. */
#define ATA_WRITE 0x30

/** Advanced Technology Attachment reset command. */
#define ATA_RESET 0x4

/** Advanced Technology Attachment busy status. */
#define ATA_BUSY 0x80

/** Advanced Technology Attachment request status. */
#define ATA_REQUEST 0x8

/** Advanced Technology Attachment busy status. */
#define ATA_ERROR 0x1

/** The size in bytes of a sector. */
#define SECTOR_SIZE 512

/** The sector FAT32 resides on disk. */
#define FAT32_START 64

/** The size in sectors of FAT32. */
#define FAT32_SIZE 1048576

/** The boot sector's signature. */
#define BOOT_SIGNATURE 0xAA55

/** The file system sector's lead signature. */
#define LEAD_SIGNATURE 0x41615252

/** The file system sector's struct signature. */
#define STRUCT_SIGNATURE 0x61417272

/** FAT32 reserved entry 1. */
#define FAT32_RESERVED 0xFFFFFF8

/** FAT32 reserved entry 2. */
#define FAT32_END 0xFFFFFFF

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
    ATA_PORT_PRIMARY_CTRL = 0x3F6,
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
    ATA_PORT_SECONDARY_CTRL = 0x376,
} ATA_port_t;

/** File Allocation Table (32-bit) file attributes. */
typedef enum FAT32_attributes {
    FAT32_ATTRIBUTE_READONLY = 1,
    FAT32_ATTRIBUTE_HIDDEN = 2,
    FAT32_ATTRIBUTE_SYSTEM = 4,
    FAT32_ATTRIBUTE_VOLUME = 8,
    FAT32_ATTRIBUTE_DIRECTORY = 16,
    FAT32_ATTRIBUTE_ARCHIVE = 32,
} FAT32_attributes_t;

#pragma pack(push, 1)

/** File Allocation Table (32-bit) boot sector. */
typedef struct FAT32_boot {
    byte_t jump[3];
    char_t name[8];
    ushort_t sector_size;
    byte_t cluster_size;
    ushort_t reserved_count;
    byte_t FAT_count;
    ushort_t root_count;
    ushort_t sector_count16;
    byte_t media;
    ushort_t FAT_size16;
    ushort_t track_size;
    ushort_t head_count;
    uint_t hidden_count;
    uint_t sector_count32;
    uint_t FAT_size32;
    ushort_t flags;
    ushort_t FS_version;
    uint_t root;
    ushort_t FS_info;
    ushort_t backup;
    byte_t reserved[12];
    byte_t bootloader[446];
    ushort_t boot_signature;
} __attribute__((packed)) FAT32_boot_t;

/** File Allocation Table (32-bit) file system sector. */
typedef struct FAT32_FS {
    uint_t lead_signature;
    byte_t reserved1[480];
    uint_t struct_signature;
    uint_t free;
    uint_t next;
    byte_t reserved2[14];
    ushort_t boot_signature;
} __attribute__((packed)) FAT32_FS_t;

/** A single File Allocation Table (32-bit) entry. */
typedef uint_t FAT32_entry_t;

/** File Allocation Table (32-bit) table sectors. */
typedef struct FAT32_table {
    FAT32_entry_t entries[128];
} __attribute__((packed)) FAT32_table_t;

/** File Allocation Table (32-bit) directory. */
typedef struct FAT32_directory {
    char_t name[8];
    char_t extension[3];
    byte_t attributes;
    byte_t reserved;
    byte_t create_ms;
    ushort_t create_time;
    ushort_t create_date;
    ushort_t open_date;
    ushort_t cluster_high;
    ushort_t write_time;
    ushort_t write_date;
    ushort_t cluster_low;
    uint_t size;
} __attribute__((packed)) FAT32_directory_t;

#pragma pack(pop)

/** Cached data for the File Allocation Table (32-bit). */
typedef struct FAT32 {
    /** Whether FAT32 is currently mounted. */
    bool_t mounted;

    /** The port used to read and write the FAT32 instance. */
    ATA_port_t port;

    /** The drive used to read and write the FAT32 instance. */
    byte_t drive;

    /** The boot sector of the FAT32 instance. */
    uint_t start;

    /** Cached data for the FAT32 boot sector. */
    FAT32_boot_t boot;

    /** Cached data for the FAT32 file system sector. */
    FAT32_FS_t fs;

    /** The current local directory of the FAT32 file system.*/
    string_t dir;
} FAT32_t;

/** Cached data for the mounted File Allocation Table (32-bit). */
extern FAT32_t FAT;

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

/** Fills <list> with the first <size> names of each file in <dir> and returns whether it was successful. */
bool_t filelist(string_t dir, uint_t size, char_t **list);

/** Mounts the given FAT32 partition as the current hard drive, if possible. */
bool_t mount(ATA_port_t port, byte_t drive, uint_t start);

/** Formats the hard drive for FAT32. */
bool_t format(ATA_port_t port, byte_t drive, uint_t start, uint_t clus_size, uint_t part_size, bool_t force);

/** Reads <num> number of 512-byte sectors at <sec> into <str>. */
char_t *secread(ATA_port_t port, byte_t drive, uint_t sec, uint_t num, char_t *str);

/** Writes <str> into <num> number of 512-byte sectors at <sec>. */
string_t secwrite(ATA_port_t port, byte_t drive, uint_t sec, uint_t num, string_t str);

#endif // HLOS_FILE_H
