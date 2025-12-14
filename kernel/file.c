// .c
// OS File IO Functions
// by Kyle Furey

#include "file.h"
#include "lib.h"
#include "assembly.h"
#include "time.h"

/** Cached data for the mounted File Allocation Table (32-bit). */
FAT32_t FAT = {0};

/**
 * Reads the file at the given path for the given size.
 * Returns a string containing the file's data or NULL if no file was found.
 */
string_t fileread(string_t path, uint_t offset, uint_t size) {
    assert(path != NULL, "fileread() - path was NULL!");
    if (!FAT.mounted) {
        return NULL;
    }
    // TODO
    return NULL;
}

/**
 * Writes the given data to the given file path.
 * Returns whether an existing file was overwritten.
 */
bool_t filewrite(string_t path, string_t data) {
    assert(path != NULL, "filewrite() - path was NULL!");
    if (!FAT.mounted) {
        return false;
    }
    // TODO
    return false;
}

/**
 * Appends the given data to the given file path.
 * Returns whether an existing file was appended to.
 */
bool_t fileappend(string_t path, string_t data) {
    assert(path != NULL, "fileappend() - path was NULL!");
    if (!FAT.mounted) {
        return false;
    }
    // TODO
    return false;
}

/**
 * Moves the file from <start> to <end>.
 * Returns whether an existing file was overwritten at <end>.
 */
bool_t filemove(string_t start, string_t end) {
    assert(start != NULL, "filemove() - start was NULL!");
    assert(end != NULL, "filemove() - end was NULL!");
    if (!FAT.mounted) {
        return false;
    }
    // TODO
    return false;
}

/** Deletes the file at the given path and returns whether a file was erased. */
bool_t filedelete(string_t path) {
    assert(path != NULL, "filedelete() - path was NULL!");
    if (!FAT.mounted) {
        return false;
    }
    // TODO
    return false;
}

/** Returns whether a file exists and writes its size into <size>. */
bool_t filesize(string_t path, uint_t *size) {
    assert(path != NULL, "filesize() - path was NULL!");
    if (!FAT.mounted) {
        return false;
    }
    // TODO
    return false;
}

/** Fills <list> with the first <size> names of each file in <dir> and returns whether it was successful. */
bool_t filelist(string_t dir, uint_t size, char_t **list) {
    assert(dir != NULL, "filelist() - dir was NULL!");
    if (list != NULL) {
        for (uint_t i = 0; i < size; ++i) {
            assert(list[i] != NULL, "filelist() - list had a NULL entry!");
        }
    }
    if (!FAT.mounted) {
        return false;
    }
    // TODO
    return false;
}

