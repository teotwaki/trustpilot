#include "client.h"

client_t * client_init(char const * endpoint) {
	DEBUG("Initialising new client.");

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

int client_destroy(client_t * this) {
	int rc = zmq_close(this->socket);

	if (rc != 0) {
		ERROR("Couldn't close ZMQ socket.");
		return -1;
	}

	rc = zmq_term(this->context);

	if (rc != 0) {
		ERROR("Couldn't terminate ZMQ context.");
		return -2;
	}

	free(this);

	return 0;
}

int client_send(client_t * this, char const * payload) {
	int length = strlen(payload);

	VDEBUG("Sending message of size %d.", length);

	zmq_msg_t msg;

	int rc = zmq_msg_init_size(&msg, length);

	if (rc != 0) {
		ERROR("Couldn't initialise ZMQ message.");
		return -1;
	}

	memcpy(zmq_msg_data(&msg), payload, length);

	rc = zmq_msg_send(this->socket, &msg, ZMQ_NOFLAGS);

	if (rc != 0) {
		ERROR("Couldn't send message to server.");
		return -2;
	}

	return 0;
}

char * client_recv(client_t * this) {
	zmq_msg_t msg;

	DEBUG("Listening for new message.");

	int rc = zmq_msg_init(&msg);

	if (rc != 0) {
		ERROR("Couldn't initialise ZMQ message.");
		return NULL;
	}

	rc = zmq_msg_recv(this->socket, &msg, ZMQ_NOFLAGS);

	if (rc != 0) {
		ERROR("Couldn't receive message.");
		return NULL;
	}

	int length = zmq_msg_size(&msg);

	VDEBUG("Received message of size %d.", length);

	char * payload = malloc(length + 2);
	memcpy(payload, zmq_msg_data(&msg), length);
	payload[length + 1] = '\0';

	zmq_msg_close(&msg);

	return payload;
}

json_object * client_recv_json(client_t * this) {
	DEBUG("Listening for JSON message.");

	char * payload = client_recv(this);

	json_object * object = json_tokener_parse(payload);

	free(payload);

	return object;
}
