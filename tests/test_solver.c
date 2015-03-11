#include <assert.h>

#include "solver.h"

void test_anagram(void);
void test_plausible(void);

void test_anagram() {
	solver_t * solver = malloc(sizeof(solver_t));
	memset(solver, 0, sizeof(solver_t));

	solver_set_seed(solver, "hello world");
	solver->current_anagram = "olle h roldw";

	assert(solver_is_anagram(solver) == true);

	solver_set_seed(solver, "foo");
	solver->current_anagram = "bar";

	assert(solver_is_anagram(solver) == false);

	solver_set_seed(solver, "bbc just some length and stuff          ");
	solver->current_anagram = "acc just some length and stuff";

	assert(solver_is_anagram(solver) == false);

	solver->current_anagram = NULL;
	solver_destroy(solver);
}

void test_plausible() {
	solver_t * solver = malloc(sizeof(solver_t));
	memset(solver, 0, sizeof(solver_t));

	solver_set_seed(solver, "hello world");
	solver->current_anagram = "olle h roldw";

	assert(solver_is_plausible_anagram(solver) == true);

	solver_set_seed(solver, "foo");
	solver->current_anagram = "bar";

	assert(solver_is_plausible_anagram(solver) == false);

	solver_set_seed(solver, "bbc just some length and stuff          ");
	solver->current_anagram = "acc just some length and stuff";

	assert(solver_is_plausible_anagram(solver) == true);

	solver->current_anagram = NULL;
	solver_destroy(solver);
}

int main(void) {
	test_plausible();
	test_anagram();
}
