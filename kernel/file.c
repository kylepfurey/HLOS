// .c
// OS File IO Functions
// by Kyle Furey

#include "file.h"
#include "lib.h"
#include "assembly.h"
#include "time.h"
#include "malloc.h"

/** Cached data for the mounted File Allocation Table (32-bit). */
FAT32_cache_t FAT32 = {0};

/**
 * Reads the file at <path> at <offset> for <size> into <file>.
 * Returns <file> or NULL if no file was found.
 */
string_t fileread(string_t path, uint_t offset, uint_t size, char_t *file) {
    assert(path != NULL, "fileread() - path was NULL!");
    assert(file != NULL, "fileread() - file was NULL!");
    if (!FAT32.mounted) {
        return NULL;
    }
    FAT32_directory_t dir = FAT32_find(path);
    if (DIRECTORY_END(dir)) {
        return NULL;
    }
    // TODO
    return NULL;
}

/**
 * Writes <data> to the file at <path>.
 * Returns whether an existing file was overwritten.
 */
bool_t filewrite(string_t path, string_t data) {
    assert(path != NULL, "filewrite() - path was NULL!");
    if (!FAT32.mounted) {
        return false;
    }
    FAT32_directory_t dir = FAT32_find(path);
    if (DIRECTORY_END(dir)) {
        return false;
    }
    // TODO
    return false;
}

/**
 * Appends <data> to the file at <path>.
 * Returns whether an existing file was appended to.
 */
bool_t fileappend(string_t path, string_t data) {
    assert(path != NULL, "fileappend() - path was NULL!");
    if (!FAT32.mounted) {
        return false;
    }
    FAT32_directory_t dir = FAT32_find(path);
    if (DIRECTORY_END(dir)) {
        return false;
    }
    // TODO
    return false;
}

/**
 * Moves the file from <start> to <end>.
 * Returns whether an existing file was overwritten at <end>.
 */
bool_t filemove(string_t end, string_t start) {
    assert(end != NULL, "filemove() - end was NULL!");
    assert(start != NULL, "filemove() - start was NULL!");
    if (!FAT32.mounted) {
        return false;
    }
    FAT32_directory_t start_dir = FAT32_find(start);
    if (DIRECTORY_END(start_dir)) {
        return false;
    }
    FAT32_directory_t end_dir = FAT32_find(end);
    // TODO
    return false;
}

/** Deletes the file at <path> and returns whether a file was erased. */
bool_t filedelete(string_t path) {
    assert(path != NULL, "filedelete() - path was NULL!");
    if (!FAT32.mounted) {
        return NOT_FOUND;
    }
    FAT32_directory_t dir = FAT32_find(path);
    if (DIRECTORY_END(dir)) {
        return false;
    }
    return FAT32_free(DIRECTORY_CLUSTER(dir), strfirst(path, '.') == NOT_FOUND);
}

/** Returns whether a file exists at <path> and writes its size into <size>. */
bool_t filesize(string_t path, uint_t *size) {
    assert(path != NULL, "filesize() - path was NULL!");
    if (!FAT32.mounted) {
        return NOT_FOUND;
    }
    FAT32_directory_t dir = FAT32_find(path);
    if (DIRECTORY_END(dir)) {
        return false;
    }
    if (size != NULL) {
        *size = dir.size;
    }
    return true;
}

/** Fills <list> with the first <size> names of each file in <path> and returns the number of files. */
uint_t filelist(string_t path, uint_t size, char_t **list) {
    assert(path != NULL, "filelist() - dir was NULL!");
    if (!FAT32.mounted) {
        return NOT_FOUND;
    }
    FAT32_directory_t file = FAT32_find(path);
    if (DIRECTORY_END(file)) {
        return 0;
    }
    FAT32_directory_t *start = (FAT32_directory_t *) FAT32_load(DIRECTORY_CLUSTER(file),ALL_CLUSTERS);
    if (start == NULL) {
        return 0;
    }
    FAT32_directory_t *iter = start;
    uint_t count = 0;
    for (; !DIRECTORY_END(*iter) && size > 0; ++iter) {
        if (iter->name[0] == FAT32_DIRECTORY_SKIP ||
            strcompare(iter->name, ".") == EQUAL_TO ||
            strcompare(iter->name, "..") == EQUAL_TO) {
            continue;
        }
        copy(list[count], iter->name, ARRAY_SIZE(iter->name) + ARRAY_SIZE(iter->extension));
        --size;
        ++count;
    }
    free(start);
    return count;
}

