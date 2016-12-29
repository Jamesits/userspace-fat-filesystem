#pragma once
#ifndef __JSH_H__
#define __JSH_H__

char *freadline(FILE *src);
char **tokenize_line(char *line);
int exec(char **tokens);

#endif
