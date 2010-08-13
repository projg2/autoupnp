/* autoupnp -- automatic UPnP open port forwarder
 * (c) 2010 Michał Górny
 * Distributed under the terms of the 3-clause BSD license
 */

#include <stdlib.h>
#include <dlfcn.h>

#include <sys/socket.h>
#include <unistd.h>

enum replaced_func {
	rf_bind = 0,
	rf_listen,
	rf_close,

	rf_last
};

static void* const get_func(const enum replaced_func rf) {
	static void* libc_handle = NULL;
	static void* funcs[rf_last];

	if (!libc_handle) {
		const char* const replaced_func_names[rf_last] =
			{ "bind", "listen", "close" };
		int i;

		libc_handle = dlopen("libc.so.6", RTLD_LAZY);
		if (!libc_handle)
			return NULL;

		for (i = 0; i < rf_last; i++)
			funcs[i] = dlsym(libc_handle, replaced_func_names[i]);
	}

	return funcs[rf];
}

int bind(const int socket, const struct sockaddr* const address,
		const socklen_t address_len) {
	const int (*bind_func)(int, const struct sockaddr*, socklen_t) =
		get_func(rf_bind);

	return bind_func(socket, address, address_len);
}

int listen(const int socket, const int backlog) {
	const int (*listen_func)(int, int) = get_func(rf_listen);

	return listen_func(socket, backlog);
}

int close(const int fildes) {
	const int (*close_func)(int) = get_func(rf_close);

	return close_func(fildes);
}
