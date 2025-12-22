// .c
// OS File IO Functions
// by Kyle Furey

#include "file.h"
#include "lib.h"
#include "assembly.h"
#include "malloc.h"

/** Cached data for the mounted File Allocation Table (32-bit). */
FAT32_cache_t FAT32 = {0};

/**
 * Reads the file at <path> at <offset> for <size> into <file>.
 * Returns whether reading was successful.
 */
bool_t fileread(string_t path, uint_t offset, uint_t size, char_t *file) {
    assert(path != NULL, "fileread() - path was NULL!");
    FAT32_directory_t dir = FAT32_find(path, NULL);
    if (DIRECTORY_END(dir)) {
        return false;
    }
    if (offset >= dir.size) {
        return false;
    }
    FAT32_cluster_t clus = DIRECTORY_CLUSTER(dir);
    uint_t clus_size = FAT32.boot.cluster_size * FAT32.boot.sector_size;
    while (offset >= clus_size) {
        clus = FAT32.table[clus];
        if (CLUSTER_END(clus)) {
            return false;
        }
        offset -= clus_size;
    }
    uint_t max = dir.size - offset;
    size = size < max ? size : max;
    char_t *mem = (char_t *) FAT32_load(clus, ALL_CLUSTERS);
    if (mem == NULL) {
        return false;
    }
    copy(file, mem + offset, size);
    free(mem);
    return true;
}

/**
 * Writes <size> bytes in <data> to the file at <path>.
 * Returns whether writing was successful.
 */
bool_t filewrite(string_t path, uint_t size, string_t data) {
    assert(path != NULL, "filewrite() - path was NULL!");
    uint_t fsize;
    if (filesize(path, &fsize)) {
        char_t *file = (char_t *) malloc(fsize);
        if (file == NULL) {
            return false;
        }
        if (!fileread(path, 0, fsize, file)) {
            free(file);
            return false;
        }
        filedelete(path);
        if (!fileappend(path, size, data)) {
            fileappend(path, fsize, file);
            free(file);
            return false;
        }
        free(file);
        return true;
    }
    return fileappend(path, size, data);
}

/**
 * Appends <size> bytes in <data> to the file at <path>.
 * Returns whether writing was successful.
 */
bool_t fileappend(string_t path, uint_t size, string_t data) {
    assert(path != NULL, "fileappend() - path was NULL!");
    FAT32_directory_t dir = FAT32_find(path, NULL);
    bool_t app = !DIRECTORY_END(dir);
    if (app && (dir.attributes & FAT32_ATTRIBUTE_READONLY) != 0) {
        return false;
    }
    uint_t fsize = app ? dir.size : 0;
    FAT32_attributes_t attr;
    if (strfirst(path, '.') != NOT_FOUND) {
        attr = FAT32_ATTRIBUTE_FILE;
    } else {
        attr = FAT32_ATTRIBUTE_DIRECTORY;
        size = 0;
        data = NULL;
    }
    char_t *file = NULL;
    FAT32_cluster_t new_clus;
    if (fsize + size > 0) {
        file = (char_t *) malloc(fsize + size);
        if (file == NULL) {
            return false;
        }
        if (app) {
            fileread(path, 0, fsize, file);
        }
        append(file, fsize, data, size);
    }
    FAT32_free(path);
    FAT32_directory_t new_dir = FAT32_alloc(path, fsize + size, file, attr, false, NULL);
    if (DIRECTORY_END(new_dir)) {
        FAT32_alloc(path, fsize, file, attr, true, NULL); // Restore the deleted file
        return false;
    }
    free(file);
    return true;
}

/**
 * Moves the file from <start> to <end>.
 * Returns whether moving was successful.
 */
bool_t filemove(string_t dest, string_t src) {
    assert(dest != NULL, "filemove() - end was NULL!");
    assert(src != NULL, "filemove() - start was NULL!");
    uint_t size;
    if (!filesize(src, &size)) {
        return false;
    }
    char_t *data = (char_t *) malloc(size);
    if (data == NULL) {
        return false;
    }
    if (!fileread(src, 0, size, data)) {
        free(data);
        return false;
    }
    if (!filewrite(dest, size, data)) {
        free(data);
        return false;
    }
    filedelete(src);
    free(data);
    return true;
}

