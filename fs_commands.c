#include "fuse_common.h"
#include "fs_commands.h"
#include "fat_file.h"
#include "fat_util.h"
#include "fat_volume.h"
#include <errno.h>
#include <sys/stat.h>
#include <ctype.h>
#include <string.h>

#define MAX_PATH_LEN 256
struct fat_volume * volume = NULL;
char pwd[MAX_PATH_LEN] = "/";

#define fs_mounted_or_fail() if (!volume) { DEBUG("Error: There is no mounted volume!\n"); return -1; }

static inline void inline_strcpy(char *s1, char *s2)
{
    while ((*s1++ = *s2++));
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
