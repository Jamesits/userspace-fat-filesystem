#include "fuse_common.h"
#include "fs_commands.h"
#include "fat_file.h"
#include "fat_util.h"
#include "fat_volume.h"
#include <errno.h>
#include <sys/stat.h>
#include <ctype.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#define MAX_PATH_LEN 256
struct fat_volume * volume = NULL;
char pwd[MAX_PATH_LEN] = "/";

#define fs_mounted_or_fail() if (!volume) { DEBUG("Error: There is no mounted volume!\n"); return -1; }

static inline void inline_strcpy(char *s1, char *s2)
{
    while ((*s1++ = *s2++));
}

/* Create a new disk image */
int fs_create(int argc, char **argv)
{
    char *mem = malloc(10485760); // this is 10MB space
    
    // write boot sector
    struct fat_boot_sector_disk *bootsec = (struct fat_boot_sector_disk *)mem;
    bootsec->jump_insn[0] = 0xEB;
    bootsec->jump_insn[1] = 0x3C;
    bootsec->jump_insn[2] = 0x90;
    memcpy(bootsec->oem_name, "JAMESFAT", 8); // must be 8 bytes, append 0x20 if less
    bootsec->bytes_per_sector = 512;
    bootsec->sectors_per_cluster = 2;
    bootsec->reserved_sectors = 1;
    bootsec->num_tables = 2;
    bootsec->max_root_entries = 512;
    bootsec->total_sectors = 20480;
    bootsec->media_descriptor = 0xF0;
    bootsec->sectors_per_fat = 40;
    bootsec->sectors_per_track = 32;
    bootsec->num_heads = 16;
    bootsec->hidden_sectors = 0;
    bootsec->ebpb.nonfat32_ebpb.physical_drive_num = 0;
    bootsec->ebpb.nonfat32_ebpb.extended_boot_sig = 41;
    bootsec->ebpb.nonfat32_ebpb.volume_id = 0; // 4 byte ID
    memcpy(bootsec->ebpb.nonfat32_ebpb.volume_label, "JAMESFAT   ", 11);
    memcpy(bootsec->ebpb.nonfat32_ebpb.fs_type, "FAT16   ", 8);

    // this is some boot program
    const char code[] = "\xfa\x31\xc0\x8e\xd0\xbc\x00\x7c\xfb\x8e\xd8\xe8\x00\x00\x5e\x83\xc6\x19\xbb\x07\x00\xfc\xac\x84\xc0\x74\x06\xb4\x0e\xcd\x10\xeb\xf5\x30\xe4\xcd\x16\xcd\x19\x0d\x0aThis program cannot be run in DOS mode\x0d\x0aPress any key to reboot\x0d\x0a";

    strcpy(mem + 62, code);
    // boot sector end at 512Bytes
    mem[0x1FE] = 0x55;
    mem[0x1FF] = 0xAA;

    FILE * file= fopen(argv[1], "wb");
    if (file != NULL) {
        fwrite(mem, 10485760, 1, file);
        fclose(file);
    }
    free(mem);
    return 0;
}

int fs_mount(int argc, char **argv)
{
    if (volume) {
        fprintf(stderr, "Warning: replacing last mounted image...\n");
        fat_unmount(volume);
    }
    volume = fat_mount(argv[1], FAT_MOUNT_FLAG_READWRITE);
    if (!volume) {
        fprintf(stderr, "Error: mount failed.\n");
    }
    return 0;
}

int fs_umount(int argc, char **argv)
{
    fs_mounted_or_fail();
    fat_unmount(volume);
    volume = NULL;
    return 0;
}

int fs_format(int argc, char **argv)
{
    fs_mounted_or_fail();
    // set all sectors as free
    u16 *fat_table = volume->fat_map;
    for (int i = 0; i < volume->bytes_per_sector; ++i) {
        fat_table[i] = 0;
    }
    
    // find root entry location
    size_t root_pos = volume->data_start_offset - volume->max_root_entries * sizeof(struct fat_dir_entry_disk);
    DEBUG("Root directory start position: %zu", root_pos);
    long pos = lseek(volume->fd, root_pos, SEEK_SET);
    DEBUG("Seeked to position %ld", pos);
    
    // try add something
    struct fat_dir_entry_disk rootdir;
    memcpy(rootdir.base_name, "TESTFOLD", 8);
    memcpy(rootdir.extension, "TXT", 3);
    rootdir.attribs = 0;
    rootdir.reserved = 0;
    rootdir.create_date = 0;
    rootdir.create_time = 0;
    rootdir.create_time_fine_res = 0;
    rootdir.last_access_date = 0;
    rootdir.file_access_bitmap = 0;
    rootdir.last_modified_date = 0;
    rootdir.last_modified_time = 0;
    rootdir.start_cluster = 2;
    rootdir.file_size = 256;
    write(volume->fd, &rootdir, sizeof(rootdir));

    return 0;
}

char get_file_type_char(mode_t m) {
    if (S_ISBLK(m)) return 'b';
    if (S_ISCHR(m)) return 'c';
    if (S_ISDIR(m)) return 'd';
    if (S_ISFIFO(m)) return 'f';
    if (S_ISREG(m)) return '-';
    if (S_ISLNK(m)) return 'l';
    // error
    return 'e';
}

