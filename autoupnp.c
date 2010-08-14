/* autoupnp -- automatic UPnP open port forwarder
 * (c) 2010 Michał Górny
 * Distributed under the terms of the 3-clause BSD license
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <dlfcn.h>

#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <netinet/ip.h>

#include "registry.h"
#include "upnp.h"

enum replaced_func {
	rf_socket,
	rf_bind,
	rf_listen,
	rf_close,

	rf_last
};

static void xchg_errno(void) {
	static int saved_errno = 0;
	const int tmp = errno;
	errno = saved_errno;
	saved_errno = tmp;
}

static void* const get_func(const enum replaced_func rf) {
	static void* libc_handle = NULL;
	static void* funcs[rf_last];

	if (!libc_handle) {
		const char* const replaced_func_names[rf_last] =
			{ "socket", "bind", "listen", "close" };
		int i, failure = 0;

		xchg_errno();

		libc_handle = dlopen("libc.so.6", RTLD_LAZY);
		if (!libc_handle) {
			fprintf(stderr, "(AutoUPnP) Unable to dlopen() the libc: %s\n", dlerror());
			exit(EXIT_FAILURE);
		}

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

		xchg_errno();
	}

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

	if (protocol == IPPROTO_TCP)
		return "tcp";
	if (protocol == IPPROTO_UDP)
		return "udp";

	return NULL;
}

int socket(const int domain, const int type, const int protocol) {
	const int (*socket_func)(int, int, int) = get_func(rf_socket);
	int fd = socket_func(domain, type, protocol);

	/* valid IPv4 socket, either TCP or UDP */
	if (fd != -1 && domain == AF_INET) {
		const char* const proto = getproto(type, protocol);

		if (proto)
			registry_add(fd, proto);
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
		}
	}

	return ret;
}

static void exit_handler(void) {
	struct registered_socket_data* i;

	while ((i = registry_yield())) {
		if (i->state == RS_WORKING)
			disable_redirect(i);
	}

	dispose_igd();
}

int listen(const int socket, const int backlog) {
	static int exit_hook_set_up = 0;
	const int (*listen_func)(int, int) = get_func(rf_listen);
	int ret = listen_func(socket, backlog);

	if (ret != -1) {
		struct registered_socket_data* rs = registry_find(socket);

		if (rs) {
			rs->state |= RS_LISTENING;
			if (rs->state == RS_WORKING) {
				if (enable_redirect(rs) != -1 && !exit_hook_set_up) {
					atexit(&exit_handler);
					exit_hook_set_up++;
				}
			}
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
		registry_remove(fildes);
	}

	return close_func(fildes);
}
