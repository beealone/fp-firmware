#ifndef _WIRELESS_H
#define _WIRELESS_H

#define PRIVATE_PACKET_SIZ (100+7)
#define PACKET_CHANNEL_SIZ 4
#define PACKET_SSID_SIZ 33
#define PACKET_BSSID_SIZ 20
#define PACKET_SECURITY_SIZ 23
#define PACKET_RSSI_SIZ 9
#define PACKET_W_MODE	8
#define PACKET_EXTCH_SIZ 7
#define PACKET_NETWORK_SIZ 3

typedef enum {
	AUTH_OPEN,
	AUTH_SHARED,
	AUTH_WEPAUTO,
	AUTH_WPAPSK,
	AUTH_WPA2PSK,
	AUTH_WPA1PSKWPA2PSK,
	AUTH_UNKNOWN
} auth_type;

typedef enum {
	ENC_NONE,
	ENC_WEP,
	ENC_TKIP,
	ENC_AES,
	ENC_TKIPAES,
	ENC_UNKNOWN,
} enc_type;

typedef enum {
	NET_INFRA,
	NET_ADHOC,
	NET_UNKNOWN,
} net_type;

typedef struct {
	char channel;
	int rssi;
	char ssid[PACKET_SSID_SIZ];
	char bssid[PACKET_BSSID_SIZ];
	enc_type enc;
	auth_type auth;
	net_type net;
} iw_node;

typedef struct {
	int num;
	iw_node *node;
} iw_list;

int wireless_set_private(char *ifname, char *args);
iw_list* new_wireless_scan(char *ifname);
int wireless_connect(iw_list *list, char *ifname, char *ssid, char *passwd);
int wireless_stat(char *ifname, char *ssid, char *bssid);
void wireless_free_results(iw_list *list);
#endif
