/* autoupnp -- automatic UPnP open port forwarder
 *	miniupnpc interface
 * (c) 2010 Michał Górny
 * Distributed under the terms of the 3-clause BSD license
 */

#include <stdlib.h>
#include <syslog.h>

#include <miniupnpc/miniupnpc.h>
#include <miniupnpc/upnpcommands.h>
#include <miniupnpc/upnperrors.h>

#include "upnp.h"

static const int discovery_delay = 2000; /* [ms] */

struct igd_data {
	struct UPNPUrls urls;
	struct IGDdatas data;

	char lan_addr[16];
};

static int igd_set_up = 0;

static struct igd_data* setup_igd(void) {
	static struct igd_data igd_data;

	if (!igd_set_up) {
		struct UPNPDev* devlist = upnpDiscover(discovery_delay, NULL, NULL, 0);

		if (UPNP_GetValidIGD(devlist, &(igd_data.urls), &(igd_data.data),
					igd_data.lan_addr, sizeof(igd_data.lan_addr)))
			igd_set_up = 1;
		else
			syslog(LOG_ERR, "(AutoUPNP) Unable to find an IGD on the network.");

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
	const struct igd_data* igd_data = setup_igd();

	if (igd_data) {
		const int ret = UPNP_AddPortMapping(
				igd_data->urls.controlURL,
				igd_data->data.servicetype,
				rs->port, rs->port, igd_data->lan_addr,
				"AutoUPNP-added port forwarding",
				rs->protocol, NULL);

		if (ret == 0)
			syslog(LOG_INFO, "(AutoUPNP) Port %s/%s forwarded successfully to %s.",
					rs->port, rs->protocol, igd_data->lan_addr);
		else
			syslog(LOG_ERR, "(AutoUPNP) UPNP_AddPortMapping(%s, %s, %s) failed: %d (%s).",
					rs->port, igd_data->lan_addr, rs->protocol, ret, strupnperror(ret));
	}
}

void disable_redirect(struct registered_socket_data* rs) {
	const struct igd_data* igd_data = setup_igd();

	if (igd_data) {
		const int ret = UPNP_DeletePortMapping(
				igd_data->urls.controlURL,
				igd_data->data.servicetype,
				rs->port, rs->protocol, NULL);

		if (ret == 0)
			syslog(LOG_INFO, "(AutoUPNP) Port forwarding for port %s/%s removed successfully.",
					rs->port, rs->protocol);
		else
			syslog(LOG_ERR, "(AutoUPNP) UPNP_DeletePortMapping(%s, %s) failed: %d (%s).",
					rs->port, rs->protocol, ret, strupnperror(ret));
	}
}
