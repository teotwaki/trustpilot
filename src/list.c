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
	this->list_size = 0;

	return this;
}

int list_destroy(list_t * this) {
	INFO("Destroying anagram list.");

	if (this->anagrams != NULL) {
		list_foreach(this, anagram) {
			if (anagram != NULL) {
				free(anagram);
				anagram = NULL;
			}
		}

		free(this->anagrams);
		this->anagrams = NULL;
	}

	return 0;
}

int list_append(list_t * this, char * item) {
	int rc = 0;

	if (this->anagrams_count >= this->list_size) {
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
	int new_size = 0;

	if (this->anagrams == NULL)
		new_size = sizeof(char *) * LIST_INITIAL_SIZE;
	else
		new_size = sizeof(char *) * this->list_size * 2;

	char * * new_anagrams = malloc(new_size);

	if (new_anagrams == NULL) {
		VERROR("Couldn't allocate %d bytes to grow list.", new_size);
		return -1;
	}

	memset(new_anagrams, 0, new_size);

	if (this->anagrams != NULL) {
		memcpy(new_anagrams, this->anagrams, sizeof(char * ) * this->list_size);
		free(this->anagrams);
	}

	this->anagrams = new_anagrams;
	this->list_size = new_size;

	return 0;
}
