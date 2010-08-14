/* autoupnp -- automatic UPnP open port forwarder
 *	user notification
 * (c) 2010 Michał Górny
 * Distributed under the terms of the 3-clause BSD license
 */

#include <stdio.h>
#include <stdarg.h>

#include <syslog.h>

#include "notify.h"

#pragma GCC visibility push(hidden)

void user_notify(enum notify_type type, const char* const format, ...) {
	va_list ap;
	char buf[1024];
	int syslog_type;

	va_start(ap, format);
	vsnprintf(buf, sizeof(buf), format, ap);
	va_end(ap);

	switch (type) {
		case notify_error:
			syslog_type = LOG_ERR;
			break;
		case notify_info:
		default:
			syslog_type = LOG_INFO;
			break;
	}

	syslog(syslog_type, "%s", buf);
}

#pragma GCC visibility pop
