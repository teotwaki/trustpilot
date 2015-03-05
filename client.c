#include "client.h"

client_t * client_init(char const * endpoint) {
	DEBUG("Initialising new client");

	client_t * client = malloc(sizeof(client_t));

	if (client == NULL) {
		VERROR("Couldn't allocate %d bytes.", sizeof(client_t));
		return NULL;
	}

	client->context = zmq_init(ZMQ_THREADS);

	if (client->context == NULL) {
		ERROR("Couldn't initialise ZMQ context.");
		return NULL;
	}

	client->socket = zmq_socket(client->context, ZMQ_REQ);

	if (client->socket == NULL) {
		ERROR("Couldn't create a socket.");
		return NULL;
	}

	int rc = zmq_connect(client->socket, endpoint);

	if (rc != 0) {
		ERROR("Couldn't connect to server.");
		return NULL;
	}

	return client;
}