/** Deletes the file at <path> and returns whether a file was erased. */
bool_t filedelete(string_t path) {
    assert(path != NULL, "filedelete() - path was NULL!");
    return FAT32_free(path);
}

/** Returns whether a file exists at <path> and writes its size into <size>. */
bool_t filesize(string_t path, uint_t *size) {
    assert(path != NULL, "filesize() - path was NULL!");
    FAT32_directory_t dir = FAT32_find(path, NULL);
    if (DIRECTORY_END(dir)) {
        if (size != NULL) {
            *size = 0;
        }
        return false;
    }
    if (size != NULL) {
        *size = dir.size;
    }
    return true;
}

/**
 * Fills <list> with the first <size> names of each file in <path> and returns the number of files.
 * <list> must be at least FILE_NAME_LEN * <size> characters.
 */
uint_t filelist(string_t path, uint_t size, char_t *list) {
    assert(path != NULL, "filelist() - path was NULL!");
    FAT32_directory_t file = FAT32_find(path, NULL);
    if (DIRECTORY_END(file) || (file.attributes & FAT32_ATTRIBUTE_DIRECTORY) == 0) {
        return 0;
    }
    FAT32_directory_t *start = (FAT32_directory_t *) FAT32_load(DIRECTORY_CLUSTER(file),ALL_CLUSTERS);
    if (start == NULL) {
        return 0;
    }
    FAT32_directory_t *iter = start;
    uint_t count = 0;
    for (iter += 2; !DIRECTORY_END(*iter) && size > 0; ++iter) {
        if (iter->name[0] == FAT32_DIRECTORY_SKIP) {
            continue;
        }
        if (list != NULL) {
            uint_t name_len = strfirst(iter->name, ' ');
            name_len = min((int_t) name_len, ARRAY_SIZE(iter->name));
            uint_t ext_len = strfirst(iter->extension, ' ');
            ext_len = min((int_t) ext_len, ARRAY_SIZE(iter->extension));
            copy(list, iter->name, name_len);
            if (ext_len > 0) {
                list[name_len] = '.';
                copy(list + name_len + 1, iter->extension, ext_len);
                list[name_len + ext_len + 1] = '\0';
            } else {
                list[name_len] = '\0';
            }
            list[FILE_NAME_LEN] = '\0';
            list += FILE_NAME_LEN + 1;
        }
        --size;
        ++count;
    }
    free(start);
    return count;
}

