#ifndef AUTOUPNP_UPNP_H
#define AUTOUPNP_UPNP_H

#include "registry.h"

void init_igd(void);
void dispose_igd(void);

int enable_redirect(struct registered_socket_data* rs);
int disable_redirect(struct registered_socket_data* rs);

#endif
