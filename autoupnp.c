/* autoupnp -- automatic UPnP open port forwarder
 * (c) 2010 Michał Górny
 * Distributed under the terms of the 3-clause BSD license
 */

#include <stdlib.h>
#include <dlfcn.h>

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