/** Mounts the given FAT32 partition as the current hard drive, if possible. */
bool_t mount(ATA_port_t port, byte_t drive, uint_t start) {
    assert(sizeof(FAT32_boot_t) == SECTOR_SIZE, "mount() - FAT32_boot_t is not one sector wide!");
    assert(sizeof(FAT32_FS_t) == SECTOR_SIZE, "mount() - FAT32_FS_t structure is not one sector wide!");
    assert(port == ATA_PRIMARY_PORT || port == ATA_SECONDARY_PORT, "mount() - port was invalid!");
    assert(drive == ATA_MASTER_DRIVE || drive == ATA_SLAVE_DRIVE, "mount() - drive was invalid!");
    FAT32_boot_t boot = {0};
    FAT32_FS_t FS = {0};
    secread(port, drive, start, 1, (char_t *) &boot);
    if (boot.boot_signature == BOOT_SIGNATURE &&
        boot.FAT_size16 == 0 &&
        boot.FAT_size32 > 0) {
    success:
        secread(port, drive, start + boot.FS_info, 1, (char_t *) &FS);
        if (FS.lead_signature == LEAD_SIGNATURE &&
            FS.struct_signature == STRUCT_SIGNATURE &&
            FS.boot_signature == BOOT_SIGNATURE) {
            FAT32.mounted = true;
            FAT32.port = port;
            FAT32.drive = drive;
            FAT32.start = start;
            FAT32.boot = boot;
            FAT32.FS = FS;
            free(FAT32.table);
            FAT32.table = malloc(SECTOR_SIZE * boot.FAT_size32);
            assert(FAT32.table != NULL, "mount() - Could not allocate a FAT!");
            secread(
                port,
                drive,
                start + boot.reserved_count,
                boot.FAT_size32,
                (char_t *) FAT32.table
            );
            return true;
        }
        return false;
    }
    secread(
        port,
        drive,
        start + min(boot.backup, boot.reserved_count),
        1,
        (char_t *) &boot
    );
    if (boot.boot_signature == BOOT_SIGNATURE &&
        boot.FAT_size16 == 0 &&
        boot.FAT_size32 > 0) {
        secwrite(port, drive, start, 1, (char_t *) &boot);
        goto success;
    }
    return false;
}

