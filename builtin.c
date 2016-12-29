#include "stdafx.h"

internal_command builtins[] = {
        BUILTIN_DEF(jsh_cd, "cd"),
        BUILTIN_DEF(jsh_exit, "exit"),
        BUILTIN_DEF(ext_exec, "call"),
        BUILTIN_DEF(jsh_about, "about"),
};

int builtins_count() { return sizeof(builtins) / sizeof(builtins[0]); }

int jsh_cd(int argc, char **argv)
{
    if (argc == 0)
    {
        fprintf(config.f_err, "cd: Tilde expension not implemented");
    }
    else {
        if (chdir(argv[1]) != 0) {
            perror("cd");
        }
    }
    return 0;
}

int jsh_exit(int argc, char **argv)
{
	config.shuttingdown = true;
	return 0;
}

int jsh_about(int argc, char **argv)
{
	fprintf(config.f_out,
		SHELL_NAME " (" SHELL_SHORT_NAME ")\n"
		"Version " STR(VERSION_MAJOR) "." STR(VERSION_MINOR) "." STR(VERSION_REVISION) "\n"
		SHELL_DESCRIPTION "\n"
        SHELL_URL "\n\nBuiltin commands: "
		);
	if (builtins_count() > 0) fprintf(config.f_out, "%s", builtins[0].command);
	for (int i = 1; i < builtins_count(); ++i)
		fprintf(config.f_out, " %s", builtins[i].command);
	fputc('\n', config.f_out);

	return 0;
}

// this is executed if a line is empty
int empty_input_wrapper(int argc, char **argv)
{
	return 0;
}
