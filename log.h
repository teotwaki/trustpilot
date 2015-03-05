#ifndef LOG_H__
#define LOG_H__

#include <stdarg.h>
#include <stdio.h>
#include <time.h>

void _log(FILE * fp, char const * tag, char const * filename, int line,
		char const * format, ...);

#define DEBUG(msg) \
	_log(stdout, "DEBUG", __FILE__, __LINE__, "%s", msg)

#define VDEBUG(format, ...) \
	_log(stdout, "DEBUG", __FILE__, __LINE__, format, __VA_ARGS__)

#define INFO(msg) \
	_log(stdout, "INFO", __FILE__, __LINE__, "%s", msg)

#define VINFO(format, ...) \
	_log(stdout, "INFO", __FILE__, __LINE__, format, __VA_ARGS__)

#define WARN(msg) \
	_log(stdout, "WARN", __FILE__, __LINE__, "%s", msg)

#define VWARN(format, ...) \
	_log(stdout, "WARN", __FILE__, __LINE__, format, __VA_ARGS__)

#define ERROR(msg) \
	_log(stderr, "ERROR", __FILE__, __LINE__, "%s", msg)

#define VERROR(format, ...) \
	_log(stderr, "ERROR", __FILE__, __LINE__, format, __VA_ARGS__)

#endif
