#include <netinet/in.h>

enum registered_socket_state {
	RS_NONE = 0,
	RS_BOUND = 1,
	RS_LISTENING = 2
};

struct registered_socket_data {
	const char* protocol;

	union {
		struct sockaddr_in as_sin;
		char* as_str;
	} addr;

	short int state;
};

void registry_add(int fildes, const char* protocol);
void registry_remove(int fildes);

struct registered_socket_data* registry_find(int fildes);
