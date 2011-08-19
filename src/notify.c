/* autoupnp -- automatic UPnP open port forwarder
 *	user notification
 * (c) 2010 Michał Górny
 * Distributed under the terms of the 3-clause BSD license
 */

#ifdef HAVE_CONFIG_H
#	include "config.h"
#endif

#include <stdio.h>
#include <stdarg.h>

#include <syslog.h>

#ifdef HAVE_PTHREAD
#	include <pthread.h>
#endif

#ifdef HAVE_LIBNOTIFY
#	include <tinynotify.h>
#endif

#include "notify.h"

#pragma GCC visibility push(hidden)

void user_notify(enum notify_type type, const char* const format, ...) {
	va_list ap;
	char buf[1024];
	int syslog_type;
	const char* notify_icon;

	va_start(ap, format);
	vsnprintf(buf, sizeof(buf), format, ap);
	va_end(ap);

	switch (type) {
		case notify_error:
			syslog_type = LOG_ERR;
			notify_icon = "network-error";
			break;
		case notify_removed:
			syslog_type = LOG_INFO;
			notify_icon = "network-idle";
			break;
		case notify_added:
		default:
			syslog_type = LOG_INFO;
			notify_icon = "network-receive";
	}

#ifdef HAVE_LIBNOTIFY
	{
		NotifySession s;
		Notification n;

		s = notify_session_new("autoupnp", NOTIFY_SESSION_NO_APP_ICON);
		n = notification_new_unformatted("AutoUPnP", buf);
		notification_set_app_icon(n, notify_icon);
		notification_send(n, s);

		notification_free(n);
		notify_session_free(s);
	}
#endif

	syslog(syslog_type, "(AutoUPnP) %s", buf);
}

#pragma GCC visibility pop
