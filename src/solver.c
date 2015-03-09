#include "solver.h"

solver_t * solver_init(void * zmq_ctx, char const * endpoint) {
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
	this->digest = NULL;

	this->client = client_init(zmq_ctx, endpoint);

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

	if (this->digest != NULL) {
		free(this->digest);
		this->digest = NULL;
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

		else if (strcmp(key, "hash") == 0) {
			if (this->digest != NULL) {
				WARN("solver_t's digest was not empty before calling "
						"initialise_words. Releasing memory.");
				free(this->digest);
			}

			this->digest = malloc(sizeof(unsigned char) * MD5_DIGEST_SIZE);
			unsigned char * digest_iter = this->digest;
			unsigned int tmp = 0;

			for (char const * input_iter = json_object_get_string(val);
					*input_iter != '\0';
					input_iter += 2)
			{
				sscanf(input_iter, "%02x", &tmp);
				*digest_iter++ = tmp;
			}

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

			if (!json_object_is_type(val, json_type_null)) {
				this->current_word = strdup(
						json_object_get_string(val));
				VDEBUG("Received new word to process: %s",
						this->current_word);
			}
			else
				INFO("Server sent NULL. Nothing more to do.");
		}
	}

	rc = json_object_put(object);

	if (rc != JSON_OBJECT_FREED) {
		ERROR("JSON object was not freed.");
		return -3;
	}

	return 0;
}

int solver_submit_results(solver_t * this, char const * match)
{
	json_object * object = json_object_new_object();
	int rc = 0;

	if (object == NULL) {
		ERROR("Couldn't instantiate JSON object.");
		return -1;
	}

	{
		json_object_object_add(object, "type",
				json_object_new_string("results"));

		if (match != NULL)
			json_object_object_add(object, "match",
					json_object_new_string(match));
		else
			json_object_object_add(object, "match", NULL);

		json_object_object_add(object, "anagrams_count",
				json_object_new_int(this->anagrams->anagrams_count));

		json_object_object_add(object, "current_word",
				json_object_new_string(this->current_word));
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

bool exists_in_pool(char const * pool, char const * word) {
	char * tmp_pool = strdup(pool);
	char * match = NULL;

	while (*word != '\0') {
		if (*word == ' ') {
			word++;
			continue;
		}

		match = strchr(tmp_pool, *word);

		if (match != NULL) {
			*match = '.';
			word++;
		}

		else
			break;
	}

	free(tmp_pool);

	return *word == '\0';
}

char * remove_from_pool(char const * pool, char const * word) {
	char * new_pool = strdup(pool);
	char * new_pool_iter = new_pool;
	char * tmp_pool = strdup(pool);
	char const * tmp_pool_iter = tmp_pool;
	char * match = NULL;

	while (*word != '\0') {
		match = strchr(tmp_pool, *word++);

		if (match != NULL)
			*match = '.';
	}

	while (*tmp_pool_iter != '\0') {
		if (*tmp_pool_iter != '.')
			*new_pool_iter++ = *tmp_pool_iter;
		tmp_pool_iter++;
	}

	*new_pool_iter = '\0';

	free(tmp_pool);

	return new_pool;
}

int solver_build_anagrams(solver_t * this,
		char const * current_pool, char const * current_anagram)
{
	char const * word = NULL;
	int anagrams_count = 0;
	char * anagram = NULL;
	char * new_pool = NULL;
	int anagram_length = 0;

	for (int i = 0; i < this->words_count; i++) {
		word = this->words[i];

		if (exists_in_pool(current_pool, word)) {
			anagram_length = strlen(current_anagram)
				+ strlen(word) + 2;

			anagram = malloc(anagram_length);
			sprintf(anagram, "%s %s", current_anagram,
					word);

			new_pool = remove_from_pool(current_pool,
					word);

			if (strlen(new_pool) == 0) {
				list_append(this->anagrams, anagram);
				anagrams_count++;
			}

			else if (strlen(new_pool) > 3) {
				anagrams_count += solver_build_anagrams(this,
						new_pool, anagram);
				free(anagram);
				anagram = NULL;
			}

			else {
				free(anagram);
				anagram = NULL;
			}

			free(new_pool);
			new_pool = NULL;
		}

	}

	return anagrams_count;
}

char const * solver_verify_hashes(solver_t * this) {
	unsigned char digest[MD5_DIGEST_SIZE];
	char * anagram = NULL;

	for (int i = 0; i < this->anagrams->anagrams_count; i++) {
		anagram = list_get(this->anagrams, i);

		MD5((unsigned char *)anagram, strlen(anagram), digest);

		if (memcmp(digest, this->digest, MD5_DIGEST_SIZE) == 0) {
			VDEBUG("Found anagram match! `%s`", anagram);
			return anagram;
		}
	}

	return NULL;
}

void solver_loop(solver_t * this) {
	char * pool = NULL;
	char const * match = NULL;

	while (solver_has_current_word(this)) {
		if (exists_in_pool(this->seed, this->current_word)) {
			pool = remove_from_pool(this->seed,
					this->current_word);
			solver_build_anagrams(this, pool, this->current_word);
			free(pool);
		}

		match = solver_verify_hashes(this);

		solver_submit_results(this, match);

		list_reset(this->anagrams);
		solver_next_word(this);
	}
}
