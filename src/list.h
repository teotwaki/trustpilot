#ifndef LIST_H__
#define LIST_H__

#include <stdlib.h>
#include <string.h>

#include "log.h"

#define LIST_INITIAL_SIZE 64

typedef struct _list_t list_t;

struct _list_t {
	char * * anagrams;
	int anagrams_count;
	int capacity;
};

// Constructor/destructor
list_t * list_init(void);
int list_destroy(list_t * this);

// List modifiers
int list_append(list_t * this, char * item);
int list_grow(list_t * this);

#endif
