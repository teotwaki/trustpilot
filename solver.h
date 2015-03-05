#ifndef SOLVER_H__
#define SOLVER_H__

#include <string.h>

#include "log.h"
#include "client.h"

typedef struct _solver_t solver_t;

struct _solver_t {
	char * seed;
	char * * words;
	int words_count;
	char * current_word;
	client_t * client;
};

// Constructor/destructor
solver_t * solver_init(char const * endpoint);
int solver_destroy(solver_t * this);

// Words processing
int solver_get_words(solver_t * this);

#endif
