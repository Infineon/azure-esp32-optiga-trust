// Required stub for Azure DPS provisioning which depends on socketio, but we don't use socketio in this sample so we can just return NULL here.

#include "azure_c_shared_utility/socketio.h"

const IO_INTERFACE_DESCRIPTION *socketio_get_interface_description(void)
{
    return NULL;
}