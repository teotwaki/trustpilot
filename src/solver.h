#ifndef SOLVER_H__
#define SOLVER_H__

#include <json-c/json.h>
#include <string.h>

#include "log.h"
#include "client.h"

#define JSON_OBJECT_FREED 1

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

// Server communications
int solver_initialise_words(solver_t * this);
int solver_submit_anagrams(solver_t * this, char const * * anagrams,
		int anagrams_count);

#endif
