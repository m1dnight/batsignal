#include "mdns.h"
#include "defines.h"

void add_services()
{
    // add our services
    mdns_service_add(NULL, "_" SERVICE, "_tcp", 8080, NULL, 0);
}

void initialise_mdns()
{
    // initialize mDNS service
    esp_err_t err = mdns_init();
    if (err)
    {
        printf("MDNS Init failed: %d\n", err);
        return;
    }

    mdns_hostname_set(HOSTNAME);
    mdns_instance_name_set(INSTANCE);

    add_services();
}