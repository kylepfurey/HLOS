// .h
// OS File IO Functions
// by Kyle Furey

#ifndef HLOS_FILE_H
#define HLOS_FILE_H

#include "lib.h"

/** The size in bytes of a sector. */
#define SECTOR_SIZE 512

/** Advanced Technology Attachment master drive command. */
#define ATA_MASTER_DRIVE 0xE0

/** Advanced Technology Attachment slave drive command. */
#define ATA_SLAVE_DRIVE 0xF0

/** Floppy disk media signature. */
#define FLOPPY_MEDIA 0xF0

/** Hard disk media signature. */
#define HARD_MEDIA 0xF8

/** The boot sector's signature. */
#define BOOT_SIGNATURE 0xAA55

/** The file system sector's lead signature. */
#define LEAD_SIGNATURE 0x41615252

/** The file system sector's struct signature. */
#define STRUCT_SIGNATURE 0x61417272

/** Converts a mounted FAT32 cluster entry into a FAT32 sector. */
#define CLUSTER_TO_SECTOR(cluster) ((FAT32.start + FAT32.boot.reserved_count + (FAT32.boot.FAT_count * FAT32.boot.FAT_size32)) + (((cluster) - 2) * FAT32.boot.cluster_size))

/** Converts a mounted FAT32 sector into a FAT32 cluster entry. */
#define SECTOR_TO_CLUSTER(sector) ((((sector) - (FAT32.start + FAT32.boot.reserved_count + (FAT32.boot.FAT_count * FAT32.boot.FAT_size32))) / FAT32.boot.cluster_size) + 2)

/** Returns the cluster entry for the given directory. */
#define DIRECTORY_CLUSTER(dir) (((dir).cluster_high << 16) | (dir).cluster_low)

/** Returns whether a cluster entry is the end of a cluster chain. */
#define CLUSTER_END(clus) ((clus) >= FAT32_CLUSTER_RESERVED && (clus) <= FAT32_CLUSTER_END)

/** Returns whether a directory is the end of a directory array. */
#define DIRECTORY_END(dir) ((dir).name[0] == FAT32_DIRECTORY_END)

/** Loads the entire cluster chain into memory. */
#define ALL_CLUSTERS ((uint_t) -1)

/** Loads the entire file into memory. */
#define ALL_BYTES ((uint_t) -1)

/** The maximum length of a file name (including dot and null-terminator). */
#define FILE_NAME_LEN 13

/** The sector FAT32 resides on disk. */
#define FAT32_START 80

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

/** File Allocation Table (32-bit) attributes. */
typedef enum FAT32_attributes {
    FAT32_ATTRIBUTE_READONLY = 1,
    FAT32_ATTRIBUTE_HIDDEN = 2,
    FAT32_ATTRIBUTE_SYSTEM = 4,
    FAT32_ATTRIBUTE_VOLUME = 8,
    FAT32_ATTRIBUTE_DIRECTORY = 16,
    FAT32_ATTRIBUTE_FILE = 32,
} FAT32_attributes_t;

/** File Allocation Table (32-bit) states. */
typedef enum FAT32_state {
    FAT32_CLUSTER_FREE = 0x0,
    FAT32_CLUSTER_BAD = 0xFFFFFFF7,
    FAT32_CLUSTER_RESERVED = 0xFFFFFFF8,
    FAT32_CLUSTER_END = 0xFFFFFFFF,
    FAT32_DIRECTORY_SKIP = 0xE5,
    FAT32_DIRECTORY_END = 0x0,
} FAT32_state_t;

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

/** A single File Allocation Table (32-bit) cluster entry. */
typedef uint_t FAT32_cluster_t;

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
typedef struct FAT32_cache {
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

    /** Cached data for the FAT32 sectors. */
    FAT32_cluster_t *table;
} FAT32_cache_t;

/** Cached data for the mounted File Allocation Table (32-bit). */
extern FAT32_cache_t FAT32;

/**
 * Reads the file at <path> at <offset> for <size> into <file>.
 * Returns whether reading was successful.
 */
bool_t fileread(string_t path, uint_t offset, uint_t size, char_t *file);

/**
 * Writes <size> bytes in <data> to the file at <path>.
 * Returns whether writing was successful.
 */
bool_t filewrite(string_t path, uint_t size, string_t data);

/**
 * Appends <size> bytes in <data> to the file at <path>.
 * Returns whether writing was successful.
 */
bool_t fileappend(string_t path, uint_t size, string_t data);

/**
 * Moves the file from <start> to <end>.
 * Returns whether moving was successful.
 */
bool_t filemove(string_t dest, string_t src);

/** Deletes the file at <path> and returns whether a file was erased. */
bool_t filedelete(string_t path);

/** Returns whether a file exists at <path> and writes its size into <size>. */
bool_t filesize(string_t path, uint_t *size);

/**
 * Fills <list> with the first <size> names of each file in <path> and returns the number of files.
 * <list> must be at least FILE_NAME_LEN * <size> characters.
 */
uint_t filelist(string_t path, uint_t size, char_t *list);

/** Mounts the given FAT32 partition as the current hard drive, if possible. */
bool_t mount(ATA_port_t port, byte_t drive, uint_t start);

/** Formats the hard drive for FAT32. */
bool_t format(ATA_port_t port, byte_t drive, uint_t start, uint_t clus_count, uint_t part_size, bool_t force);

/**
 * Allocates new clusters for the mounted FAT32 instance at <path>. <clus> is set to the cluster of the created directory.
 * Returns the new directory. name[0] is FAT32_DIRECTORY_END on failure.
 */
FAT32_directory_t FAT32_alloc(
    string_t path,
    uint_t size,
    string_t data,
    FAT32_attributes_t attr,
    bool_t force,
    FAT32_cluster_t *clus
);

/** Loads all of <clus> for the mounted FAT32 instance into memory. This pointer must be freed! */
void *FAT32_load(FAT32_cluster_t clus, uint_t max);

/**
 * Returns the directory for the file at <path>. <clus> is set to the cluster of the found directory.
 * name[0] is FAT32_DIRECTORY_END on failure.
 */
FAT32_directory_t FAT32_find(string_t path, FAT32_cluster_t *clus);

/** Frees the cluster chain for the mounted FAT32 instance at <path>. Returns whether it was successful. */
bool_t FAT32_free(string_t path);

/** Reads <num> number of 512-byte sectors at <sec> into <str>. */
char_t *secread(ATA_port_t port, byte_t drive, uint_t sec, byte_t num, char_t *str);

/** Writes <str> into <num> number of 512-byte sectors at <sec>. */
string_t secwrite(ATA_port_t port, byte_t drive, uint_t sec, byte_t num, string_t str);

#endif // HLOS_FILE_H
