#include "client.h"

client_t * client_init(char const * endpoint) {
	client_t * client = malloc(sizeof(client_t));

	client->context = zmq_init(ZMQ_THREADS);
	assert(client->context != NULL);

	client->socket = zmq_socket(client->context, ZMQ_REQ);
	assert(client->socket != NULL);

	int rc = zmq_connect(client->socket, endpoint);
	assert(rc == 0);

	return client;
}
