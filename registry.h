#ifndef AUTOUPNP_REGISTRY_H
#define AUTOUPNP_REGISTRY_H

#include <netinet/in.h>

struct registered_socket_data {
	const char* protocol;
	char port[6];
	short int state;
};

struct registered_socket_data* registry_add(int fildes);
void registry_remove(int fildes);

struct registered_socket_data* registry_find(int fildes);
struct registered_socket_data* registry_yield(void);

#endif
