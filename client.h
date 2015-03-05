#ifndef CLIENT_H__
#define CLIENT_H__

#include <json-c/json.h>
#include <stdlib.h>
#include <string.h>
#include <zmq.h>

#include "log.h"

#define ZMQ_THREADS 1
#define ZMQ_NOFLAGS 0

typedef struct _client_t client_t;

struct _client_t {
	void * context;
	void * socket;
};

// Constructor/destructor
client_t * client_init(char const * endpoint);
int client_destroy(client_t * this);

// Communication with server
int client_send(client_t * this, char const * payload);
char * client_recv(client_t * this);
json_object * client_recv_json(client_t * this);

#endif
