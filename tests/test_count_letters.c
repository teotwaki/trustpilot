#include <assert.h>

#include "solver.h"

int main(void) {
	char const * tmp = NULL;

	tmp = "hello";
	assert(count_letters(tmp) == 5);

	tmp = "a b c d e f g h";
	assert(count_letters(tmp) == 8);
}
