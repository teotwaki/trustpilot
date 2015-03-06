#ifndef LOG_H__
#define LOG_H__

#include <stdarg.h>
#include <stdio.h>
#include <time.h>

void _log(FILE * fp, char const * tag, char const * filename, int line,
		char const * format, ...);

#define _LOG(fp, tag, format, ...) \
	_log(fp, tag, __FILE__, __LINE__, format, __VA_ARGS__)

#define DEBUG(msg) \
	VDEBUG("%s", msg)

#define VDEBUG(format, ...) \
	_LOG(stdout, "DEBUG", format, __VA_ARGS__)

#define INFO(msg) \
	VINFO("%s", msg)

#define VINFO(format, ...) \
	_LOG(stdout, "INFO", format, __VA_ARGS__)

#define WARN(msg) \
	VWARN("%s", msg)

#define VWARN(format, ...) \
	_LOG(stdout, "WARN", format, __VA_ARGS__)

#define ERROR(msg) \
	VERROR("%s", msg)

#define VERROR(format, ...) \
	_LOG(stderr, "ERROR", format, __VA_ARGS__)

#endif
