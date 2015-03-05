#ifndef SOLVER_H__
#define SOLVER_H__

typedef struct _solver_t solver_t;

struct _solver_t {
	char const * seed;
	char const * const * words;
	int words_count;
	char const * current_word;
};

#endif
