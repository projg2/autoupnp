#ifndef AUTOUPNP_NOTIFY_H
#define AUTOUPNP_NOTIFY_H

enum notify_type {
	notify_added,
	notify_removed,
	notify_error
};

void user_notify(enum notify_type type, const char* const format, ...);

#endif
