#include "solver.h"

solver_t * solver_init(char const * endpoint) {
	DEBUG("Initialising new solver.");

	int rc = 0;
	solver_t * this = malloc(sizeof(solver_t));

	if (this == NULL) {
		VERROR("Couldn't allocate %d bytes for solver.",
				sizeof(solver_t));
		return NULL;
	}

	this->seed = NULL;
	this->words = NULL;
	this->words_count = 0;
	this->current_word = NULL;

	this->client = client_init(endpoint);

	if (this->client == NULL) {
		ERROR("Couldn't initialise client_t instance.");
		solver_destroy(this);
		return NULL;
	}

	rc = solver_get_words(this);

	if (rc != 0) {
		ERROR("Couldn't retrieve words.");
		solver_destroy(this);
		return NULL;
	}

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

int solver_get_words(solver_t * this) {
	DEBUG("Retrieving words from server.");

	int rc = 0;

	rc = client_send(this->client, CLIENT_GET_WORDS);

	if (rc != 0) {
		ERROR("Couldn't send message to server.");
		return -1;
	}

	json_object * object = client_recv_json(this->client);

	if (object == NULL) {
		ERROR("Couldn't receive JSON message.");
		return -2;
	}

	json_object_object_foreach(object, key, val) {
		if (strcmp(key, "seed") == 0)
			this->seed = strdup(json_object_get_string(val));

		else {
			if (this->words != NULL)
				for (int i = 0; i < this->words_count; i++)
					free(this->words[i]);

			this->words_count = json_object_array_length(val);

			this->words = malloc(sizeof(char *) * this->words_count);
			for (int i = 0; i < this->words_count; i++)
				this->words[i] = strdup(
						json_object_get_string(
							json_object_array_get_idx(val, i)));
		}
	}

	json_object_put(object);

	VDEBUG("Received %d words.", this->words_count);

	return 0;
}
