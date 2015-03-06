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
	char * solver_host = get_host();
	solver_t * solver = solver_init(solver_host);

	if (solver == NULL) {
		ERROR("Solver initialisation failed.");
		free(solver_host);
		exit(EXIT_FAILURE);
	}

	solver_destroy(solver);
	free(solver_host);

	exit(EXIT_SUCCESS);
}
