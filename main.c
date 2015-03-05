#include "main.h"

int main() {
	client_t * client = client_init("tcp://" HOST ":" PORT);

	if (client == NULL) {
		ERROR("Client initialisation failed.");
		EXIT_FAILURE;
	}

	client_destroy(client);

	EXIT_SUCCESS;
}
