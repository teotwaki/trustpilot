#include "solver.h"

solver_t * solver_init(char const * endpoint) {
	DEBUG("Initialising new solver.");

	solver_t * this = malloc(sizeof(solver_t));

	if (this == NULL) {
		VERROR("Couldn't allocate %d bytes for solver.",
				sizeof(solver_t));
		return NULL;
	}

	this->client = client_init(endpoint);

	if (this->client == NULL) {
		ERROR("Couldn't initialise client_t instance.");
		return NULL;
	}

	this->seed = NULL;
	this->words = NULL;
	this->words_count = 0;
	this->current_word = NULL;

	return this;
}

int solver_destroy(solver_t * this) {
	DEBUG("Destroying solver.");

	if (this->seed != NULL) {
		free(this->seed);
		this->seed = NULL;
	}

	if (this->words != NULL) {
		for (int i = 0; i < this->words_count; i++) {
			if (this->words[i] != NULL) {
				free(this->words[i]);
				this->words[i] = NULL;
			}
		}

		free(this->words);
		this->words = NULL;
	}

	if (this->current_word != NULL) {
		free(this->current_word);
		this->current_word = NULL;
	}

	if (this->client != NULL) {
		client_destroy(this->client);
		this->client = NULL;
	}

	free(this);

	return 0;
}
