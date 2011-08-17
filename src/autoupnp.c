/* autoupnp -- automatic UPnP open port forwarder
 * (c) 2010 Michał Górny
 * Distributed under the terms of the 3-clause BSD license
 */

#ifdef HAVE_CONFIG_H
#	include "config.h"
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <dlfcn.h>

#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netinet/in.h>

#ifdef HAVE_PTHREAD
#	include <pthread.h>
#endif

#include "notify.h"
#include "registry.h"
#include "upnp.h"

enum replaced_func {
	rf_socket,
	rf_bind,
	rf_listen,
	rf_close,

	rf_last
};

enum registered_socket_state {
	RS_NONE = 0,
	RS_BOUND = 1,
	RS_LISTENING = 2,
	RS_WORKING = 3
};

static void xchg_errno(void) {
	static int saved_errno = 0;
	const int tmp = errno;
	errno = saved_errno;
	saved_errno = tmp;
}

static void exit_handler(void) {
	struct registered_socket_data* i;
#ifdef HAVE_PTHREAD
	static pthread_mutex_t exit_lock = PTHREAD_MUTEX_INITIALIZER;

	pthread_mutex_lock(&exit_lock);
#endif
	while ((i = registry_yield())) {
		if (i->state == RS_WORKING)
			disable_redirect(i);
		registry_unlock(i);
	}

	dispose_igd();
	dispose_registry();
#ifdef HAVE_PTHREAD
	pthread_mutex_unlock(&exit_lock);
#endif
}

static void init_handler(void) {
	init_registry();
	init_igd();
	atexit(&exit_handler);
}

static void* const get_func(const enum replaced_func rf) {
	static void* libc_handle = NULL;
	static void* funcs[rf_last];
#ifdef HAVE_PTHREAD
	static pthread_mutex_t get_func_mutex = PTHREAD_MUTEX_INITIALIZER;

	pthread_mutex_lock(&get_func_mutex);
#endif
	if (!libc_handle) {
		const char* const replaced_func_names[rf_last] =
			{ "socket", "bind", "listen", "close" };
		int i, failure = 0;

		xchg_errno();

#ifdef RTLD_NEXT
		libc_handle = RTLD_NEXT;
#else
		libc_handle = dlopen("libc.so", RTLD_LAZY);
		for (i = 0; !libc_handle && i < 10; i++) {
			char libc_name[10];
			snprintf(libc_name, sizeof(libc_name), "libc.so.%d", i);
			libc_handle = dlopen(libc_name, RTLD_LAZY);
		}
		if (!libc_handle) {
			fprintf(stderr, "(AutoUPnP) Unable to dlopen() the libc: %s\n", dlerror());
			exit(EXIT_FAILURE);
		}
#endif

		for (i = 0; i < rf_last; i++) {
			funcs[i] = dlsym(libc_handle, replaced_func_names[i]);
			if (!funcs[i]) {
				fprintf(stderr, "(AutoUPnP) Unable to dlsym(%s): %s\n",
						replaced_func_names[i], dlerror());
				failure++;
			}
		}
		if (failure)
			exit(EXIT_FAILURE);

		init_handler();
		xchg_errno();
	}
#ifdef HAVE_PTHREAD
	pthread_mutex_unlock(&get_func_mutex);
#endif

	return funcs[rf];
}

static const char* const getproto(const int type, const int protocol) {
	if (!protocol) {
		if (type == SOCK_STREAM)
			return "tcp";
		if (type == SOCK_DGRAM)
			return "udp";

		return NULL;
	}

	/* These shall be defined as macros, POSIX states */
#ifdef IPPROTO_TCP
	if (protocol == IPPROTO_TCP)
		return "tcp";
#endif
#ifdef IPPROTO_UDP
	if (protocol == IPPROTO_UDP)
		return "udp";
#endif

	return NULL;
}

int socket(const int domain, const int type, const int protocol) {
	const int (*socket_func)(int, int, int) = get_func(rf_socket);
	int fd = socket_func(domain, type, protocol);

	/* valid IPv4 socket, either TCP or UDP */
	if (fd != -1 && domain == AF_INET) {
		const char* const proto = getproto(type, protocol);
		struct registered_socket_data* d;

		if (proto && ((d = registry_add(fd)))) {
			d->protocol = proto;
			d->state = RS_NONE;
			registry_unlock(d);
		}
	}

	return fd;
}

int bind(const int socket, const struct sockaddr* const address,
		const socklen_t address_len) {
	const int (*bind_func)(int, const struct sockaddr*, socklen_t) =
		get_func(rf_bind);
	const int ret = bind_func(socket, address, address_len);

	if (ret != -1 && address_len == sizeof(struct sockaddr_in)) {
		struct registered_socket_data* rs = registry_find(socket);

		if (rs) {
			const struct sockaddr_in* sin = (void*) address;

			/* IPv4 check again, and require being bound on all interfaces */
			if (sin->sin_family == AF_INET && sin->sin_addr.s_addr == INADDR_ANY) {
				snprintf(rs->port, sizeof(rs->port), "%d", ntohs(sin->sin_port));
				rs->state |= RS_BOUND;
				if (rs->state == RS_WORKING)
					enable_redirect(rs);
			}

			registry_unlock(rs);
		}
	}

	return ret;
}

int listen(const int socket, const int backlog) {
	const int (*listen_func)(int, int) = get_func(rf_listen);
	int ret = listen_func(socket, backlog);

	if (ret != -1) {
		struct registered_socket_data* rs = registry_find(socket);

		if (rs) {
			rs->state |= RS_LISTENING;
			if (rs->state == RS_WORKING)
				enable_redirect(rs);

			registry_unlock(rs);
		}
	}

	return ret;
}

int close(const int fildes) {
	const int (*close_func)(int) = get_func(rf_close);
	struct registered_socket_data* rs = registry_find(fildes);

	if (rs) {
		if (rs->state == RS_WORKING)
			disable_redirect(rs);
		registry_unlock(rs);
		registry_remove(fildes);
	}

	return close_func(fildes);
}
