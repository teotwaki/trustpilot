#include "main.h"

int main() {

	client_t * client = client_init("tcp://" HOST ":" PORT);
	assert(client != NULL);

	return 0;
}