/** Mounts the given FAT32 partition as the current hard drive, if possible. */
bool_t mount(ATA_port_t port, byte_t drive, uint_t start) {
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
            FAT32.table = (FAT32_cluster_t *) calloc(boot.FAT_size32, boot.sector_size);
            assert(FAT32.table != NULL, "mount() - Could not allocate enough to cache the FAT in memory!");
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
    assert(port == ATA_PRIMARY_PORT || port == ATA_SECONDARY_PORT, "format() - port was invalid!");
    assert(drive == ATA_MASTER_DRIVE || drive == ATA_SLAVE_DRIVE, "format() - drive was invalid!");
    assert(clus_count <= 128, "format() - clus_count is too large for FAT32!");
    assert(clus_count > 0 && (clus_count & (clus_count - 1)) == 0, "format() - clus_count is not a power of 2!");
    assert(part_size >= SECTOR_SIZE, "format() - part_size is too small for FAT32!");
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
    boot.media = HARD_MEDIA;
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
    FS.next = 0xFFFFFFFF;
    set(FS.reserved2, 0, ARRAY_SIZE(FS.reserved2));
    FS.boot_signature = BOOT_SIGNATURE;
    secwrite(port, drive, start + 1, 1, (char_t *) &FS);
    // FAT sectors
    FAT32_cluster_t *table = (FAT32_cluster_t *) calloc(boot.FAT_size32, boot.sector_size);
    if (table == NULL) {
        return false;
    }
    table[0] = FAT32_CLUSTER_RESERVED;
    table[1] = FAT32_CLUSTER_END;
    table[boot.root] = FAT32_CLUSTER_END;
    for (uint_t i = 0; i < boot.FAT_count; ++i) {
        secwrite(
            port,
            drive,
            start + boot.reserved_count + (boot.FAT_size32 * i),
            boot.FAT_size32,
            (char_t *) table
        );
    }
    free(table);
    uint_t data = start + boot.reserved_count + (boot.FAT_size32 * boot.FAT_count);
    uint_t root = data + ((boot.root - 2) * boot.cluster_size);
    // Root directory
    FAT32_directory_t *dots = (FAT32_directory_t *) calloc(boot.cluster_size, boot.sector_size);
    for (uint_t i = 0; i < 2; ++i) {
        set(dots[i].name, ' ', ARRAY_SIZE(dots[i].name));
        dots[i].name[0] = '.';
        set(dots[i].extension, ' ', ARRAY_SIZE(dots[i].extension));
        dots[i].attributes = FAT32_ATTRIBUTE_DIRECTORY;
        dots[i].reserved = 0;
        dots[i].create_ms = 0;
        dots[i].create_time = 0;
        dots[i].create_date = 0;
        dots[i].open_date = 0;
        dots[i].write_time = 0;
        dots[i].write_date = 0;
        dots[i].size = 0;
    }
    dots[0].cluster_high = (boot.root >> 16) & 0xFFFF;
    dots[0].cluster_low = boot.root & 0xFFFF;
    dots[1].name[1] = '.';
    dots[1].cluster_high = (boot.root >> 16) & 0xFFFF;
    dots[1].cluster_low = boot.root & 0xFFFF;
    secwrite(port, drive, root, boot.cluster_size, (char_t *) dots);
    free(dots);
    return true;
}

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
) {
    assert(path != NULL, "FAT32_alloc() - path was NULL!");
    assert(strlen(path) <= MAX_STRING_LEN, "FAT32_alloc() - path was too long!");
    assert(attr != 0, "FAT32_alloc() - attr was empty!");
    assert(
        size == 0 || (attr & FAT32_ATTRIBUTE_DIRECTORY) == 0,
        "FAT32_alloc() - Directories must not have data!"
    );
    assert(FAT32.mounted, "FAT32_alloc() - FAT32 was not mounted!");
    assert(DIRECTORY_END(FAT32_find(path, NULL)), "FAT32_alloc() - Cannot reallocate an existing file!");
    // Locate parent directory
    char_t parent_path[MAX_STRING_LEN];
    strcopy(parent_path, path);
    FAT32_cluster_t parent_clus;
    FAT32_directory_t parent_dir;
    uint_t slash = strlast(parent_path, '/');
    if (slash != NOT_FOUND) {
        parent_path[slash] = '\0';
        ++slash;
        parent_dir = FAT32_find(parent_path, &parent_clus);
        if (DIRECTORY_END(parent_dir)) {
            if (!force) {
                if (clus != NULL) {
                    *clus = FAT32_CLUSTER_END;
                }
                return (FAT32_directory_t){0};
            }
            parent_dir = FAT32_alloc(parent_path, 0, NULL, FAT32_ATTRIBUTE_DIRECTORY, true, &parent_clus);
            assert(!DIRECTORY_END(parent_dir), "FAT32_alloc() - Could not recursively allocate missing directories!");
        }
    } else {
        slash = 0;
        parent_dir = FAT32_find("", &parent_clus);
    }
    // Locate next available directory entry
    uint_t clus_size = FAT32.boot.cluster_size * FAT32.boot.sector_size;
    uint_t FAT_total = (FAT32.boot.FAT_size32 * FAT32.boot.sector_size) / sizeof(FAT32_cluster_t);
    FAT32_cluster_t first_dir_clus = DIRECTORY_CLUSTER(parent_dir);
    FAT32_directory_t *array = (FAT32_directory_t *) FAT32_load(first_dir_clus, ALL_CLUSTERS);
    assert(array != NULL, "FAT32_alloc() - Could not allocate enough memory for a directory's cluster chain!");
    FAT32_directory_t *dir_iter = array;
    FAT32_cluster_t clus_iter = first_dir_clus;
    char_t *array_iter = (char_t *) array;
    uint_t dir_count = 2;
    uint_t dir_total = clus_size / sizeof(FAT32_directory_t);
    for (dir_iter += 2; !DIRECTORY_END(*dir_iter); ++dir_iter) {
        if (dir_count % dir_total == 0) {
            clus_iter = FAT32.table[clus_iter];
            array_iter += clus_size;
        }
        if (dir_iter->name[0] == FAT32_DIRECTORY_SKIP) {
            break;
        }
        ++dir_count;
    }
    // Out of directory entries, allocate a new cluster
    if (DIRECTORY_END(*dir_iter)) {
        free(array);
        array = NULL;
        FAT32_cluster_t last_dir_clus = first_dir_clus;
        while (!CLUSTER_END(FAT32.table[last_dir_clus])) {
            last_dir_clus = FAT32.table[last_dir_clus];
        }
        for (FAT32_cluster_t i = 2; i < FAT_total; ++i) {
            if (FAT32.table[i] == FAT32_CLUSTER_FREE) {
                FAT32.table[last_dir_clus] = i;
                FAT32.table[i] = FAT32_CLUSTER_END;
                array = (FAT32_directory_t *) calloc(1, clus_size);
                assert(array != NULL, "FAT32_alloc() - Could not allocate enough memory for a cluster!");
                dir_iter = array;
                clus_iter = i;
                array_iter = (char_t *) array;
                break;
            }
        }
        assert(array != NULL, "FAT32_alloc() - Could not locate a cluster for the new directory!");
    }
    // Parse filename
    string_t name = parent_path + slash;
    uint_t dot = strlast(name, '.');
    uint_t len = (dot == NOT_FOUND) ? strlen(name) : dot;
    len = min((int_t) len, ARRAY_SIZE(dir_iter->name));
    set(dir_iter->name, ' ', ARRAY_SIZE(dir_iter->name));
    for (uint_t i = 0; i < len; ++i) {
        dir_iter->name[i] = name[i];
    }
    set(dir_iter->extension, ' ', ARRAY_SIZE(dir_iter->extension));
    // Calculate minimum number of clusters
    uint_t total_clus;
    if ((attr & FAT32_ATTRIBUTE_DIRECTORY) != 0) {
        total_clus = 1;
    } else {
        if (dot != NOT_FOUND) {
            string_t ext = name + dot + 1;
            uint_t ext_len = min((int_t) strlen(ext), ARRAY_SIZE(dir_iter->extension));
            for (uint_t i = 0; i < ext_len; ++i) {
                dir_iter->extension[i] = ext[i];
            }
        }
        total_clus = max((int_t) ((size + clus_size - 1) / clus_size), 1);
    }
    // Find available clusters
    FAT32_cluster_t first_mem_clus = NOT_FOUND;
    FAT32_cluster_t last_mem_clus = NOT_FOUND;
    uint_t remaining = total_clus;
    for (FAT32_cluster_t i = 2; i < FAT_total; ++i) {
        if (FAT32.table[i] == FAT32_CLUSTER_FREE) {
            if (first_mem_clus == NOT_FOUND) {
                first_mem_clus = i;
            } else {
                FAT32.table[last_mem_clus] = i;
            }
            last_mem_clus = i;
            --remaining;
            if (remaining == 0) {
                break;
            }
        }
    }
    assert(remaining == 0, "FAT32_alloc() - Could not locate enough clusters for the new directory's cluster chain!");
    FAT32.table[last_mem_clus] = FAT32_CLUSTER_END;
    // Build new directory entry
    dir_iter->attributes = attr;
    dir_iter->reserved = 0;
    dir_iter->create_ms = 0;
    dir_iter->create_time = 0;
    dir_iter->create_date = 0;
    dir_iter->open_date = 0;
    dir_iter->cluster_high = (first_mem_clus >> 16) & 0xFFFF;
    dir_iter->write_time = 0;
    dir_iter->write_date = 0;
    dir_iter->cluster_low = first_mem_clus & 0xFFFF;
    // Write memory or . and .. entries to disk
    if ((attr & FAT32_ATTRIBUTE_DIRECTORY) == 0) {
        dir_iter->size = size;
        char_t *mem = (char_t *) calloc(1, clus_size);
        assert(mem != NULL, "FAT32_alloc() - Could not allocate enough memory for a cluster!");
        for (FAT32_cluster_t i = first_mem_clus; !CLUSTER_END(i); i = FAT32.table[i]) {
            uint_t chunk = min((int_t) clus_size, (int_t) size);
            if (data != NULL) {
                copy(mem, data, chunk);
            }
            secwrite(FAT32.port, FAT32.drive, CLUSTER_TO_SECTOR(i), FAT32.boot.cluster_size, mem);
            data += chunk;
            size -= chunk;
        }
        free(mem);
    } else {
        dir_iter->size = 0;
        FAT32_directory_t *dots = (FAT32_directory_t *) calloc(1, FAT32.boot.cluster_size * FAT32.boot.sector_size);
        for (uint_t i = 0; i < 2; ++i) {
            set(dots[i].name, ' ', ARRAY_SIZE(dots[i].name));
            dots[i].name[0] = '.';
            set(dots[i].extension, ' ', ARRAY_SIZE(dots[i].extension));
            dots[i].attributes = FAT32_ATTRIBUTE_DIRECTORY;
            dots[i].reserved = 0;
            dots[i].create_ms = 0;
            dots[i].create_time = 0;
            dots[i].create_date = 0;
            dots[i].open_date = 0;
            dots[i].write_time = 0;
            dots[i].write_date = 0;
            dots[i].size = 0;
        }
        dots[0].cluster_high = (first_mem_clus >> 16) & 0xFFFF;
        dots[0].cluster_low = first_mem_clus & 0xFFFF;
        dots[1].name[1] = '.';
        dots[1].cluster_high = (parent_clus >> 16) & 0xFFFF;
        dots[1].cluster_low = parent_clus & 0xFFFF;
        secwrite(FAT32.port, FAT32.drive, CLUSTER_TO_SECTOR(first_mem_clus), FAT32.boot.cluster_size, (char_t *) dots);
        free(dots);
    }
    // Write directory and FAT table to disk
    secwrite(FAT32.port, FAT32.drive, CLUSTER_TO_SECTOR(clus_iter), FAT32.boot.cluster_size, array_iter);
    for (uint_t i = 0; i < FAT32.boot.FAT_count; ++i) {
        secwrite(
            FAT32.port,
            FAT32.drive,
            FAT32.start + FAT32.boot.reserved_count + (FAT32.boot.FAT_size32 * i),
            FAT32.boot.FAT_size32,
            (char_t *) FAT32.table
        );
    }
    // Return directory and output cluster of new directory
    FAT32_directory_t result_dir = *dir_iter;
    free(array);
    if (clus != NULL) {
        *clus = clus_iter;
    }
    return result_dir;
}

