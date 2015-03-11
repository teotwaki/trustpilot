#ifndef CLIENT_H__
#define CLIENT_H__

#include <json-c/json.h>
#include <stdlib.h>
#include <string.h>
#include <zmq.h>

#include "log.h"

#define ZMQ_NOFLAGS 0
#define ZMQ_ERROR zmq_strerror(errno)
#define ZMQ_TIMEOUT 60000

#define CLIENT_INITIALISE "{\"type\":\"initialise\"}"
#define CLIENT_NEXT_WORD "{\"type\":\"next_word\"}"

typedef struct _client_t client_t;

struct _client_t {
	void * ctx;
	void * sock;
	json_tokener * tokener;
};

// Constructor/destructor
client_t * client_init(void * zmq_ctx, char const * endpoint);
int client_destroy(client_t * this);

// Communication with server
int client_send(client_t * this, char const * payload);
char * client_recv(client_t * this);
int client_recv_ign(client_t * this);
json_object * client_recv_json(client_t * this);

#endif
