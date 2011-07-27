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
#	include <libnotify/notify.h>
#endif

#include "notify.h"

#pragma GCC visibility push(hidden)

void dispose_notify(void) {
#ifdef HAVE_LIBNOTIFY
	if (notify_is_initted())
		notify_uninit();
#endif
}

void user_notify(enum notify_type type, const char* const format, ...) {
#ifdef HAVE_LIBNOTIFY
#	ifdef HAVE_PTHREAD
	pthread_mutex_t notify_init_lock = PTHREAD_MUTEX_INITIALIZER;
#	endif
#endif
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
#ifdef HAVE_PTHREAD
	pthread_mutex_lock(&notify_init_lock);
#endif
	if (!notify_is_initted())
		notify_init("autoupnp");
#ifdef HAVE_PTHREAD
	pthread_mutex_unlock(&notify_init_lock);
#endif

	if (notify_is_initted()) {
#ifndef NOTIFY_CHECK_VERSION /* macro did not exist before libnotify-0.5.2 */
#	define NOTIFY_CHECK_VERSION(x,y,z) 0
#endif

#if NOTIFY_CHECK_VERSION(0,7,0)
		NotifyNotification* n = notify_notification_new(
				"AutoUPnP", buf, notify_icon);
#else
		NotifyNotification* n = notify_notification_new(
				"AutoUPnP", buf, notify_icon, NULL);
#endif

		notify_notification_show(n, NULL);
	}
#endif

	syslog(syslog_type, "(AutoUPnP) %s", buf);
}

#pragma GCC visibility pop