/** Loads all of <clus> for the mounted FAT32 instance into memory. This pointer must be freed! */
void *FAT32_load(FAT32_cluster_t clus, uint_t max) {
    assert(
        clus < ((FAT32.boot.FAT_size32 * FAT32.boot.sector_size) / sizeof(FAT32_cluster_t)),
        "FAT32_load() - Invalid cluster entry!"
    );
    assert(FAT32.mounted, "FAT32_load() - FAT32 was not mounted!");
    assert(clus != FAT32_CLUSTER_END, "FAT32_load() - Cannot load END cluster!");
    assert(clus != FAT32_CLUSTER_BAD, "FAT32_load() - Cannot load BAD cluster!");
    FAT32_cluster_t start = clus;
    uint_t count = 0;
    while (!CLUSTER_END(clus) && count < max) {
        clus = FAT32.table[clus];
        ++count;
    }
    clus = start;
    void *mem = malloc(FAT32.boot.cluster_size * FAT32.boot.sector_size * count);
    if (mem == NULL) {
        return NULL;
    }
    for (uint_t i = 0; i < count; ++i) {
        secread(
            FAT32.port,
            FAT32.drive,
            CLUSTER_TO_SECTOR(clus),
            FAT32.boot.cluster_size,
            ((char_t *) mem) + (FAT32.boot.cluster_size * FAT32.boot.sector_size * i)
        );
        clus = FAT32.table[clus];
    }
    return mem;
}

