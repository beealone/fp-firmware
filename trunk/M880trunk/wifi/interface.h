#ifndef _INTERFACE_H
#define _INTERFACE_H

int interface_up(const char *ifname);
int interface_down(const char *ifname);
int is_interface_up(const char *ifname);
int interface_index(const char *ifname, int *index);
char *ipaddr_to_string(unsigned int addr);
int string_to_ipaddr(const char *str);
int interface_get_addr(const char *if_name, unsigned int *addr);
int interface_set_addr(const char *ifname, unsigned int addr);
int interface_get_mask(const char *if_name, unsigned int *mask);
int interface_set_mask(const char *ifname, unsigned int mask);
int interface_remove_default_route(const char *ifname);
int interface_add_host_route(const char *ifname, unsigned int addr);
int interface_get_default_route(char *ifname, unsigned int *gw);
#endif