/** Formats the hard drive for FAT32. */
bool_t format(ATA_port_t port, byte_t drive, uint_t start, uint_t clus_count, uint_t part_size, bool_t force) {
    assert(sizeof(FAT32_boot_t) == SECTOR_SIZE, "format() - FAT32_boot_t is not one sector wide!");
    assert(sizeof(FAT32_FS_t) == SECTOR_SIZE, "format() - FAT32_FS_t structure is not one sector wide!");
    assert(sizeof(FAT32_directory_t) == 32, "format() - FAT32_directory_t structure is not 32 bytes wide!");
    assert(port == ATA_PRIMARY_PORT || port == ATA_SECONDARY_PORT, "format() - port was invalid!");
    assert(drive == ATA_MASTER_DRIVE || drive == ATA_SLAVE_DRIVE, "format() - drive was invalid!");
    assert(clus_count > 0 && (clus_count & (clus_count - 1)) == 0, "format() - clus_count is not power of 2!");
    assert(clus_count <= 64, "format() - cluster size too large for FAT32!");
    // Boot sector
    FAT32_boot_t boot = {0};
    FAT32_FS_t FS = {0};
    if (!force) {
        secread(port, drive, start, 1, (char_t *) &boot);
        if (boot.boot_signature == BOOT_SIGNATURE &&
            boot.FAT_size16 == 0 &&
            boot.FAT_size32 > 0) {
            secread(port, drive, start + boot.FS_info, 1, (char_t *) &FS);
            if (FS.lead_signature == LEAD_SIGNATURE &&
                FS.struct_signature == STRUCT_SIGNATURE &&
                FS.boot_signature == BOOT_SIGNATURE) {
                return false;
            }
        }
        secread(
            port,
            drive,
            start + min(boot.backup, boot.reserved_count),
            1,
            (char_t *) &boot
        );
        if (boot.boot_signature == BOOT_SIGNATURE &&
            boot.FAT_size16 == 0 &&
            boot.FAT_size32 > 0) {
            secwrite(port, drive, start, 1, (char_t *) &boot);
            secread(port, drive, start + boot.FS_info, 1, (char_t *) &FS);
            if (FS.lead_signature == LEAD_SIGNATURE &&
                FS.struct_signature == STRUCT_SIGNATURE &&
                FS.boot_signature == BOOT_SIGNATURE) {
                return false;
            }
        }
    }
    boot.jump[0] = 0xEB;
    boot.jump[1] = 0x58;
    boot.jump[2] = 0x90;
    set(boot.OEM, ' ', ARRAY_SIZE(boot.OEM));
    boot.OEM[0] = 'H';
    boot.OEM[1] = 'L';
    boot.OEM[2] = 'O';
    boot.OEM[3] = 'S';
    boot.sector_size = SECTOR_SIZE;
    boot.cluster_size = clus_count;
    boot.reserved_count = 32;
    boot.FAT_count = 2;
    boot.root_count = 0;
    boot.sector_count16 = 0;
    boot.media = 0xF8;
    boot.FAT_size16 = 0;
    boot.track_size = 63;
    boot.head_count = 255;
    boot.hidden_count = start;
    boot.sector_count32 = part_size;
    boot.flags = 0;
    boot.FS_version = 0;
    boot.root = 2;
    boot.FS_info = 1;
    boot.backup = 6;
    set(boot.reserved1, 0, ARRAY_SIZE(boot.reserved1));
    boot.drive = 0x80;
    boot.reserved2 = 0;
    boot.extended_signature = 0x29;
    boot.id = 0xBADA55;
    set(boot.name, ' ', ARRAY_SIZE(boot.name));
    boot.name[0] = 'H';
    boot.name[1] = 'L';
    boot.name[2] = 'O';
    boot.name[3] = 'S';
    set(boot.type, ' ', ARRAY_SIZE(boot.type));
    boot.type[0] = 'F';
    boot.type[1] = 'A';
    boot.type[2] = 'T';
    boot.type[3] = '3';
    boot.type[4] = '2';
    set(boot.bootloader, 0, ARRAY_SIZE(boot.bootloader));
    boot.boot_signature = BOOT_SIGNATURE;
    // File system sector
    uint_t FAT_size = 1;
    uint_t previous_size = 0;
    uint_t sectors = 0;
    uint_t clusters = 0;
    do {
        previous_size = FAT_size;
        sectors = boot.sector_count32 - boot.reserved_count - (boot.FAT_count * FAT_size);
        clusters = sectors / boot.cluster_size;
        FAT_size = (clusters * 4 + (boot.sector_size - 1)) / boot.sector_size;
    } while (FAT_size != previous_size);
    assert(clusters >= 65525, "format() - Cluster count must be >= 65525!");
    boot.FAT_size32 = FAT_size;
    secwrite(port, drive, start, 1, (char_t *) &boot); // Boot sector
    secwrite(port, drive, start + boot.backup, 1, (char_t *) &boot); // Backup sector
    FS.lead_signature = LEAD_SIGNATURE;
    set(FS.reserved1, 0, ARRAY_SIZE(FS.reserved1));
    FS.struct_signature = STRUCT_SIGNATURE;
    FS.free = 0xFFFFFFFF;
    FS.next = 2;
    set(FS.reserved2, 0, ARRAY_SIZE(FS.reserved2));
    FS.boot_signature = BOOT_SIGNATURE;
    secwrite(port, drive, start + 1, 1, (char_t *) &FS);
    // FAT sectors
    FAT32_cluster_t table[128];
    for (uint_t i = 0; i < boot.FAT_count; ++i) {
        table[0] = FAT32_CLUSTER_RESERVED;
        table[1] = FAT32_CLUSTER_END;
        secwrite(
            port,
            drive,
            start + boot.reserved_count + i * boot.FAT_size32,
            1,
            (char_t *) &table
        );
        table[0] = 0;
        table[1] = 0;
        for (uint_t j = 1; j < boot.FAT_size32; ++j) {
            secwrite(
                port,
                drive,
                start + boot.reserved_count + i * boot.FAT_size32 + j,
                1,
                (char_t *) &table
            );
        }
    }
    uint_t data = start + boot.reserved_count + (boot.FAT_count * boot.FAT_size32);
    uint_t root = data + ((boot.root - 2) * boot.cluster_size);
    byte_t cluster[SECTOR_SIZE] = {0};
    for (uint_t i = 0; i < boot.cluster_size; ++i) {
        secwrite(port, drive, root + i, 1, (char_t *) cluster);
    }
    // Root directory
    mount(port, drive, root);
    FAT32_alloc("HLOS", 0, FAT32_ATTRIBUTE_VOLUME);
    return true;
}

