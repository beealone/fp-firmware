#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <linux/if.h>
#include <linux/sockios.h>
#include <linux/route.h>
#include <linux/wireless.h>

#include "utils.h"

#define IFUP (IFF_UP | IFF_RUNNING | IFF_BROADCAST | IFF_MULTICAST)
#define sin_addr(s) (((struct sockaddr_in *)(s))->sin_addr)

static int interface_set_flags(int sock, const char *name, unsigned set, unsigned clr)
{
	struct ifreq ifr;

	if (sock < 0 || name == NULL){
		return -1;
	}

	memset(&ifr, 0x00, sizeof(struct ifreq));
	strncpy(ifr.ifr_name, name, IFNAMSIZ);
	ifr.ifr_name[IFNAMSIZ - 1] = '\0';

	if(ioctl(sock, SIOCGIFFLAGS, &ifr) < 0){
		return -1;
	}
	ifr.ifr_flags = (ifr.ifr_flags & (~clr)) | set;

	return ioctl(sock, SIOCSIFFLAGS, &ifr);
}

int interface_up(const char *ifname)
{
	int sock = -1;

	if (ifname == NULL) {
		return -1;
	}

	sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (sock < 0) {
		return -1;
	}
	
	if (interface_set_flags(sock, ifname, IFF_UP, 0) < 0) {
		close(sock);
		return -1;
	}

	close(sock);

	return 0;
}

int interface_down(const char *ifname)
{
	int sock = -1;

	if (ifname == NULL) {
		return -1;
	}

	sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (sock < 0) {
		return -1;
	}

	if (interface_set_flags(sock, ifname, 0, IFF_UP) < 0) {
		close(sock);
		return -1;
	}

	close(sock);

	return 0;
}

int is_interface_up(char *ifname)
{
	int is_up = 0;
	int sock = -1;
	struct ifreq ifr;

	sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (sock < 0) {
		return -1;
	}

	strcpy(ifr.ifr_name, ifname);
	if(ioctl(sock, SIOCGIFFLAGS, (caddr_t) &ifr) < 0) {
		close(sock);
		return -1;
	}

	if ((ifr.ifr_flags & IFF_UP) && (ifr.ifr_flags & IFF_RUNNING)) {
		is_up = 1;
	} else {
		is_up = 0;
	}

	close(sock);

	return is_up;
}

int interface_index(const char *ifname, int *index)
{
	int sock = -1;
	struct ifreq ifr;

	if (ifname == NULL) {
		return -1;
	}

	sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (sock < 0) {
		return -1;
	}

	memset(&ifr, 0x00, sizeof(struct ifreq));
	strncpy(ifr.ifr_name, ifname, IFNAMSIZ);
	ifr.ifr_name[IFNAMSIZ - 1] = '\0';

	if (ioctl(sock, SIOCGIFINDEX, &ifr) < 0){
		close(sock);
		return -1;
	}

	if (index != NULL) {
		*index = ifr.ifr_ifindex;
	}

	close(sock);

	return 0;
}

static void interface_inaddr_init(struct sockaddr *sa, unsigned int addr)
{
	struct sockaddr_in *sin = (struct sockaddr_in *) sa;
	sin->sin_family = AF_INET;
	sin->sin_port = 0;
	sin->sin_addr.s_addr = addr;
}

char *ipaddr_to_string(unsigned int addr)
{
	struct in_addr in_addr;

	in_addr.s_addr = addr;
	return inet_ntoa(in_addr);
}

int string_to_ipaddr(const char *str)
{
	return inet_addr(str);
}

int interface_get_addr(const char *if_name, unsigned int *addr)
{
	int sock = -1;
	struct ifreq ifr;

	if (if_name == NULL){
		return -1;
	}

	sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (sock < 0){
		return -1;
	}

	memset(&ifr, 0x00, sizeof(struct ifreq));
	strncpy(ifr.ifr_name, if_name, IFNAMSIZ);
	ifr.ifr_name[IFNAMSIZ - 1] = '\0';

	if(ioctl(sock, SIOCGIFADDR, &ifr) < 0) {
		*addr = 0;
		close(sock);
		return -1;
	}

	if (addr != NULL) {
		*addr =  ((struct sockaddr_in*) &ifr.ifr_addr)->sin_addr.s_addr;
	}

	close(sock);

	return 0;
}