int fs_ls(int argc, char **argv)
{
    fs_mounted_or_fail();
    char path[MAX_PATH_LEN] = {0};
	struct fat_file *file;

    // == get absolute path
    DEBUG("dest: %s", argv[1]);

    // note about inline_strcpy: use standard strcpy will trigger a out of bound detection
    if (!argv[1]) {
        // empty or invalid path
        DEBUG("got invalid path");
        inline_strcpy(path, pwd);
    } else if (argv[1][0] == '/') {
        // absolute path
        DEBUG("got absolute path length=%lu", strlen(argv[1]));
        inline_strcpy(path, argv[1]);
    } else {
        // relative path
        DEBUG("got relative path length=%lu", strlen(argv[1]));
        inline_strcpy(path, pwd);
        inline_strcpy(path + strlen(pwd), argv[1]);
    }

    DEBUG("final path: %s", path);

    // == end get absolute path

	file = fat_pathname_to_file(volume, path);
    if (!file) {
        DEBUG("%s not found", path);
        return -errno;
    }
    if (!fat_file_is_directory(file)) {
        DEBUG("%s is not a directory", path);
        return -ENOTDIR;

    }
	fat_file_inc_num_times_opened(file);

	struct fat_file *child;
    struct stat *attr = malloc(sizeof(struct stat));

    // TODO: replace `.` and `..`

	if (!file->children_read)
		if (fat_dir_read_children(file))
			return -errno;
	fat_dir_for_each_child(child, file) {
        fat_file_to_stbuf(child, attr);
        printf("%c\t%lldb\t%s\n", get_file_type_char(attr->st_mode), attr->st_size, child->dentry.name);
    }
    free(attr);
	fat_file_dec_num_times_opened(file);
	return 0;
}

int fs_cd(int argc, char **argv)
{
    fs_mounted_or_fail();
    struct fat_file *file;
    char path[MAX_PATH_LEN] = {0};

    // == get absolute path
    DEBUG("dest: %s", argv[1]);

    // note about inline_strcpy: use standard strcpy will trigger a out of bound detection
    if (!argv[1]) {
        // empty or invalid path
        DEBUG("got invalid path");
        inline_strcpy(path, pwd);
    } else if (argv[1][0] == '/') {
        // absolute path
        DEBUG("got absolute path length=%lu", strlen(argv[1]));
        inline_strcpy(path, argv[1]);
    } else {
        // relative path
        DEBUG("got relative path length=%lu", strlen(argv[1]));
        inline_strcpy(path, pwd);
        if (path[strlen(path) - 1] != '/') {
            path[strlen(path)] = '/';
            inline_strcpy(path + strlen(pwd) + 1, argv[1]);
        } else {
            inline_strcpy(path + strlen(pwd), argv[1]);
        }
    }

    DEBUG("final path: %s", path);

    // == end get absolute path

    file = fat_pathname_to_file(volume, path);
    if (!file) {
        DEBUG("%s not found", path);
        fprintf(stderr, "Wrong directory %s", path);
        return -errno;
    }
    if (!fat_file_is_directory(file)) {
        DEBUG("%s is not a directory", path);
        fprintf(stderr, "%s is not a directory", path);
        return -ENOTDIR;

    }
    inline_strcpy(pwd, path);

    return 0;
}

int fs_pwd(int argc, char **argv)
{
    fs_mounted_or_fail();
    puts(pwd);
    return 0;
}

int fs_cat(int argc, char **argv)
{
    struct fat_file *file;
    char path[MAX_PATH_LEN] = {0};

    // == get absolute path
    DEBUG("dest: %s", argv[1]);

    // note about inline_strcpy: use standard strcpy will trigger a out of bound detection
    if (!argv[1]) {
        // empty or invalid path
        DEBUG("got invalid path");
        inline_strcpy(path, pwd);
    } else if (argv[1][0] == '/') {
        // absolute path
        DEBUG("got absolute path length=%lu", strlen(argv[1]));
        inline_strcpy(path, argv[1]);
    } else {
        // relative path
        DEBUG("got relative path length=%lu", strlen(argv[1]));
        inline_strcpy(path, pwd);
        if (path[strlen(path) - 1] != '/') {
            path[strlen(path)] = '/';
            inline_strcpy(path + strlen(pwd) + 1, argv[1]);
        } else {
            inline_strcpy(path + strlen(pwd), argv[1]);
        }
    }

    DEBUG("final path: %s", path);

    // == end get absolute path

    file = fat_pathname_to_file(volume, path);
    if (!file)
        return -errno;
    if (fat_file_is_directory(file))
        return -EISDIR;
    if (file->num_times_opened == 0)
        if (fat_file_alloc_cluster_cache(file))
            return -errno;
    struct stat *attr = malloc(sizeof(struct stat));
    fat_file_to_stbuf(file, attr);
    fat_file_inc_num_times_opened(file);

    char *buf = malloc(attr->st_size + 1);
    fat_file_pread(file, buf, attr->st_size, 0);

    fat_file_dec_num_times_opened(file);
    if (file->num_times_opened == 0)
        fat_file_free_cluster_cache(file);

    for (int i = 0; i < attr->st_size; ++i) putchar(buf[i]);
    printf("\n");

    free(buf);
    free(attr);
    return 0;
}