/**
 * Returns the directory for the file at <path>. <clus> is set to the cluster of the found directory.
 * name[0] is FAT32_DIRECTORY_END on failure.
 */
FAT32_directory_t FAT32_find(string_t path, FAT32_cluster_t *clus) {
    assert(path != NULL, "FAT32_find() - path was NULL!");
    assert(strlen(path) <= MAX_STRING_LEN, "FAT32_find() - path was too long!");
    assert(FAT32.mounted, "FAT32_find() - FAT32 was not mounted!");
    while (path[0] == '/') {
        ++path;
    }
    if (path[0] == '\0') {
        FAT32_directory_t root;
        set(root.name, ' ', ARRAY_SIZE(root.name));
        set(root.extension, ' ', ARRAY_SIZE(root.extension));
        root.attributes = FAT32_ATTRIBUTE_DIRECTORY;
        root.reserved = 0;
        root.create_ms = 0;
        root.create_time = 0;
        root.create_date = 0;
        root.open_date = 0;
        root.cluster_high = (FAT32.boot.root >> 16) & 0xFFFF;
        root.write_time = 0;
        root.write_date = 0;
        root.cluster_low = FAT32.boot.root & 0xFFFF;
        root.size = 0;
        if (clus != NULL) {
            *clus = FAT32.boot.root;
        }
        return root;
    }
    FAT32_directory_t *array = NULL;
    FAT32_cluster_t clus_iter = FAT32.boot.root;
    FAT32_directory_t *dir_iter = NULL;
    FAT32_cluster_t prev_clus = FAT32.boot.root;
    char_t buffer[MAX_STRING_LEN];
    strcopy(buffer, path);
    char_t *path_iter = buffer;
    strlower(path_iter);
    while (true) {
        free(array);
        array = (FAT32_directory_t *) FAT32_load(clus_iter, ALL_CLUSTERS);
        assert(array != NULL, "FAT32_find() - Could not allocate enough memory for a directory's cluster chain!");
        dir_iter = array;
        uint_t slash = strfirst(path_iter, '/');
        if (slash != NOT_FOUND) {
            path_iter[slash] = '\0';
        }
        while (path_iter[0] == '/') {
            ++path_iter;
        }
        if (path_iter[0] == '\0') {
            break;
        }
        for (; !DIRECTORY_END(*dir_iter); ++dir_iter) {
            if (dir_iter->name[0] == FAT32_DIRECTORY_SKIP) {
                continue;
            }
            char_t name[FILE_NAME_LEN];
            uint_t name_len = strfirst(dir_iter->name, ' ');
            name_len = min((int_t) name_len, ARRAY_SIZE(dir_iter->name));
            uint_t ext_len = strfirst(dir_iter->extension, ' ');
            ext_len = min((int_t) ext_len, ARRAY_SIZE(dir_iter->extension));
            copy(name, dir_iter->name, name_len);
            if (ext_len > 0) {
                name[name_len] = '.';
                copy(name + name_len + 1, dir_iter->extension, ext_len);
                name[name_len + ext_len + 1] = '\0';
            } else {
                name[name_len] = '\0';
            }
            strlower(name);
            if (strcompare(path_iter, name) == EQUAL_TO) {
                prev_clus = clus_iter;
                clus_iter = DIRECTORY_CLUSTER(*dir_iter);
                break;
            }
        }
        if (DIRECTORY_END(*dir_iter)) {
            free(array);
            if (clus != NULL) {
                *clus = FAT32_CLUSTER_END;
            }
            return (FAT32_directory_t){0};
        }
        if (slash == NOT_FOUND) {
            break;
        }
        path_iter += slash + 1;
    }
    FAT32_directory_t dir = *dir_iter;
    free(array);
    if (clus != NULL) {
        *clus = prev_clus;
    }
    return dir;
}

