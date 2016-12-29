#pragma once
#ifndef __STDAFX_H__
#define __STDAFX_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <wordexp.h>
#include <sys/wait.h>

#ifndef NDEBUG
#  include <stdio.h>
#  include <errno.h>
#  define DEBUG(format, ...)					\
({								\
	int errno_save = errno;					\
	fprintf(stderr, "%s:%u %s(): ",				\
		  __FILE__, __LINE__, __func__);		\
	fprintf(stderr, format, ##__VA_ARGS__);			\
	putc('\n', stderr);					\
	errno = errno_save;					\
})
#else
#  define DEBUG(format, ...)
#endif

#include "config.h"
#include "util.h"
#include "main.h"
#include "jshcore.h"
#include "builtin.h"

#endif
