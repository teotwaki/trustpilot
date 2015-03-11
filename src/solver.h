#ifndef SOLVER_H__
#define SOLVER_H__

#include <json-c/json.h>
#include <openssl/md5.h>
#include <stdbool.h>
#include <string.h>
#include <sys/time.h>

#include "log.h"
#include "client.h"

#define JSON_OBJECT_FREED 1
#define MD5_DIGEST_SIZE 16
#define microseconds(tv) \
	(1000000 * tv.tv_sec + tv.tv_usec)
#define MIN_WORD_LENGTH 2

typedef struct _solver_t solver_t;

struct _solver_t {
	char * seed;
	int seed_length;
	int seed_ord;
	unsigned char * seed_digest;

	char * * words;
	int words_count;

	client_t * client;

	char * current_anagram;
	char * current_word;
	unsigned char * current_digest;
	int anagrams_count;

	char * match;
	char * tmp_first;
	char * tmp_second;
};

// Constructor/destructor
solver_t * solver_init(void * zmq_ctx, char const * endpoint);
int solver_destroy(solver_t * this);

// Setters
void solver_set_seed(solver_t * this, char const * seed);

// Server communications
int solver_initialise_words(solver_t * this);
int solver_next_word(solver_t * this);
int solver_submit_results(solver_t * this, char const * match, int duration);

// Object inspection
int solver_has_current_word(solver_t * this);

// Actual meat
bool exists_in_pool(char const * pool, char const * word);
int count_letters(char const * ptr);
char * lttrdup(char const * str);
void remove_from_pool(char * new_pool, char const * pool, char const * word);
void solver_build_anagrams(solver_t * this, char const * current_pool);
bool solver_is_anagram(solver_t * this);
bool solver_is_plausible_anagram(solver_t * this);
void solver_loop(solver_t * this);

#endif
