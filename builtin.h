#pragma once
#ifndef __BUILTINS_H__
#define __BUILTINS_H__

typedef int(*command_func)(int argc, char **argv);

typedef struct icmd {
	command_func function;
	size_t length;
	char *command;
} internal_command;

#define BUILTIN_DEF(FUNC, CMD) { FUNC, sizeof(CMD), CMD }

extern internal_command builtins[];
int builtins_count();
int empty_input_wrapper(int argc, char **argv);

int jsh_cd(int argc, char **argv);
int jsh_exit(int argc, char **argv);
int jsh_about(int argc, char **argv);
int int_exec(int argc, char **argv);
int ext_exec(int argc, char **argv);

#endif
