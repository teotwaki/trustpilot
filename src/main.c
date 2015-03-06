#include "main.h"

char * get_host(void) {
	char const * host = getenv("SOLVER_HOST");

	if (host == NULL) {
		WARN("Environment variable SOLVER_HOST not set.");
		host = "tcp://" HOST ":" PORT;
		VINFO("Defaulting to %s.", host);
	}

	return strdup(host);
}

int main() {
	int rc = 0;
	solver_t * solver = NULL;

	{
		char * solver_host = get_host();
		solver = solver_init(solver_host);

		free(solver_host);
		solver_host = NULL;
	}

	if (solver == NULL) {
		ERROR("Solver initialisation failed.");
		exit(EXIT_FAILURE);
	}

	rc = solver_loop(solver);

	if (rc != 0) {
		ERROR("Solver failed to finish its loop.");
		solver_destroy(solver);
		exit(EXIT_FAILURE);
	}

	solver_destroy(solver);

	exit(EXIT_SUCCESS);
}
