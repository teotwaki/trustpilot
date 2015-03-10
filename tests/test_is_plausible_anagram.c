#include <assert.h>

#include "solver.h"

int main(void);

int main(void) {
	char const * first = NULL;
	char const * second = NULL;

	first = "hello world";
	second = "olle h roldw";

	assert(is_plausible_anagram(first, second) == true);

	first = "hello world";
	second = "hello world";

	assert(is_plausible_anagram(first, second) == true);

	first = "foo";
	second = "bar";

	assert(is_plausible_anagram(first, second) == false);

	first = "bbc just some length and stuff          ";
	second = "acc just some length and stuff";

	assert(is_plausible_anagram(first, second) == true);

	return 0;
}
