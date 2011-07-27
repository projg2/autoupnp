#ifndef AUTOUPNP_REGISTRY_H
#define AUTOUPNP_REGISTRY_H

#include "config.h"

#ifdef HAVE_PTHREAD
#	include <pthread.h>
#endif
#include <netinet/in.h>

struct registered_socket_data {
	const char* protocol;
	char port[6];
	short int state;
#ifdef HAVE_PTHREAD
	pthread_mutex_t lock;
#endif
};

void init_registry(void);
void dispose_registry(void);

struct registered_socket_data* registry_add(int fildes);
void registry_remove(int fildes);
struct registered_socket_data* registry_find(int fildes);
struct registered_socket_data* registry_yield(void);
void registry_unlock(struct registered_socket_data* sock);

#endif
