#ifndef CLIENT_H__
#define CLIENT_H__

#include <assert.h>
#include <stdlib.h>
#include <zmq.h>

#define ZMQ_THREADS 1
#define ZMQ_NOFLAGS 0

typedef struct _client_t client_t;

struct _client_t {
	void * context;
	void * socket;
};

client_t * client_init(char const * endpoint);

#endif
