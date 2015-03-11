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
	this->digest = NULL;
	this->current_digest = malloc(sizeof(unsigned char) * MD5_DIGEST_SIZE);
	this->match = NULL;
	this->anagrams_count = 0;
	this->current_anagram = NULL;
	this->tmp_first = NULL;
	this->tmp_second = NULL;

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

	if (this->digest != NULL) {
		free(this->digest);
		this->digest = NULL;
	}

	if (this->current_digest != NULL) {
		free(this->current_digest);
		this->current_digest = NULL;
	}

	if (this->current_digest != NULL) {
		free(this->current_digest);
		this->current_digest = NULL;
	}

	if (this->current_anagram != NULL) {
		free(this->current_anagram);
		this->current_anagram = NULL;
	}

	if (this->tmp_first != NULL) {
		free(this->tmp_first);
		this->tmp_first = NULL;
	}

	if (this->tmp_second != NULL) {
		free(this->tmp_second);
		this->tmp_second = NULL;
	}

	free(this);

	return 0;
}

void solver_set_seed(solver_t * this, char const * seed) {
	if (this->seed != NULL) {
		free(this->seed);
	}

	this->seed = strdup(seed);

	this->seed_length = 0;
	this->seed_ord = 0;

	for (char * iter = this->seed; *iter != '\0'; iter++)
		if (*iter != ' ') {
			this->seed_length++;
			this->seed_ord += *iter;
		}
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
				this->seed_length = 0;
			}

			solver_set_seed(this, json_object_get_string(val));

			int buffer_length = this->seed_length * 3;
			this->current_anagram = malloc(buffer_length);

			if (this->tmp_first != NULL)
				free(this->tmp_first);

			if (this->tmp_second != NULL)
				free(this->tmp_second);

			this->tmp_first = malloc(buffer_length);
			this->tmp_second = malloc(buffer_length);
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

int solver_submit_results(solver_t * this, char const * match, int duration)
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
				json_object_new_int(this->anagrams_count));

		json_object_object_add(object, "current_word",
				json_object_new_string(this->current_word));

		json_object_object_add(object, "duration",
				json_object_new_int(duration));
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

int count_letters(char const * ptr) {
	int count = 0;

	while (*ptr != '\0')
		if (*ptr++ != ' ')
			count++;

	return count;
}

// Return a copy of the letters a string (without spaces)
char * lttrdup(char const * str) {
	// Technically, we're wasting a few bytes here, but that's OK.
	int length = strlen(str) + 1;
	char * copy = malloc(length);
	char * tmp = copy;
	memset(copy, 0, length);

	do {
		if (*str != ' ')
			*tmp++ = *str;
	} while (*str++ != '\0');

	return copy;
}

void remove_from_pool(char * new_pool, char const * pool, char const * word) {
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
}

void solver_build_anagrams(solver_t * this)
{
	char * i_end = this->current_anagram + strlen(this->current_anagram);
	char * j_end = NULL;

	for (int i = 0; this->match == NULL && i < this->words_count; i++) {
		sprintf(i_end, " %s", this->words[i]);
		j_end = i_end + strlen(this->words[i]);

		for (int j = 0; this->match == NULL && j < this->words_count; j++) {
			sprintf(j_end, " %s", this->words[j]);

			if (solver_is_anagram(this)) {
				MD5((unsigned char *)this->current_anagram,
						strlen(this->current_anagram),
						this->current_digest);
				if (memcmp(this->current_digest, this->digest,
							MD5_DIGEST_SIZE) == 0)
					this->match = strdup(this->current_anagram);
				this->anagrams_count++;
			}
		}

		*i_end = '\0';
	}
}

bool solver_is_anagram(solver_t * this) {
	if (solver_is_plausible_anagram(this)) {
		// If the strings are equivalent, they are anagrams
		if (strcmp(this->current_anagram, this->seed) == 0)
			return true;

		else {
			// Remove the spaces from our strings
			char * copy = lttrdup(this->current_anagram);
			char * match = NULL;
			char const * seed_iter = this->seed;

			// Iterate over cpy_second until the end is reached
			while (*seed_iter != '\0') {

				// second is the original string, with spaces. Skip over them.
				if (*seed_iter != ' ') {
					// find the character pointed by tmp_second in cpy_first
					match = strchr(copy, *seed_iter);

					// if a match is found, overwrite it
					if (match != NULL)
						*match = '.';

					// no match found, skip to cleanup
					else
						break;
				}

				// move on to the next character
				seed_iter++;
			}

			// if tmp_second doesn't point to the end of the string, we found a
			// character that doesn't exist in first, hence, no anagram
			bool result = *seed_iter == '\0';

			// cleanup
			free(copy);

			return result;
		}
	}

	return false;
}

bool solver_is_plausible_anagram(solver_t * this) {
	int first_length = 0;
	int first_ord = 0;

	for (char const * first = this->current_anagram;
			*first != '\0'; first++)
		if (*first != ' ') {
			first_ord += *first;
			first_length++;
		}

	return first_length == this->seed_length && first_ord == this->seed_ord;
}

void solver_loop(solver_t * this) {
	struct timeval start, end;

	while (solver_has_current_word(this)) {
		gettimeofday(&start, NULL);

		strcpy(this->current_anagram, this->current_word);
		solver_build_anagrams(this);

		gettimeofday(&end, NULL);

		solver_submit_results(this, this->match,
				microseconds(end) - microseconds(start));

		if (this->match != NULL) {
			free(this->match);
			this->match = NULL;
		}

		this->anagrams_count = 0;
		solver_next_word(this);
	}
}