int interface_set_addr(const char *ifname, unsigned int addr)
{
	int sock = -1;
	struct ifreq ifr;

	if (ifname == NULL){
		return -1;
	}

	sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (sock < 0) {
		return -1;
	}

	memset(&ifr, 0x00, sizeof(struct ifreq));
	strncpy(ifr.ifr_name, ifname, IFNAMSIZ);
	ifr.ifr_name[IFNAMSIZ - 1] = '\0';

	interface_inaddr_init(&ifr.ifr_addr, addr);
	if (ioctl(sock, SIOCSIFADDR, &ifr) < 0){
		close(sock);
		return -1;
	}

	close(sock);

	return 0;
}

int interface_get_mask(const char *if_name, unsigned int *mask)
{
	int sock = -1;
	struct ifreq ifr;

	if (if_name == NULL || mask == NULL){
		return -1;
	}

	sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (sock < 0){
		return -1;
	}

	memset(&ifr, 0x00, sizeof(struct ifreq));
	strncpy(ifr.ifr_name, if_name, IFNAMSIZ);
	ifr.ifr_name[IFNAMSIZ - 1] = '\0';

	if(ioctl(sock, SIOCGIFNETMASK, &ifr) < 0) {
		*mask = 0;
		close(sock);
		return -1;
	}

	if (mask != NULL) {
		*mask = ((struct sockaddr_in*) &ifr.ifr_addr)->sin_addr.s_addr;
	}

	close(sock);

	return 0;
}


int interface_set_mask(const char *ifname, unsigned int mask)
{
	int sock = -1;
	struct ifreq ifr;

	if (ifname == NULL){
		return -1;
	}

	sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (sock < 0) {
		return -1;
	}

	memset(&ifr, 0x00, sizeof(struct ifreq));
	strncpy(ifr.ifr_name, ifname, IFNAMSIZ);
	ifr.ifr_name[IFNAMSIZ - 1] = '\0';

	interface_inaddr_init(&ifr.ifr_addr, mask);
	if (ioctl(sock, SIOCSIFNETMASK, &ifr) < 0){
		close(sock);
		return -1;
	}

	close(sock);

	return 0;
}

int interface_remove_default_route(const char *ifname)
{
	int sock = -1;
	struct rtentry rt;

	sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (sock < 0) {
		return -1;
	}

	memset(&rt, 0, sizeof(rt));
	rt.rt_dev = (void *)ifname;
	rt.rt_flags = RTF_UP | RTF_GATEWAY;
	interface_inaddr_init(&rt.rt_dst, 0);

	if (ioctl(sock, SIOCDELRT, &rt) < 0) {
		close(sock);
		return -1;
	}

	close(sock);

	return 0;
}

int interface_add_host_route(const char *ifname, unsigned int addr)
{
	int sock = -1;
	struct rtentry rt;

	sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (sock < 0) {
		return -1;
	}


	memset(&rt, 0, sizeof(rt));

	rt.rt_dst.sa_family = AF_INET;
	rt.rt_flags = RTF_UP | RTF_GATEWAY;
	rt.rt_dev = (void*) ifname;
	interface_inaddr_init(&rt.rt_genmask, 0);
	interface_inaddr_init(&rt.rt_gateway, addr);

	if (ioctl(sock, SIOCADDRT, &rt) < 0) {
		close(sock);
		return -1;
	}

	close(sock);

	return 0;
}

int interface_get_default_route(char *ifname, unsigned int *gw)
{
	FILE *fp;
	char name[64];
	unsigned int dst = 0;
	unsigned int gateway = 0;
	unsigned int mask = 0;
	int flags, refcnt, use, metric, mtu, win, irtt;

	if (ifname == NULL){
		return -1;
	}

	fp = fopen("/proc/net/route", "r");
	if (fp == NULL){
		return -1;
	}

	/* skip the header line */
	if (fscanf(fp, "%*[^\n]\n") < 0) {
		fclose(fp);
		return -1;
	}

	gateway = 0;
	for (;;) {
		int nread = fscanf(fp, "%63s%X%X%X%d%d%d%X%d%d%d\n",
				name, &dst, &gateway, &flags,
				&refcnt, &use, &metric, &mask,
				&mtu, &win, &irtt);
		if (nread != 11) {
			break;
		}

		if ((flags & (RTF_UP | RTF_GATEWAY)) == (RTF_UP | RTF_GATEWAY)
				&& dst == 0
				&& strcmp(ifname, name) == 0) {
			break;
		}
	}

	if (gw != NULL) {
		*gw = gateway;
	}

	fclose(fp);

	return 0;
}

