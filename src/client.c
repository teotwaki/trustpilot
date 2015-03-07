#include "client.h"

client_t * client_init(void * zmq_ctx, char const * endpoint) {
	DEBUG("Initialising new client.");

	int rc = 0;
	client_t * this = malloc(sizeof(client_t));

	if (this == NULL) {
		VERROR("Couldn't allocate %d bytes.", sizeof(client_t));
		return NULL;
	}

	this->ctx = zmq_ctx;
	this->sock = NULL;
	this->tokener = NULL;

	this->sock = zmq_socket(this->ctx, ZMQ_REQ);

	if (this->sock == NULL) {
		VERROR("Couldn't create a socket: %s", ZMQ_ERROR);
		client_destroy(this);
		return NULL;
	}

	int linger = 0;
	rc = zmq_setsockopt(this->sock, ZMQ_LINGER, &linger, sizeof(linger));

	if (rc != 0) {
		VERROR("Couldn't set ZMQ_LINGER option to 0: %s", ZMQ_ERROR);
		client_destroy(this);
		return NULL;
	}

	int timeout = 1000;
	rc = zmq_setsockopt(this->sock, ZMQ_RCVTIMEO, &timeout, sizeof(timeout));

	if (rc != 0) {
		VERROR("Couldn't set ZMQ_RECVTIMEO option to 1000: %s", ZMQ_ERROR);
		client_destroy(this);
		return NULL;
	}

	VINFO("Attempting to connect to %s", endpoint);

	rc = zmq_connect(this->sock, endpoint);

	if (rc != 0) {
		VERROR("Couldn't connect to server: %s", ZMQ_ERROR);;
		client_destroy(this);
		return NULL;
	}

	this->tokener = json_tokener_new();

	if (this->tokener == NULL) {
		ERROR("Couldn't instantiate JSON tokener.");
		client_destroy(this);
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
			VERROR("Couldn't close ZMQ socket: %s", ZMQ_ERROR);
			return -1;
		}

		this->sock = NULL;
	}

	if (this->tokener != NULL) {
		json_tokener_free(this->tokener);
		this->tokener = NULL;
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
		VERROR("Couldn't initialise ZMQ message: %s", ZMQ_ERROR);
		return -1;
	}

	memcpy(zmq_msg_data(&msg), payload, length);

	rc = zmq_msg_send(&msg, this->sock, ZMQ_NOFLAGS);

	if (rc == -1) {
		VERROR("Couldn't send message to server: %s", ZMQ_ERROR);
		return -2;
	}

	return 0;
}

char * client_recv(client_t * this) {
	zmq_msg_t msg;

	DEBUG("Listening for new message.");

	int rc = zmq_msg_init(&msg);

	if (rc != 0) {
		VERROR("Couldn't initialise ZMQ message: %s", ZMQ_ERROR);
		return NULL;
	}

	rc = zmq_msg_recv(&msg, this->sock, ZMQ_NOFLAGS);

	if (rc == -1) {
		VERROR("Couldn't receive message: %s", ZMQ_ERROR);
		return NULL;
	}

	int length = zmq_msg_size(&msg);

	VDEBUG("Received message of size %d.", length);

	char * payload = malloc(length + 2);
	memset(payload, 0, length + 2);
	memcpy(payload, zmq_msg_data(&msg), length);

	zmq_msg_close(&msg);

	return payload;
}

int client_recv_ign(client_t * this) {
	DEBUG("Listening for new message.");
	int rc = 0;

	rc = zmq_recv(this->sock, NULL, 0, ZMQ_NOFLAGS);

	if (rc == -1) {
		VERROR("Couldn't receive message: %s", ZMQ_ERROR);
		return -1;
	}

	return 0;
}

json_object * client_recv_json(client_t * this) {
	DEBUG("Listening for JSON message.");

	char * payload = client_recv(this);

	if (payload == NULL) {
		ERROR("Couldn't receive message from server.");
		return NULL;
	}

	int length = strlen(payload);

	json_object * object = json_tokener_parse_ex(
			this->tokener, payload, length);

	if (object == NULL) {
		enum json_tokener_error error = json_tokener_get_error(this->tokener);
		VERROR("Couldn't parse JSON message: %s",
				json_tokener_error_desc(error));
	}

	json_tokener_reset(this->tokener);
	free(payload);

	return object;
}
