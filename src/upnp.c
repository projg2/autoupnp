/* autoupnp -- automatic UPnP open port forwarder
 *	miniupnpc interface
 * (c) 2010-2024 Michał Górny
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <stdlib.h>

#include <pthread.h>

#include <miniupnpc/miniupnpc.h>
#include <miniupnpc/upnpcommands.h>
#include <miniupnpc/upnperrors.h>

#include "upnp.h"
#include "notify.h"

static const int discovery_delay = 2000; /* [ms] */
static const int NO_IGD = -100; /* seems fairly safe */

struct igd_data {
	struct UPNPUrls urls;
	struct IGDdatas data;

	char lan_addr[16];
};

static int igd_set_up = 0;
static pthread_mutex_t igd_data_lock;

static void unlock_igd(void) {
	pthread_mutex_unlock(&igd_data_lock);
}

static struct igd_data* setup_igd(void) {
	static struct igd_data igd_data;

	pthread_mutex_lock(&igd_data_lock);
	if (!igd_set_up) {
		struct UPNPDev* devlist = upnpDiscover(discovery_delay,
				getenv("AUTOUPNP_IF"), NULL, 0, 0, 2, NULL);

		int ret = UPNP_GetValidIGD(devlist, &(igd_data.urls), &(igd_data.data),
				igd_data.lan_addr, sizeof(igd_data.lan_addr)
#if MINIUPNPC_API_VERSION >= 18
				, NULL, 0
#endif
				);

		if (ret == 1)
			igd_set_up = 1;
#if MINIUPNPC_API_VERSION >= 18
		else if (ret == 2)
			igd_set_up = 1;
#endif
		else
			user_notify(notify_error, "Unable to find an IGD on the network.");

		freeUPNPDevlist(devlist);
	}

	if (igd_set_up)
		return &igd_data;
	else
		unlock_igd();
	return NULL;
}

static void dispose_igd_data(void) {
	FreeUPNPUrls(&(setup_igd()->urls));
	igd_set_up = 0;
	unlock_igd();
}

static const char* const mystrupnperror(const int err) {
	const char* const origdesc = strupnperror(err);

	if (!origdesc) {
		switch (err) {
			case UPNPCOMMAND_INVALID_ARGS:
				return "invalid arguments";
			case UPNPCOMMAND_HTTP_ERROR:
				return "HTTP/socket error";
			case UPNPCOMMAND_UNKNOWN_ERROR:
				return "unknown library error";
			default:
				return "unknown UPnP error";
		}
	} else
		return origdesc;
}

enum upnpc_common_mode {
	upnpc_add,
	upnpc_remove
};

static int upnpc_common(struct registered_socket_data* rs, const enum upnpc_common_mode mode,
		const struct igd_data** dataptr) {
	const struct igd_data* igd_data = setup_igd();
	static int retrying = 0;

	if (dataptr)
		*dataptr = igd_data;

	if (igd_data) {
		int ret;

		if (mode == upnpc_add)
			 ret = UPNP_AddPortMapping(
					igd_data->urls.controlURL,
					igd_data->data.first.servicetype,
					rs->port, rs->port, igd_data->lan_addr,
					"AutoUPNP-added port forwarding",
					rs->protocol,
					NULL,
					NULL);
		else if (mode == upnpc_remove)
			ret = UPNP_DeletePortMapping(
					igd_data->urls.controlURL,
					igd_data->data.first.servicetype,
					rs->port, rs->protocol, NULL);

		if (!retrying && ret == UPNPCOMMAND_HTTP_ERROR) {
			/* HTTP request failed? Maybe our IGD definitions are out-of-date.
			 * Let's get rid of them and retry. */
			dispose_igd_data();

			retrying++;
			ret = upnpc_common(rs, mode, dataptr);
			retrying--;
		}

		unlock_igd();
		return ret;
	} else
		return NO_IGD;
}

#pragma GCC visibility push(hidden)

void init_igd(void) {
	pthread_mutexattr_t mattr;

	pthread_mutexattr_init(&mattr);
	pthread_mutexattr_settype(&mattr, PTHREAD_MUTEX_RECURSIVE);
	pthread_mutex_init(&igd_data_lock, &mattr);
	pthread_mutexattr_destroy(&mattr);
}

void dispose_igd(void) {
	if (igd_set_up)
		dispose_igd_data();
	pthread_mutex_destroy(&igd_data_lock);
}

int enable_redirect(struct registered_socket_data* rs) {
	const struct igd_data* igd_data;
	const int ret = upnpc_common(rs, upnpc_add, &igd_data);

	if (ret == UPNPCOMMAND_SUCCESS) {
		char extip[16];

		if (UPNP_GetExternalIPAddress(
				igd_data->urls.controlURL,
				igd_data->data.first.servicetype,
				extip) == UPNPCOMMAND_SUCCESS)
			user_notify(notify_added, "<a href='%s:%s'>%s:%s</a> (%s) forwarded successfully to %s:%s.",
					extip, rs->port, extip, rs->port, rs->protocol, igd_data->lan_addr, rs->port);
		else
			user_notify(notify_added, "Port %s (%s) forwarded successfully to %s:%s.",
					rs->port, rs->protocol, igd_data->lan_addr, rs->port);
	} else if (ret != NO_IGD)
		user_notify(notify_error, "UPNP_AddPortMapping(%s, %s, %s) failed: %d (%s).",
				rs->port, igd_data->lan_addr, rs->protocol, ret, mystrupnperror(ret));

	return ret;
}

int disable_redirect(struct registered_socket_data* rs) {
	const int ret = upnpc_common(rs, upnpc_remove, NULL);

	if (ret == UPNPCOMMAND_SUCCESS)
		user_notify(notify_removed, "Port forwarding for port %s (%s) removed successfully.",
				rs->port, rs->protocol);
	else if (ret != NO_IGD)
		user_notify(notify_error, "UPNP_DeletePortMapping(%s, %s) failed: %d (%s).",
					rs->port, rs->protocol, ret, mystrupnperror(ret));

	return ret;
}

#pragma GCC visibility pop
