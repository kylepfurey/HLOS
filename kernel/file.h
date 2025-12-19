// .h
// OS File IO Functions
// by Kyle Furey

#ifndef HLOS_FILE_H
#define HLOS_FILE_H

#include "lib.h"

/** The size in bytes of a sector. */
#define SECTOR_SIZE 512

/** The size in bytes of a directory. */
#define DIRECTORY_SIZE 32

/** Advanced Technology Attachment master drive command. */
#define ATA_MASTER_DRIVE 0xE0

/** Advanced Technology Attachment slave drive command. */
#define ATA_SLAVE_DRIVE 0xF0

/** The boot sector's signature. */
#define BOOT_SIGNATURE 0xAA55

/** The file system sector's lead signature. */
#define LEAD_SIGNATURE 0x41615252

/** The file system sector's struct signature. */
#define STRUCT_SIGNATURE 0x61417272

/** Converts a mounted FAT32 cluster into a FAT32 sector. */
#define CLUSTER_TO_SECTOR(cluster) ((FAT32.start + FAT32.boot.reserved_count + (FAT32.boot.FAT_count * FAT32.boot.FAT_size32)) + (((cluster) - 2) * FAT32.boot.cluster_size))

/** Converts a mounted FAT32 sector into a FAT32 cluster. */
#define SECTOR_TO_CLUSTER(sector) ((((sector) - (FAT32.start + FAT32.boot.reserved_count + (FAT32.boot.FAT_count * FAT32.boot.FAT_size32))) / FAT32.boot.cluster_size) + 2)

/** The sector FAT32 resides on disk. */
#define FAT32_START 64

/** The size in sectors of FAT32. */
#define FAT32_SIZE 1048576

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

/** Advanced Technology Attachment commands. */
typedef enum ATA_command {
    ATA_READ = 0x20,
    ATA_WRITE = 0x30,
    ATA_RESET = 0x4,
    ATA_BUSY = 0x80,
    ATA_REQUEST = 0x8,
    ATA_ERROR = 0x1,
} ATA_command_t;

/** File Allocation Table (32-bit) file attributes. */
typedef enum FAT32_attributes {
    FAT32_ATTRIBUTE_READONLY = 1,
    FAT32_ATTRIBUTE_HIDDEN = 2,
    FAT32_ATTRIBUTE_SYSTEM = 4,
    FAT32_ATTRIBUTE_VOLUME = 8,
    FAT32_ATTRIBUTE_DIRECTORY = 16,
    FAT32_ATTRIBUTE_FILE = 32,
} FAT32_attributes_t;

/** Fille Allocation Table (32-bit) cluster states. */
typedef enum FAT32_cluster_state {
    FAT32_RESERVED = 0xFFFFFF8,
    FAT32_END = 0xFFFFFFF,
    FAT32_EMPTY = 0x0,
    FAT32_FREE = 0xE5,
} FAT32_cluster_state_t;

#pragma pack(push, 1)

/** File Allocation Table (32-bit) boot sector. */
typedef struct FAT32_boot {
    byte_t jump[3];
    char_t OEM[8];
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
    byte_t reserved1[12];
    byte_t drive;
    byte_t reserved2;
    byte_t extended_signature;
    uint_t id;
    char_t name[11];
    char_t type[8];
    byte_t bootloader[420];
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
    FAT32_FS_t FS;

    /** The current local directory of the FAT32 file system.*/
    string_t dir;
} FAT32_t;

/** Cached data for the mounted File Allocation Table (32-bit). */
extern FAT32_t FAT32;

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

/** Fills <list> with the first <size> names of each file in <dir> and returns the number of files. */
uint_t filelist(string_t dir, uint_t size, char_t **list);

/** Mounts the given FAT32 partition as the current hard drive, if possible. */
bool_t mount(ATA_port_t port, byte_t drive, uint_t start);

/** Formats the hard drive for FAT32. */
bool_t format(ATA_port_t port, byte_t drive, uint_t start, uint_t clus_count, uint_t part_size, bool_t force);

/** Allocates a new file entry for the mounted FAT32 instance at <path>. Returns the new entry or NOT_FOUND. */
FAT32_entry_t FAT32_alloc(string_t path, uint_t size, FAT32_attributes_t attr);

/** Finds a file entry for the mounted FAT32 instance at <path>. Returns the new entry or NOT_FOUND. */
FAT32_entry_t FAT32_find(string_t path);

/** Frees the file entry for the mounted FAT32 instance at <path>. Returns whether it was successful. */
bool_t FAT32_free(FAT32_entry_t entry);

/** Reads <num> number of 512-byte sectors at <sec> into <str>. */
char_t *secread(ATA_port_t port, byte_t drive, uint_t sec, uint_t num, char_t *str);

/** Writes <str> into <num> number of 512-byte sectors at <sec>. */
string_t secwrite(ATA_port_t port, byte_t drive, uint_t sec, uint_t num, string_t str);

#endif // HLOS_FILE_H
