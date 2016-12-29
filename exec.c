#include "stdafx.h"

int int_exec(int argc, char **argv)
{
	if (!argc)
	{
		empty_input_wrapper(argc, argv);
		return 1;
	}
	for (int i = 0; i < builtins_count(); ++i)
		if (!strcmp(argv[0], builtins[i].command))
		{
			config.last_return_value = (builtins[i].function)(argc, argv);
			return 1;
		}
	return 0;
}

int ext_exec(int argc, char **argv)
{
	pid_t pid, wpid;
	int status;

	pid = fork();
	if (pid == 0)
	{
		// Child process
		if (execvp(argv[0], argv) == -1)
		{
			perror(argv[0]);
		}
		exit(EXIT_FAILURE);
	}
	else if (pid < 0)
	{
		// Error forking
		perror("jsh");
	}
	else
	{
		// Parent process
		do {
			wpid = waitpid(pid, &status, WUNTRACED);
		} while (!WIFEXITED(status) && !WIFSIGNALED(status));
		config.last_return_value = WEXITSTATUS(status);
	}
	return 0;
}
