#include "log.h"

void _log(FILE * fp, char const * tag, char const * filename, int line,
		char const * format, ...) {
	time_t now;
	char iso_buffer[25];
	struct tm * tm_info;
	time(&now);
	tm_info = localtime(&now);
	strftime(iso_buffer, 25, "%FT%T%z", tm_info);

	pthread_t thread = pthread_self();

	fprintf(fp, "%s - [%s] - %s:%d - %p - ", iso_buffer, tag, filename, line,
			(void *)thread);

	va_list ap;
	va_start(ap, format);
	vfprintf(fp, format, ap);
	va_end(ap);

	fprintf(fp, "\n");
}
