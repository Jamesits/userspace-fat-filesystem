#pragma once
#ifndef __FS_COMMANDS_H__
#define __FS_COMMANDS_H__

#include "stdafx.h"

extern struct fat_volume * volume;

int fs_mount(int argc, char **argv);
int fs_format(int argc, char **argv);
int fs_ls(int argc, char **argv);
int fs_pwd(int argc, char **argv);
int fs_cd(int argc, char **argv);
int fs_mkdir(int argc, char **argv);
int fs_rm(int argc, char **argv);
int fs_touch(int argc, char **argv);
int fs_cat(int argc, char **argv);
int fs_print(int argc, char **argv);
int fs_umount(int argc, char **argv);

#endif