/** Allocates new clusters for the mounted FAT32 instance at <path>. Returns the first cluster entry or NOT_FOUND. */
FAT32_cluster_t FAT32_alloc(string_t path, uint_t size, FAT32_attributes_t attr) {
    assert(path != NULL, "FAT32_alloc() - path was NULL!");
    assert(attr != 0, "FAT32_alloc() - attr was empty!");
    if (!FAT32.mounted) {
        return NOT_FOUND;
    }
    // TODO
    return NOT_FOUND;
}

/** Loads all of <clus> for the mounted FAT32 instance into memory. This pointer must be freed! */
void *FAT32_load(FAT32_cluster_t clus, uint_t max) {
    if (!FAT32.mounted || CLUSTER_END(clus)) {
        return NULL;
    }
    assert(
        clus < ((FAT32.boot.FAT_size32 * FAT32.boot.sector_size) / sizeof(FAT32_cluster_t)),
        "FAT32_load() - Invalid cluster entry!"
    );
    FAT32_cluster_t start = clus;
    uint_t count = 0;
    while (!CLUSTER_END(clus) && count < max) {
        clus = FAT32.table[clus];
        ++count;
    }
    clus = start;
    void *mem = malloc(FAT32.boot.cluster_size * count * FAT32.boot.sector_size);
    if (mem == NULL) {
        return NULL;
    }
    for (uint_t i = 0; i < count; ++i) {
        secread(
            FAT32.port,
            FAT32.drive,
            CLUSTER_TO_SECTOR(clus),
            FAT32.boot.cluster_size,
            ((char_t *) mem) + FAT32.boot.cluster_size * FAT32.boot.sector_size * i
        );
        clus = FAT32.table[clus];
    }
    return mem;
}

/** Returns the directory entry for the file at <path>. name[0] is FAT32_DIRECTORY_END on failure. */
FAT32_directory_t FAT32_find(string_t path) {
    assert(path != NULL, "FAT32_find() - path was NULL!");
    if (!FAT32.mounted) {
        return (FAT32_directory_t){0};
    }
    FAT32_directory_t dir;
    // TODO
    return dir;
}

