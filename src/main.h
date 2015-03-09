#ifndef MAIN_H__
#define MAIN_H__

#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <zmq.h>

#include "solver.h"
#include "log.h"

#define HOST "localhost"
#define PORT "20123"

#define ZMQ_THREADS 1

char * get_host(void);
int get_num_cores(void);
int get_num_threads(void);
void * run(void * zmq_ctx);

int main(void);

#endif