/** Frees the cluster chain for the mounted FAT32 instance at <path>. Returns whether it was successful. */
bool_t FAT32_free(string_t path) {
    assert(path != NULL, "FAT32_free() - path was NULL!");
    assert(FAT32.mounted, "FAT32_free() - FAT32 was not mounted!");
    FAT32_cluster_t freed_clus;
    FAT32_directory_t freed_dir = FAT32_find(path, &freed_clus);
    if (DIRECTORY_END(freed_dir)) {
        return false;
    }
    FAT32_cluster_t data_clus = DIRECTORY_CLUSTER(freed_dir);
    if (data_clus == FAT32.boot.root) {
        return false; // Don't delete root
    }
    if ((freed_dir.attributes & FAT32_ATTRIBUTE_DIRECTORY) != 0) {
        FAT32_directory_t *data_dir = (FAT32_directory_t *) FAT32_load(data_clus,ALL_CLUSTERS);
        if (data_dir == NULL) {
            return false;
        }
        FAT32_directory_t *iter = data_dir;
        for (iter += 2; !DIRECTORY_END(*iter); ++iter) {
            if (iter->name[0] == FAT32_DIRECTORY_SKIP) {
                continue;
            }
            char_t buffer[MAX_STRING_LEN];
            strcopy(buffer, path);
            strpush(buffer, '/');
            uint_t len = strlen(buffer);
            uint_t child_len = strfirst(iter->name, ' ');
            if (child_len == NOT_FOUND) {
                child_len = ARRAY_SIZE(iter->name);
            }
            append(buffer, len, iter->name, child_len);
            len += child_len;
            if ((iter->attributes & FAT32_ATTRIBUTE_FILE) != 0) {
                buffer[len] = '.';
                uint_t ext_len = strfirst(iter->extension, ' ');
                if (ext_len == NOT_FOUND) {
                    ext_len = ARRAY_SIZE(iter->extension);
                }
                append(buffer, len, iter->extension, ext_len);
                len += ext_len;
            }
            buffer[len] = '\0';
            FAT32_free(buffer);
        }
        free(data_dir);
    }
    FAT32_directory_t *array = (FAT32_directory_t *) FAT32_load(freed_clus, 1);
    FAT32_directory_t *iter = array;
    while (!DIRECTORY_END(*iter)) {
        if (iter->name[0] == FAT32_DIRECTORY_SKIP) {
            ++iter;
            continue;
        }
        if (DIRECTORY_CLUSTER(freed_dir) == DIRECTORY_CLUSTER(*iter)) {
            iter->name[0] = FAT32_DIRECTORY_SKIP;
            secwrite(
                FAT32.port,
                FAT32.drive,
                CLUSTER_TO_SECTOR(freed_clus),
                FAT32.boot.cluster_size,
                (char_t *) array
            );
            break;
        }
        ++iter;
    }
    free(array);
    while (!CLUSTER_END(data_clus)) {
        FAT32_cluster_t next = FAT32.table[data_clus];
        FAT32.table[data_clus] = FAT32_CLUSTER_FREE;
        data_clus = next;
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
char_t *secread(ATA_port_t port, byte_t drive, uint_t sec, byte_t num, char_t *str) {
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
    in(status_port);
    in(status_port);
    in(status_port);
    in(status_port);
    byte_t status;
    for (uint_t sector = 0; sector < num; ++sector) {
        do {
            status = in(status_port);
            assert((status & ATA_ERROR) == 0, "secread() - ATA returned error during read!");
        } while (((status & ATA_BUSY) != 0) || ((status & ATA_REQUEST) == 0));
        for (uint_t word = 0; word < (SECTOR_SIZE / 2); ++word) {
            uint_t index = (sector * SECTOR_SIZE) + (word * 2);
            ushort_t data = in2(data_port);
            str[index] = data & 255;
            str[index + 1] = data >> 8;
        }
    }
    do {
        status = in(status_port);
        assert((status & ATA_ERROR) == 0, "secread() - ATA returned error after read!");
    } while ((status & ATA_BUSY) != 0);
    sti();
    return str;
}

/** Writes <str> into <num> number of 512-byte sectors at <sec>. */
string_t secwrite(ATA_port_t port, byte_t drive, uint_t sec, byte_t num, string_t str) {
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
    in(status_port);
    in(status_port);
    in(status_port);
    in(status_port);
    byte_t status;
    for (uint_t sector = 0; sector < num; ++sector) {
        do {
            status = in(status_port);
            assert((status & ATA_ERROR) == 0, "secwrite() - ATA returned error during write!");
        } while (((status & ATA_BUSY) != 0) || ((status & ATA_REQUEST) == 0));
        for (uint_t word = 0; word < (SECTOR_SIZE / 2); ++word) {
            uint_t index = (sector * SECTOR_SIZE) + (word * 2);
            ushort_t data = str[index] | (str[index + 1] << 8);
            out2(data_port, data);
        }
    }
    do {
        status = in(status_port);
        assert((status & ATA_ERROR) == 0, "secwrite() - ATA returned error after write!");
    } while ((status & ATA_BUSY) != 0);
    sti();
    return str;
}
