#ifndef LOG_H__
#define LOG_H__

#include <stdarg.h>
#include <stdio.h>
#include <time.h>

void _log(FILE * fp, char const * tag, char const * format, ...);

#define DEBUG(msg) \
	_log(stdout, "DEBUG", "%s", msg)

#define VDEBUG(format, ...) \
	_log(stdout, "DEBUG", format, __VA_ARGS__)

#define INFO(msg) \
	_log(stdout, "INFO", "%s", msg)

#define VINFO(format, ...) \
	_log(stdout, "INFO", format, __VA_ARGS__)

#define WARN(msg) \
	_log(stdout, "WARN", "%s", msg)

#define VWARN(format, ...) \
	_log(stdout, "WARN", format, __VA_ARGS__)

#define ERROR(msg) \
	_log(stderr, "ERROR", "%s", msg)

#define VERROR(format, ...) \
	_log(stderr, "ERROR", format, __VA_ARGS__)

#endif
