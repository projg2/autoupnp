/* autoupnp -- automatic UPnP open port forwarder
 *	miniupnpc interface
 * (c) 2010 Michał Górny
 * Distributed under the terms of the 3-clause BSD license
 */

#include <stdlib.h>
#include <syslog.h>

#include <miniupnpc/miniupnpc.h>

#include "upnp.h"

static const int discovery_delay = 2000; /* [ms] */

struct igd_data {
	struct UPNPUrls urls;
	struct IGDdatas datas;

	char lan_addr[16];
};

static int igd_set_up = 0;

static struct igd_data* setup_igd(void) {
	static struct igd_data igd_data;

	if (!igd_set_up) {
		struct UPNPDev* devlist = upnpDiscover(discovery_delay, NULL, NULL, 0);

		if (UPNP_GetValidIGD(devlist, &(igd_data.urls), &(igd_data.datas),
					igd_data.lan_addr, sizeof(igd_data.lan_addr))) {
			igd_set_up = 1;

			syslog(LOG_INFO, "IGD found, local IP: %s!", igd_data.lan_addr);
		}

		freeUPNPDevlist(devlist);
	}

	if (igd_set_up)
		return &igd_data;
	return NULL;
}

static void dispose_igd(void) {
	if (igd_set_up) {
		struct igd_data* igd_data = setup_igd();
		FreeUPNPUrls(&(igd_data->urls));
	}
}

void enable_redirect(struct registered_socket_data* rs) {
	setup_igd();
	syslog(LOG_INFO, "(AutoUPNP) Enabling the redirect for port %s", rs->port);
}

void disable_redirect(struct registered_socket_data* rs) {
	syslog(LOG_INFO, "(AutoUPNP) Disabling the redirect for port %s", rs->port);
}
