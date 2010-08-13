/* autoupnp -- automatic UPnP open port forwarder
 *	miniupnpc interface
 * (c) 2010 Michał Górny
 * Distributed under the terms of the 3-clause BSD license
 */

#include <syslog.h>

#include "upnp.h"

void enable_redirect(struct registered_socket_data* rs) {
	syslog(LOG_INFO, "(AutoUPNP) Enabling the redirect for port %s", rs->port);
}

void disable_redirect(struct registered_socket_data* rs) {
	syslog(LOG_INFO, "(AutoUPNP) Disabling the redirect for port %s", rs->port);
}