/** Frees the cluster train for the mounted FAT32 instance at <clus>. Returns whether it was successful. */
bool_t FAT32_free(FAT32_cluster_t clus, bool_t dir) {
    if (!FAT32.mounted || CLUSTER_END(clus)) {
        return false;
    }
    assert(
        clus < ((FAT32.boot.FAT_size32 * FAT32.boot.sector_size) / sizeof(FAT32_cluster_t)),
        "FAT32_free() - Invalid cluster entry!"
    );
    if (dir) {
        FAT32_directory_t *start = (FAT32_directory_t *) FAT32_load(clus,ALL_CLUSTERS);
        if (start == NULL) {
            return false;
        }
        FAT32_directory_t *iter = start;
        for (; !DIRECTORY_END(*iter); ++iter) {
            if (iter->name[0] == FAT32_DIRECTORY_SKIP ||
                strcompare(iter->name, ".") == EQUAL_TO ||
                strcompare(iter->name, "..") == EQUAL_TO) {
                continue;
            }
            FAT32_cluster_t target = DIRECTORY_CLUSTER(*iter);
            if ((iter->attributes & FAT32_ATTRIBUTE_DIRECTORY) != 0) {
                FAT32_free(target, true);
            }
            if ((iter->attributes & FAT32_ATTRIBUTE_FILE) != 0) {
                FAT32_free(target, false);
            }
        }
        free(start);
    }
    while (!CLUSTER_END(clus)) {
        FAT32_cluster_t next = FAT32.table[clus];
        FAT32.table[clus] = FAT32_CLUSTER_FREE;
        clus = next;
    }
    for (uint_t i = 0; i < FAT32.boot.FAT_count; ++i) {
        secwrite(
            FAT32.port,
            FAT32.drive,
            FAT32.start + FAT32.boot.reserved_count + (FAT32.boot.FAT_size32 * i),
            FAT32.boot.FAT_size32,
            (char_t *) FAT32.table
        );
    }
    return true;
}

/** Reads <num> number of 512-byte sectors at <sec> into <str>. */
char_t *secread(ATA_port_t port, byte_t drive, uint_t sec, uint_t num, char_t *str) {
    assert(port == ATA_PRIMARY_PORT || port == ATA_SECONDARY_PORT, "secread() - port was invalid!");
    assert(drive == ATA_MASTER_DRIVE || drive == ATA_SLAVE_DRIVE, "secread() - drive was invalid!");
    assert(str != NULL, "secread() - str was NULL!");
    assert(num > 0, "secread() - num was 0!");
    ATA_port_t drive_port;
    ATA_port_t sector_port;
    ATA_port_t low_port;
    ATA_port_t mid_port;
    ATA_port_t high_port;
    ATA_port_t cmd_port;
    ATA_port_t status_port;
    ATA_port_t data_port;
    if (port == ATA_PRIMARY_PORT) {
        drive_port = ATA_PORT_PRIMARY_DRIVE;
        sector_port = ATA_PORT_PRIMARY_SECTOR_COUNT;
        low_port = ATA_PORT_PRIMARY_LBA_LOW;
        mid_port = ATA_PORT_PRIMARY_LBA_MID;
        high_port = ATA_PORT_PRIMARY_LBA_HIGH;
        cmd_port = ATA_PORT_PRIMARY_CMD;
        status_port = ATA_PORT_PRIMARY_STATUS;
        data_port = ATA_PORT_PRIMARY_DATA;
    } else {
        drive_port = ATA_PORT_SECONDARY_DRIVE;
        sector_port = ATA_PORT_SECONDARY_SECTOR_COUNT;
        low_port = ATA_PORT_SECONDARY_LBA_LOW;
        mid_port = ATA_PORT_SECONDARY_LBA_MID;
        high_port = ATA_PORT_SECONDARY_LBA_HIGH;
        cmd_port = ATA_PORT_SECONDARY_CMD;
        status_port = ATA_PORT_SECONDARY_STATUS;
        data_port = ATA_PORT_SECONDARY_DATA;
    }
    cli();
    out(drive_port, drive | ((sec >> 24) & 15));
    out(sector_port, num);
    out(low_port, sec & 255);
    out(mid_port, (sec >> 8) & 255);
    out(high_port, (sec >> 16) & 255);
    out(cmd_port, ATA_READ);
    sleep(1);
    byte_t status;
    do {
        status = in(status_port);
        assert((status & ATA_ERROR) == 0, "secread() - ATA returned error before read!");
    } while (((status & ATA_BUSY) != 0) || ((status & ATA_REQUEST) == 0));
    num *= (SECTOR_SIZE / 2);
    for (uint_t i = 0; i < num; ++i) {
        ushort_t data = in2(data_port);
        str[i * 2] = data & 255;
        str[(i * 2) + 1] = data >> 8;
    }
    do {
        status = in(status_port);
        assert((status & ATA_ERROR) == 0, "secread() - ATA returned error after read!");
    } while ((status & ATA_BUSY) != 0);
    sti();
    return str;
}

