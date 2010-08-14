#ifndef AUTOUPNP_NOTIFY_H
#define AUTOUPNP_NOTIFY_H

enum notify_type {
	notify_info,
	notify_error
};

void dispose_notify(void);

void user_notify(enum notify_type type, const char* const format, ...);

#endif
