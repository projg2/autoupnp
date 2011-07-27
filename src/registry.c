/* autoupnp -- automatic UPnP open port forwarder
 *	The open socket registry
 * (c) 2010 Michał Górny
 * Distributed under the terms of the 3-clause BSD license
 */

#ifdef HAVE_CONFIG_H
#	include "config.h"
#endif

#include <stdlib.h>
#include <unistd.h>

#ifdef HAVE_PTHREAD
#	include <pthread.h>
#endif

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
#ifdef HAVE_PTHREAD
static pthread_rwlock_t socket_registry_lock;
static pthread_mutex_t socket_registry_yield_lock;
#endif

void init_registry(void) {
#ifdef HAVE_PTHREAD
	pthread_mutexattr_t mattr;

	pthread_rwlock_init(&socket_registry_lock, NULL);

	pthread_mutexattr_init(&mattr);
	pthread_mutexattr_settype(&mattr, PTHREAD_MUTEX_RECURSIVE);
	pthread_mutex_init(&socket_registry_yield_lock, &mattr);
	pthread_mutexattr_destroy(&mattr);
#endif
}

void dispose_registry(void) {
#ifdef HAVE_PTHREAD
	pthread_mutex_destroy(&socket_registry_yield_lock);
	pthread_rwlock_destroy(&socket_registry_lock);
#endif
}

struct registered_socket_data* registry_add(const int fildes) {
	struct registered_socket* new_socket = malloc(sizeof(*new_socket));

	if (!new_socket)
		return NULL;

	new_socket->fd = fildes;
	new_socket->pid = getpid();
#ifdef HAVE_PTHREAD
	pthread_mutex_init(&(new_socket->data.lock), NULL);
	/* Lock it soon enough to make sure find() won't get it before. */
	pthread_mutex_lock(&(new_socket->data.lock));

	/* Appending is an atomic op, so we can just lock for reading. */
	pthread_rwlock_rdlock(&socket_registry_lock);
#endif
	new_socket->next = socket_registry;
	socket_registry = new_socket;
#ifdef HAVE_PTHREAD
	pthread_rwlock_unlock(&socket_registry_lock);
#endif

	return &(new_socket->data);
}

void registry_remove(const int fildes) {
	struct registered_socket *i, *prev;
	const pid_t mypid = getpid();

#ifdef HAVE_PTHREAD
	pthread_rwlock_wrlock(&socket_registry_lock);
#endif
	for (i = socket_registry, prev = NULL; i; prev = i, i = i->next) {
		if (i->fd == fildes && i->pid == mypid) {
#ifdef HAVE_PTHREAD
			/* Make sure nobody uses the socket any longer. */
			pthread_mutex_lock(&(i->data.lock));
#endif

			if (prev)
				prev->next = i->next;
			else
				socket_registry = i->next;

#ifdef HAVE_PTHREAD
			pthread_mutex_unlock(&(i->data.lock));
			pthread_mutex_destroy(&(i->data.lock));
#endif
			free(i);
			break;
		}
	}
#ifdef HAVE_PTHREAD
	pthread_rwlock_unlock(&socket_registry_lock);
#endif
}

struct registered_socket_data* registry_find(const int fildes) {
	struct registered_socket* i;
	const pid_t mypid = getpid();
	struct registered_socket_data* ret = NULL;

#ifdef HAVE_PTHREAD
	pthread_rwlock_rdlock(&socket_registry_lock);
#endif
	for (i = socket_registry; i; i = i->next) {
		if (i->fd == fildes && i->pid == mypid) {
#ifdef HAVE_PTHREAD
			pthread_mutex_lock(&(i->data.lock));
#endif
			ret = &(i->data);
			break;
		}
	}
#ifdef HAVE_PTHREAD
	pthread_rwlock_unlock(&socket_registry_lock);
#endif

	return ret;
}

struct registered_socket_data* registry_yield(void) {
	static struct registered_socket* i;
	static int iteration_done = 1;
	struct registered_socket* ret;
	const pid_t mypid = getpid();

	/* To sum up, we're using the following locks here:
	 * - rwlock_rdlock() for the list operation time,
	 * - recursive mutex_lock() once for the function call time,
	 *	second time for the whole iteration time,
	 * - socket-level mutex_lock() for the current and next item.
	 */

#ifdef HAVE_PTHREAD
	/* We do not need to keep the rdlock throughout the whole iteration,
	 * as we're keeping socket-level locks. */
	pthread_rwlock_rdlock(&socket_registry_lock);
	pthread_mutex_lock(&socket_registry_yield_lock);
#endif

	if (iteration_done) {
#ifdef HAVE_PTHREAD
		/* Lock recursively to ensure iterator thread safety. */
		pthread_mutex_lock(&socket_registry_yield_lock);
#endif

		i = socket_registry;
		/* Disable iteration_done a little later. */
	}

	while (i && i->pid != mypid)
		i = i->next;

	if (iteration_done) {
#ifdef HAVE_PTHREAD
		if (i)
			pthread_mutex_lock(&(i->data.lock));
#endif
		iteration_done = 0;
	}

	ret = i;
	if (i) {
		i = i->next;
#ifdef HAVE_PTHREAD
		if (i)
			pthread_mutex_lock(&(i->data.lock));
#endif
	} else {
		iteration_done = 1;
#ifdef HAVE_PTHREAD
		pthread_mutex_unlock(&socket_registry_yield_lock);
#endif
	}

#ifdef HAVE_PTHREAD
	pthread_mutex_unlock(&socket_registry_yield_lock);
	pthread_rwlock_unlock(&socket_registry_lock);
#endif

	if (ret)
		return &(ret->data);
	else
		return NULL;
}

void registry_unlock(struct registered_socket_data* sock) {
#ifdef HAVE_PTHREAD
	pthread_mutex_unlock(&(sock->lock));
#endif
}

#pragma GCC visibility pop