/** Writes <str> into <num> number of 512-byte sectors at <sec>. */
string_t secwrite(ATA_port_t port, byte_t drive, uint_t sec, uint_t num, string_t str) {
    assert(port == ATA_PRIMARY_PORT || port == ATA_SECONDARY_PORT, "secwrite() - port was invalid!");
    assert(drive == ATA_MASTER_DRIVE || drive == ATA_SLAVE_DRIVE, "secwrite() - drive was invalid!");
    assert(str != NULL, "secwrite() - str was NULL!");
    assert(num > 0, "secwrite() - num was 0!");
    ATA_port_t drive_port;
    ATA_port_t sector_port;
    ATA_port_t low_port;
    ATA_port_t mid_port;
    ATA_port_t high_port;
    ATA_port_t cmd_port;
    ATA_port_t status_port;
    ATA_port_t data_port;
    if (port == ATA_PRIMARY_PORT) {
        drive_port = ATA_PORT_PRIMARY_DRIVE;
        sector_port = ATA_PORT_PRIMARY_SECTOR_COUNT;
        low_port = ATA_PORT_PRIMARY_LBA_LOW;
        mid_port = ATA_PORT_PRIMARY_LBA_MID;
        high_port = ATA_PORT_PRIMARY_LBA_HIGH;
        cmd_port = ATA_PORT_PRIMARY_CMD;
        status_port = ATA_PORT_PRIMARY_STATUS;
        data_port = ATA_PORT_PRIMARY_DATA;
    } else {
        drive_port = ATA_PORT_SECONDARY_DRIVE;
        sector_port = ATA_PORT_SECONDARY_SECTOR_COUNT;
        low_port = ATA_PORT_SECONDARY_LBA_LOW;
        mid_port = ATA_PORT_SECONDARY_LBA_MID;
        high_port = ATA_PORT_SECONDARY_LBA_HIGH;
        cmd_port = ATA_PORT_SECONDARY_CMD;
        status_port = ATA_PORT_SECONDARY_STATUS;
        data_port = ATA_PORT_SECONDARY_DATA;
    }
    cli();
    out(drive_port, drive | ((sec >> 24) & 15));
    out(sector_port, num);
    out(low_port, sec & 255);
    out(mid_port, (sec >> 8) & 255);
    out(high_port, (sec >> 16) & 255);
    out(cmd_port, ATA_WRITE);
    sleep(1);
    byte_t status;
    do {
        status = in(status_port);
        assert((status & ATA_ERROR) == 0, "secwrite() - ATA returned error before write!");
    } while (((status & ATA_BUSY) != 0) || ((status & ATA_REQUEST) == 0));
    num *= (SECTOR_SIZE / 2);
    for (uint_t i = 0; i < num; ++i) {
        ushort_t data = str[i * 2] | (str[(i * 2) + 1] << 8);
        out2(data_port, data);
    }
    do {
        status = in(status_port);
        assert((status & ATA_ERROR) == 0, "secwrite() - ATA returned error after write!");
    } while ((status & ATA_BUSY) != 0);
    sti();
    return str;
}
