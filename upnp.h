#ifndef AUTOUPNP_UPNP_H
#define AUTOUPNP_UPNP_H

#include "registry.h"

void enable_redirect(struct registered_socket_data* rs);
void disable_redirect(struct registered_socket_data* rs);

#endif
