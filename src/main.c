#include "main.h"

char * get_host() {
	char const * host = getenv("SOLVER_HOST");

	if (host == NULL) {
		WARN("Environment variable SOLVER_HOST not set.");
		host = "tcp://" HOST ":" PORT;
		VINFO("Defaulting to %s.", host);
	}

	return strdup(host);
}

int get_num_threads() {
	char const * threads = getenv("SOLVER_THREADS");

	if (threads == NULL) {
		WARN("Environment variable SOLVER_THREADS not set.");
		threads = "2";
		VINFO("Defaulting to %s.", threads);
	}

	return atoi(threads);
}

void * run(void * zmq_ctx) {
	int rc = 0;
	solver_t * solver = NULL;

	{
		char * solver_host = get_host();
		solver = solver_init(zmq_ctx, solver_host);

		free(solver_host);
		solver_host = NULL;
	}

	if (solver == NULL) {
		ERROR("Solver initialisation failed.");
		pthread_exit(NULL);
	}

	rc = solver_loop(solver);

	if (rc != 0) {
		ERROR("Solver failed to finish its loop.");
		solver_destroy(solver);
		pthread_exit(NULL);
	}

	solver_destroy(solver);
	return NULL;
}

int main() {
	void * ctx = zmq_init(ZMQ_THREADS);

	int num_threads = get_num_threads();

	pthread_t * threads = malloc(sizeof(pthread_t) * num_threads);

	for (int i = 0; i < num_threads; i++)
		pthread_create(&threads[i], NULL, run, ctx);

	for (int i = 0; i < num_threads; i++)
		pthread_join(threads[i], NULL);

	zmq_ctx_term(ctx);

	free(threads);
	exit(EXIT_SUCCESS);
}
