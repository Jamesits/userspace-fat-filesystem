#include "fuse_common.h"
#include "fs_commands.h"
#include "fat_file.h"
#include "fat_util.h"
#include "fat_volume.h"
#include <errno.h>
#include <sys/stat.h>

struct fat_volume * volume = NULL;

#define fs_mounted_or_fail() if (!volume) { DEBUG("Error: There is no mounted volume!\n"); return -1; }

int fs_mount(int argc, char **argv)
{
    if (volume) {
        fprintf(stderr, "Warning: replacing last mounted image...\n");
        fat_unmount(volume);
    }
    volume = fat_mount(argv[1], 0);
    if (!volume) {
        fprintf(stderr, "Error: mount failed.\n");
    }
    return 0;
}

int fs_umount(int argc, char **argv)
{
    fs_mounted_or_fail();
    fat_unmount(volume);
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
    char *path;
	struct fat_file *file;

    // TODO: handle working directory
    path = argv[1];

    DEBUG("path: %s\n", path);
	file = fat_pathname_to_file(volume, path);
	if (!file)
		return -errno;
	if (!fat_file_is_directory(file))
		return -ENOTDIR;
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
	fat_file_dec_num_times_opened(file);
	return 0;
}