/** Mounts the given FAT32 partition as the current hard drive, if possible. */
bool_t mount(ATA_port_t port, byte_t drive, uint_t start) {
    assert(port == ATA_PRIMARY_PORT || port == ATA_SECONDARY_PORT, "mount() - port was invalid!");
    assert(drive == ATA_MASTER_DRIVE || drive == ATA_SLAVE_DRIVE, "mount() - drive was invalid!");
    assert(sizeof(FAT32_boot_t) == SECTOR_SIZE, "mount() - FAT32_boot_t is not one sector wide!");
    assert(sizeof(FAT32_FS_t) == SECTOR_SIZE, "mount() - FAT32_FS_t structure is not one sector wide!");
    FAT32_boot_t boot = {0};
    FAT32_FS_t fs = {0};
    secread(port, drive, start, 1, (char_t *) &boot);
    if (boot.boot_signature == BOOT_SIGNATURE &&
        boot.FAT_size16 == 0 &&
        boot.FAT_size32 > 0) {
    success:
        secread(port, drive, start + boot.FS_info, 1, (char_t *) &fs);
        if (fs.lead_signature == LEAD_SIGNATURE &&
            fs.struct_signature == STRUCT_SIGNATURE &&
            fs.boot_signature == BOOT_SIGNATURE) {
            FAT.mounted = true;
            FAT.port = port;
            FAT.drive = drive;
            FAT.start = start;
            FAT.boot = boot;
            FAT.fs = fs;
            FAT.dir = "/";
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
bool_t format(ATA_port_t port, byte_t drive, uint_t start, uint_t clus_size, uint_t part_size, bool_t force) {
    assert(port == ATA_PRIMARY_PORT || port == ATA_SECONDARY_PORT, "format() - port was invalid!");
    assert(drive == ATA_MASTER_DRIVE || drive == ATA_SLAVE_DRIVE, "format() - drive was invalid!");
    assert(clus_size > 0 && (clus_size & (clus_size - 1)) == 0, "format() - clus_size is not power of 2!");
    assert(sizeof(FAT32_boot_t) == SECTOR_SIZE, "format() - FAT32_boot_t is not one sector wide!");
    assert(sizeof(FAT32_FS_t) == SECTOR_SIZE, "format() - FAT32_FS_t structure is not one sector wide!");
    assert(sizeof(FAT32_table_t) == SECTOR_SIZE, "format() - FAT32_table_t structure is not one sector wide!");
    // Boot sector
    FAT32_boot_t boot = {0};
    FAT32_FS_t fs = {0};
    if (!force) {
        secread(port, drive, start, 1, (char_t *) &boot);
        if (boot.boot_signature == BOOT_SIGNATURE &&
            boot.FAT_size16 == 0 &&
            boot.FAT_size32 > 0) {
            secread(port, drive, start + boot.FS_info, 1, (char_t *) &fs);
            if (fs.lead_signature == LEAD_SIGNATURE &&
                fs.struct_signature == STRUCT_SIGNATURE &&
                fs.boot_signature == BOOT_SIGNATURE) {
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
            secread(port, drive, start + boot.FS_info, 1, (char_t *) &fs);
            if (fs.lead_signature == LEAD_SIGNATURE &&
                fs.struct_signature == STRUCT_SIGNATURE &&
                fs.boot_signature == BOOT_SIGNATURE) {
                return false;
            }
        }
    }
    boot.jump[0] = 0xEB;
    boot.jump[1] = 0x58;
    boot.jump[2] = 0x90;
    set(boot.name, ' ', 8);
    boot.name[0] = 'H';
    boot.name[1] = 'L';
    boot.name[2] = 'O';
    boot.name[3] = 'S';
    boot.sector_size = SECTOR_SIZE;
    boot.cluster_size = clus_size;
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
    set(boot.reserved, 0, ARRAY_SIZE(boot.reserved));
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
    fs.lead_signature = LEAD_SIGNATURE;
    set(fs.reserved1, 0, ARRAY_SIZE(fs.reserved1));
    fs.struct_signature = STRUCT_SIGNATURE;
    fs.free = clusters - 2;
    fs.next = 3;
    set(fs.reserved2, 0, ARRAY_SIZE(fs.reserved2));
    fs.boot_signature = BOOT_SIGNATURE;
    secwrite(port, drive, start + 1, 1, (char_t *) &fs);
    // FAT sectors
    FAT32_table_t table = {0};
    for (uint_t i = 0; i < boot.FAT_count; ++i) {
        table.entries[0] = FAT32_RESERVED;
        table.entries[1] = FAT32_END;
        table.entries[2] = FAT32_END;
        secwrite(
            port,
            drive,
            start + boot.reserved_count + i * boot.FAT_size32,
            1,
            (char_t *) &table
        );
        table.entries[0] = 0;
        table.entries[1] = 0;
        table.entries[2] = 0;
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
    FAT32_directory_t dir = {0};
    date_t current_date = date();
    uint_t current_time = time();
    set(dir.name, ' ', 8);
    dir.name[0] = 'H';
    dir.name[1] = 'L';
    dir.name[2] = 'O';
    dir.name[3] = 'S';
    dir.extension[0] = ' ';
    dir.extension[1] = ' ';
    dir.extension[2] = ' ';
    dir.attributes = FAT32_ATTRIBUTE_VOLUME;
    dir.reserved = 0;
    dir.create_ms = current_time % 60;
    dir.create_time = (current_date.hour << 11) | (current_date.minute << 5) | (current_date.second / 2);
    dir.create_date = ((current_date.year - 1980) << 9) | (current_date.month << 5) | current_date.day;
    dir.open_date = dir.create_date;
    dir.cluster_high = 0;
    dir.write_time = dir.create_time;
    dir.write_date = dir.create_date;
    dir.cluster_low = 0;
    dir.size = 0;
    secwrite(port, drive, root, 1, (char_t *) &dir);
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
