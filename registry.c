/* autoupnp -- automatic UPnP open port forwarder
 *	The open socket registry
 * (c) 2010 Michał Górny
 * Distributed under the terms of the 3-clause BSD license
 */

#include <stdlib.h>
#include <unistd.h>

#include "registry.h"

#pragma GCC visibility push(hidden)

/* We're using PID matching here to avoid removing parent's redirections
 * in a forked child process. */

struct registered_socket {
	int fd;
	pid_t pid;
	struct registered_socket_data data;
	struct registered_socket* next;
};

static struct registered_socket* socket_registry = NULL;

struct registered_socket_data* registry_add(const int fildes) {
	struct registered_socket* new_socket = malloc(sizeof(*new_socket));

	if (!new_socket)
		return NULL;

	new_socket->fd = fildes;
	new_socket->pid = getpid();
	new_socket->next = socket_registry;
	socket_registry = new_socket;

	return &(new_socket->data);
}

void registry_remove(const int fildes) {
	struct registered_socket *i, *prev;
	const pid_t mypid = getpid();

	for (i = socket_registry, prev = NULL; i; prev = i, i = i->next) {
		if (i->fd == fildes && i->pid == mypid) {
			if (prev)
				prev->next = i->next;
			else
				socket_registry = i->next;
			free(i);
			return;
		}
	}
}

struct registered_socket_data* registry_find(const int fildes) {
	struct registered_socket* i;
	const pid_t mypid = getpid();

	for (i = socket_registry; i; i = i->next) {
		if (i->fd == fildes && i->pid == mypid)
			return &(i->data);
	}

	return NULL;
}

struct registered_socket_data* registry_yield(void) {
	static struct registered_socket* i;
	static int iteration_done = 1;
	struct registered_socket* ret;
	const pid_t mypid = getpid();

	if (iteration_done) {
		i = socket_registry;
		iteration_done = 0;
	}

	while (i && i->pid != mypid)
		i = i->next;

	ret = i;
	if (i)
		i = i->next;
	else
		iteration_done = 1;

	if (ret)
		return &(ret->data);
	else
		return NULL;
}

#pragma GCC visibility pop
