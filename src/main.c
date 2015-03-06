#include "main.h"

int main() {
	solver_t * solver = solver_init("tcp://" HOST ":" PORT);

	if (solver == NULL) {
		ERROR("Solver initialisation failed.");
		exit(EXIT_FAILURE);
	}

	solver_destroy(solver);

	exit(EXIT_SUCCESS);
}
