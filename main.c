#include "stdafx.h"

shared_config config;

int main(int argc, char **argv)
{
	// load config
	initialize_config(&config);
	// load rc files
	load_script("/etc/jshrc");
	load_script(".jsh_profile");
	load_script(".jshrc");
	// process_args
	if (argc > 1)
	{
		// enter scripting mode
		for (int i = 1; i < argc; ++i)
			if (load_script(argv[i]))
				fprintf(config.f_err, "jsh: cannot execute script %s\n", argv[i]);
	} else {
		// enter REPL
		jsh_repl(config.f_in, false);
	}
	return EXIT_SUCCESS;
}

void initialize_config(shared_config *c)
{
	c->f_in = stdin;
	c->f_out = stdout;
	c->f_err = stderr;
	c->shuttingdown = false;
	c->last_return_value = 0;
}

int load_script(char *file)
{
	if(access(file, R_OK | X_OK) != -1)
	{
		FILE *script = fopen(file, "r");
		if (!script)
			// file not found
			return -1;
		jsh_repl(script, true);
		fclose(script);
	} else if(access(file, R_OK) != -1)
	{
		// file not found
		return -1;
	} else {
		// no execution bit
		return -2;
	}
	return EXIT_SUCCESS;
}

void jsh_repl(FILE *input, bool quiet)
{
	// REPL loop;
	do {
		if (!quiet) printf(DEFAULT_PS1);
		char *line = freadline(input);
		if (!line)
		{
			config.shuttingdown = true;
			continue;
		}
		// ignore empty line
		if (strlen(line) == 0) goto cont;
		// fprintf(stderr, "[input stream]Get line: %s\n", line);
		char **token = tokenize_line(line);
		// for (int i = 0; token[i]; ++i)
		// 	fprintf(stderr, "[tokenizer]Token #%d: %s\n", i, token[i]);
		// fprintf(stderr, "\n");
		if (token)
		{

			exec(token);
			// check return status;
			// fprintf(stderr, "Return value: %d\n", config.last_return_value);
			free(token);
			token = NULL;
		} else {
			fprintf(config.f_err, "jsh: tokenizer fatal\n");
		}
		cont:
		free(line);
	} while(!config.shuttingdown);
}
