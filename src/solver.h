#ifndef SOLVER_H__
#define SOLVER_H__

#include <json-c/json.h>
#include <openssl/md5.h>
#include <stdbool.h>
#include <string.h>

#include "list.h"
#include "log.h"
#include "client.h"

#define JSON_OBJECT_FREED 1
#define MD5_DIGEST_SIZE 16

typedef struct _solver_t solver_t;

struct _solver_t {
	char * seed;
	char * * words;
	int words_count;
	char * current_word;
	client_t * client;
	list_t * anagrams;
	unsigned char * digest;
};

// Constructor/destructor
solver_t * solver_init(void * zmq_ctx, char const * endpoint);
int solver_destroy(solver_t * this);

// Server communications
int solver_initialise_words(solver_t * this);
int solver_next_word(solver_t * this);
int solver_submit_anagrams(solver_t * this, char const * * anagrams,
		int anagrams_count);

// Object inspection
int solver_has_current_word(solver_t * this);

// Actual meat
bool exists_in_pool(char const * pool, char const * word);
char * remove_from_pool(char const * pool, char const * word);
int solver_build_anagrams(solver_t * this,
		char const * current_pool, char const * current_anagram);
int solver_loop(solver_t * this);

#endif
