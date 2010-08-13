#ifndef AUTOUPNP_REGISTRY_H
#define AUTOUPNP_REGISTRY_H

#include <netinet/in.h>

enum registered_socket_state {
	RS_NONE = 0,
	RS_BOUND = 1,
	RS_LISTENING = 2,
	RS_WORKING = 3
};

struct registered_socket_data {
	const char* protocol;
	char port[6];
	short int state;
};

void registry_add(int fildes, const char* protocol);
void registry_remove(int fildes);

struct registered_socket_data* registry_find(int fildes);

#endif
