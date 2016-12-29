#pragma once
#ifndef __CONFIG_H__
#define __CONFIG_H__

#define SHELL_NAME "JamesShell"
#define SHELL_SHORT_NAME "jsh"
#define SHELL_DESCRIPTION "A simple and naive shell implementation by James Swineson."
#define SHELL_URL "https://github.com/Jamesits/JamesShell"

#define VERSION_MAJOR 0
#define VERSION_MINOR 0
#define VERSION_REVISION 3

#define DEFAULT_PS1 "filesystem $ "

#define READ_BUF_SIZE 1024
#define TOKEN_SPLIT_BUF_SIZE 16
#define TOKEN_DELIMETERS " \t\r\n"

#endif /* end of include guard: __CONFIG_H__ */
