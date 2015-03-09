#include "list.h"

list_t * list_init() {
	INFO("Initialising new anagram list");

	list_t * this = malloc(sizeof(list_t));

	if (this == NULL) {
		VERROR("Couldn't allocate list_t instance (%d bytes).",
				sizeof(list_t));
		return NULL;
	}

	this->anagrams = NULL;
	this->anagrams_count = 0;
	this->capacity = 0;

	return this;
}

int list_destroy(list_t * this) {
	INFO("Destroying anagram list.");

	list_reset(this);

		free(this->anagrams);
		this->anagrams = NULL;
	}

	free(this);

	return 0;
}

char * list_get(list_t * this, int idx) {
	if (this->capacity > 0 && idx < this->anagrams_count)
		return this->anagrams[idx];

	return NULL;
}

int list_append(list_t * this, char * item) {
	int rc = 0;

	if (this->anagrams_count >= this->capacity) {
		rc = list_grow(this);

		if (rc != 0) {
			ERROR("No space left in list.");
			return -1;
		}
	}

	this->anagrams[this->anagrams_count++] = item;

	return 0;
}

int list_grow(list_t * this) {
	int new_size, new_capacity = 0;

	if (this->anagrams == NULL)
		new_capacity = LIST_INITIAL_SIZE;
	else
		new_capacity = this->capacity * 2;

	new_size = new_capacity * sizeof(char *);

	VDEBUG("Growing list capacity up to %d items (previously %d).",
			new_capacity, this->capacity);

	char * * new_anagrams = malloc(new_size);

	if (new_anagrams == NULL) {
		VERROR("Couldn't allocate %d bytes to grow list.", new_size);
		return -1;
	}

	memset(new_anagrams, 0, new_size);

	if (this->anagrams != NULL) {
		memcpy(new_anagrams, this->anagrams, this->capacity * sizeof(char *));
		free(this->anagrams);
	}

	this->anagrams = new_anagrams;
	this->capacity = new_capacity;

	return 0;
}

void list_reset(list_t * this) {
	if (this->anagrams != NULL) {
		for (int i = 0; i < this->anagrams_count; i++)
			if (this->anagrams[i] != NULL) {
				free(this->anagrams[i]);
				this->anagrams[i] = NULL;
			}
		this->anagrams_count = 0;
	}
}
