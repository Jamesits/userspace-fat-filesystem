#include "fs_commands.h"
#include "fat_file.h"
#include "fat_util.h"
#include "fat_volume.h"
#include <errno.h>

struct fat_volume * volume = NULL;

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
    if (!volume) {
        fprintf(stderr, "Error: There is no mounted volume!\n");
        return -1;
    }
    fat_unmount(volume);
    return 0;
}
