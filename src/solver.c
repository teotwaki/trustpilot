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
	this->anagrams = NULL;

	this->client = client_init(endpoint);

	if (this->client == NULL) {
		ERROR("Couldn't initialise client_t instance.");
		solver_destroy(this);
		return NULL;
	}

	rc = solver_initialise_words(this);

	if (rc != 0) {
		ERROR("Couldn't retrieve words.");
		solver_destroy(this);
		return NULL;
	}

	rc = solver_next_word(this);

	if (rc != 0) {
		ERROR("Couldn't initialise solver_t::current_word.");
		solver_destroy(this);
		return NULL;
	}

	this->anagrams = list_init();

	if (this->anagrams == NULL) {
		ERROR("Couldn't initialise solver_t::anagrams.");
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

	if (this->anagrams != NULL) {
		list_destroy(this->anagrams);
		this->anagrams = NULL;
	}

	free(this);

	return 0;
}

int solver_initialise_words(solver_t * this) {
	DEBUG("Retrieving words from server.");

	int rc = 0;

	rc = client_send(this->client, CLIENT_INITIALISE);

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
		if (strcmp(key, "seed") == 0) {
			if (this->seed != NULL) {
				WARN("solver_t's seed was not empty before calling "
						"initialise_words. Releasing memory.");
				free(this->seed);
			}
			this->seed = strdup(json_object_get_string(val));
		}

		else {
			if (this->words != NULL) {
				WARN("solver_t's words list was not empty before calling "
						"initialise_words. Releasing memory.");
				for (int i = 0; i < this->words_count; i++)
					free(this->words[i]);
				free(this->words);
				this->words = NULL;
				this->words_count = 0;
			}

			this->words_count = json_object_array_length(val);

			this->words = malloc(sizeof(char *) * this->words_count);
			for (int i = 0; i < this->words_count; i++)
				this->words[i] = strdup(
						json_object_get_string(
							json_object_array_get_idx(val, i)));
		}
	}

	rc = json_object_put(object);

	if (rc != JSON_OBJECT_FREED) {
		ERROR("JSON object was not freed.");
		return -3;
	}

	VDEBUG("Received %d words.", this->words_count);

	return 0;
}

int solver_next_word(solver_t * this) {
	DEBUG("Retrieving new word from server.");

	int rc = 0;

	rc = client_send(this->client, CLIENT_NEXT_WORD);

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
		if (strcmp(key, "word") == 0) {
			if (this->current_word != NULL) {
				free(this->current_word);
				this->current_word = NULL;
			}

			if (!json_object_is_type(val, json_type_null))
				this->current_word = strdup(json_object_get_string(val));
		}
	}

	rc = json_object_put(object);

	if (rc != JSON_OBJECT_FREED) {
		ERROR("JSON object was not freed.");
		return -3;
	}

	VDEBUG("Received new word to process: %s", this->current_word);

	return 0;
}

int solver_submit_anagrams(solver_t * this, char const * * anagrams,
		int anagrams_count)
{
	json_object * object = json_object_new_object();
	int rc = 0;

	if (object == NULL) {
		ERROR("Couldn't instantiate JSON object.");
		return -1;
	}

	{
		json_object_object_add(object, "type",
				json_object_new_string("found_anagrams"));

		json_object * array = json_object_new_array();

		for (int i = 0; i < anagrams_count; i++) {
			json_object_array_add(array,
					json_object_new_string(anagrams[i]));
		}

		json_object_object_add(object, "result", array);
	}

	rc = client_send(this->client,
			json_object_get_string(object));

	if (rc != 0) {
		ERROR("Couldn't send JSON message");
		return -2;
	}

	rc = json_object_put(object);

	if (rc != JSON_OBJECT_FREED) {
		ERROR("JSON object was not freed.");
		return -3;
	}

	rc = client_recv_ign(this->client);

	if (rc != 0) {
		ERROR("Received invalid server reply.");
		return -4;
	}

	return 0;
}

int solver_has_current_word(solver_t * this) {
	return this->current_word != NULL;
}

int solver_find_anagrams(solver_t * this) {
	if (this->anagrams != NULL) {
		for (int i = 0; i < this->anagrams->anagrams_count; i++)
			VDEBUG("Anagram: %s", list_get(this->anagrams, i));
	}

	return 0;
}

int solver_loop(solver_t * this) {
	int rc = 0;
	int anagrams = 0;
	int total_anagrams = 0;

	while (solver_has_current_word(this)) {
		anagrams = solver_find_anagrams(this);
		total_anagrams += anagrams;
	}

	return rc;
}
