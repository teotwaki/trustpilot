#include "client.h"

client_t * client_init(char const * endpoint) {
	DEBUG("Initialising new client.");

	int rc = 0;
	client_t * this = malloc(sizeof(client_t));

	if (this == NULL) {
		VERROR("Couldn't allocate %d bytes.", sizeof(client_t));
		return NULL;
	}

	this->ctx = NULL;
	this->sock = NULL;

	this->ctx = zmq_init(ZMQ_THREADS);

	if (this->ctx == NULL) {
		ERROR("Couldn't initialise ZMQ context.");
		return NULL;
	}

	this->sock = zmq_socket(this->ctx, ZMQ_REQ);

	if (this->sock == NULL) {
		ERROR("Couldn't create a socket.");
		client_destroy(this);
		return NULL;
	}

	rc = zmq_connect(this->sock, endpoint);

	if (rc != 0) {
		ERROR("Couldn't connect to server.");
		return NULL;
	}

	return this;
}

int client_destroy(client_t * this) {
	DEBUG("Destroying client.");

	int rc = 0;

	if (this->sock != NULL) {
		rc = zmq_close(this->sock);

		if (rc != 0) {
			ERROR("Couldn't close ZMQ socket.");
			return -1;
		}

		this->sock = NULL;
	}

	if (this->ctx != NULL) {
		rc = zmq_term(this->ctx);

		if (rc != 0) {
			ERROR("Couldn't terminate ZMQ context.");
			return -2;
		}

		this->ctx = NULL;
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

	rc = zmq_msg_send(this->sock, &msg, ZMQ_NOFLAGS);

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

	rc = zmq_msg_recv(this->sock, &msg, ZMQ_NOFLAGS);

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
