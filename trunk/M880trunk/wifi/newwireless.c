#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <time.h>
#include <ifaddrs.h>
#include <crypt.h>
#include <sys/time.h>
#include <sys/types.h>
#include <linux/sockios.h>
//#include <linux/wireless.h>
#include<sys/ioctl.h>

#include "iwlib.h"
#include "wireless.h"

#include "newwireless.h"


#define PRIVATE_COMMAND_DEFNUM 32
#define WIRELESS_IOMIN_BUFSIZ 64
#define WIRELESS_IOMAX_BUFSIZ 1024
#define IWIO_BUFSIZ 4096

static void skip_white_space(char **p_cur)
{
	if (*p_cur == NULL) {
		return;
	}

	while (**p_cur != '\0' && **p_cur == ' ') {
		(*p_cur)++;
	}
}

static int skip_next_backslant(char **p_cur)
{
	if (*p_cur == NULL) {
		return -1;
	}

	*p_cur = strchr(*p_cur, '/');

	if (*p_cur == NULL) {
		return -1;
	}

	(*p_cur)++;

	return 0;

}

static int skip_next_colon(char **p_cur)
{
	if (*p_cur == NULL) {
		return -1;
	}

	*p_cur = strchr(*p_cur, ':');

	if (*p_cur == NULL) {
		return -1;
	}

	(*p_cur)++;

	return 0;
}

static int strstartwith(const char *line, const char *prefix)
{
	/*1--connected, 0--disconnect*/
	for (; *line != '\0' && *prefix != '\0'; line++, prefix++) {
		if (*line != *prefix) {
			return 0;
		}
	}

	return ((*prefix == '\0') ? 1:0);
}

/*
static int decode_rssi(char* rssi)
{
	int res = 0;
	int value = 0;
	char *pos = rssi;

	skip_white_space(&pos);
	value = atoi(pos);
	if (value > -51) {
		res = 100;
	} else if (value > -81) {
		res = (unsigned int)(24 + ((value + 80) * 26)/10);
	} else if (value > -91) {
		res = (unsigned int)(((value + 90) * 26)/10);
	} else {
		res = 0;
	}

	return res;
}
*/


static void decode_security(char *str, auth_type *auth, enc_type *enc)
{
	char *pos = NULL;

	pos = str;
	skip_white_space(&pos);
	if (strstartwith(pos, "NONE") != 0){
		*auth = AUTH_OPEN;
		*enc = ENC_NONE;
		return;
	} 

	if (strstartwith(pos, "WEP") != 0){
		*auth = AUTH_SHARED;
		*enc = ENC_WEP;
		return;
	}

	if (strstartwith(pos, "OPEN") != 0) {
		*auth = AUTH_OPEN;
	} else if (strstartwith(pos, "WEPAUTO") != 0) {
		*auth = AUTH_WEPAUTO;
	} else if (strstartwith(pos, "WPA2PSK") != 0) {
		*auth = AUTH_WPA2PSK;
	} else if (strstartwith(pos, "WPAPSK") != 0) {
		*auth = AUTH_WPAPSK;
	} else if (strstartwith(pos, "WPA1PSKWPA2PSK") != 0) {
		*auth = AUTH_WPA1PSKWPA2PSK;
	} else {
		*auth = AUTH_UNKNOWN;
		*enc = ENC_UNKNOWN;
		return;
	}

	if (skip_next_backslant(&pos) < 0) {
		*enc = ENC_UNKNOWN;
		return;
	}

	if (strstartwith(pos, "WEP") != 0) {
		*enc = ENC_WEP;
	} else if (strstartwith(pos, "TKIPAES") != 0) {
		*enc = ENC_TKIPAES;
	} else if (strstartwith(pos, "TKIP") != 0) {
		*enc = ENC_TKIP;
	} else if (strstartwith(pos, "AES") != 0) {
		*enc = ENC_AES;
	} else {
		*enc = ENC_UNKNOWN;
	}
}

static net_type decode_network_type(char *str)
{
	char *pos = str;
	net_type type = NET_UNKNOWN;

	skip_white_space(&pos);
	if (strstartwith(pos, "In") != 0) {
		type = NET_INFRA;
	}else if (strstartwith(pos, "Ad") != 0) {
		type = NET_ADHOC;
	}

	return type;
}

static void strim_white_space(char *data, int len)
{
	int i;

	i = 0;
	while (i < len - 1) {
		if (data[i] == ' ') {
			break;
		}
		i++;
	}

	data[i] = '\0';
}

iw_list* decode_scan_results(char *data, int len)
{
	int i = 0;
	int num = 0;
	char *pos = NULL;
	char buf[WIRELESS_IOMIN_BUFSIZ];
	iw_node *node = NULL;
	iw_list *list = NULL;

	if (data == NULL || len < PRIVATE_PACKET_SIZ) {
		return NULL;
	}

	list = (iw_list *)malloc(sizeof(iw_list));
	if (list == NULL) {
		return NULL;
	}

	num = len / PRIVATE_PACKET_SIZ - 1;
	list->node = (iw_node *)malloc(num * sizeof(iw_node));
	if (list->node == NULL) {
		free(list);
		list=NULL;
		return NULL;
	}
	list->num = num;

	i = 0;
	pos = data + PRIVATE_PACKET_SIZ + 1;
	node = list->node;
	while (i < num) {
		if (pos + PRIVATE_PACKET_SIZ > data + len) {
			break;
		}

		memset(buf, 0x00, sizeof(buf));
		memcpy(buf, pos, PACKET_CHANNEL_SIZ);
		node[i].channel = atoi(buf);
		pos += PACKET_CHANNEL_SIZ;

		memset(node[i].ssid, 0x00, sizeof(node[i].ssid));
		memcpy(node[i].ssid, pos, PACKET_SSID_SIZ);
		strim_white_space(node[i].ssid, sizeof(node[i].ssid));
		pos += PACKET_SSID_SIZ;

		memset(node[i].bssid, 0x00, sizeof(node[i].bssid));
		memcpy(node[i].bssid, pos, PACKET_BSSID_SIZ);
		strim_white_space(node[i].bssid, sizeof(node[i].bssid));
		pos += PACKET_BSSID_SIZ;

		memset(buf, 0, sizeof(buf));
		memcpy(buf, pos, PACKET_SECURITY_SIZ);
		decode_security(buf, &node[i].auth, &node[i].enc);
		pos += PACKET_SECURITY_SIZ;

		memset(buf, 0, sizeof(buf));
		memcpy(buf, pos, PACKET_RSSI_SIZ);
		node[i].rssi = atoi(buf);
		pos += (PACKET_EXTCH_SIZ + PACKET_RSSI_SIZ+PACKET_W_MODE);

		memset(buf, 0, sizeof(buf));
		memcpy(buf, pos, PACKET_NETWORK_SIZ);
		node[i].net = decode_network_type(buf);
		pos += PACKET_NETWORK_SIZ;

		i++;
	}

	return list;
}

static struct iw_priv_args* wireless_get_private_command(int sock, char *ifname, int *num)
{
	int max;
	struct iwreq req;
	struct iw_priv_args *priv = NULL;

	if (sock < 0 || ifname == NULL || num == NULL) {
		return NULL;
	}

	max = PRIVATE_COMMAND_DEFNUM;
	while (max < 1024) {
		priv = (struct iw_priv_args *)realloc(priv, max * sizeof(struct iw_priv_args));
		if (priv == NULL) {
			return NULL;
		}

		strncpy(req.ifr_name, ifname, IFNAMSIZ);
		req.u.data.pointer = (caddr_t) priv;
		req.u.data.length = max;
		req.u.data.flags = 0;
		if (ioctl(sock, SIOCGIWPRIV, &req) < 0) {
			if (errno != E2BIG) {
				break;
			} 
		} else {
			*num = req.u.data.length;
			return priv;
		}

		if (req.u.data.length > max) {
			max = req.u.data.length;
		} else {
			max *= 2;
		}
	}

	if (priv != NULL) {
		free(priv);
		priv = NULL;
	}

	return NULL;
}

static const int priv_type_size[] = {
	0,							/* IW_PRIV_TYPE_NONE */
	1,							/* IW_PRIV_TYPE_BYTE */
	1,							/* IW_PRIV_TYPE_CHAR */
	0,							/* Not defined */
	sizeof(__u32),				/* IW_PRIV_TYPE_INT */
	sizeof(struct iw_freq),     /* IW_PRIV_TYPE_FLOAT */
	sizeof(struct sockaddr),    /* IW_PRIV_TYPE_ADDR */
	0,							/* Not defined */
};

static int wireless_get_priv_size(int args)
{
	int num = args & IW_PRIV_SIZE_MASK;
	int type = (args & IW_PRIV_TYPE_MASK) >> 12;

	return (num * priv_type_size[type]);
}

int wireless_set_private(char *ifname, char *args)
{
	int index;
	int num = 0;
	int sock = -1;
	char buffer[IWIO_BUFSIZ];
	struct iwreq req;
	struct iw_priv_args *priv = NULL;

	if (ifname == NULL || args == NULL) {
		return -1;
	}

	sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (sock < 0) {
		return -1;
	}

	priv = wireless_get_private_command(sock, ifname, &num);
	if (priv == NULL) {
		close(sock);
		return -1;
	}

	index = 0;
	while (index < num) {
		if (strcmp(priv[index].name, "set") == 0) {
			break;
		}
		index++;
	}

	if (num == index) {
		close(sock);
		free(priv);
		priv=NULL;
		return -1;
	}

	memset(buffer, 0x00, sizeof(buffer));
	if ((priv[index].set_args & IW_PRIV_TYPE_MASK)
			&& (priv[index].set_args & IW_PRIV_SIZE_MASK)) {
		if ((priv[index].set_args & IW_PRIV_TYPE_MASK) == IW_PRIV_TYPE_CHAR) {
			req.u.data.length = strlen(args) + 1;
			if (req.u.data.length > (priv[index].set_args & IW_PRIV_SIZE_MASK)) {
				req.u.data.length = priv[index].set_args & IW_PRIV_SIZE_MASK;
			}
			memcpy(buffer, args, req.u.data.length);
			buffer[sizeof(buffer) - 1] = '\0';
		}
	} else {
		free(priv);
		priv=NULL;
		close(sock);
		return -1;
	}

	if ((priv[index].set_args & IW_PRIV_SIZE_FIXED) &&
			wireless_get_priv_size(priv[index].set_args) <= IFNAMSIZ) {
		memcpy(req.u.name, buffer, IFNAMSIZ);
	} else {
		req.u.data.pointer = (caddr_t)buffer;
		req.u.data.flags = 0;
	}

	strncpy(req.ifr_name, ifname, IFNAMSIZ);
	if (ioctl(sock, priv[index].cmd, &req) < 0) {
		free(priv);
		priv=NULL;
		close(sock);
		return -1;
	}
	
	free(priv);
	priv=NULL;
	close(sock);

	return 0;	
}

iw_list* new_wireless_scan(char *ifname)
{
	int index;
	int num = 0;
	int sock = -1;
	char buffer[IWIO_BUFSIZ] ;
	struct iwreq req;
	struct iw_priv_args *priv = NULL;
	iw_list *list = NULL;

	if (ifname == NULL) {
		return NULL;
	}

	sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (sock < 0) {
		return NULL;
	}

	priv = wireless_get_private_command(sock, ifname, &num);
	if (priv == NULL) {
		close(sock);
		return NULL;
	}

	index = 0;
	while (index < num) {
		if (strcmp(priv[index].name, "get_site_survey") == 0) {
			break;
		}
		index++;
	}

	if (num == index) {
		close(sock);
		free(priv);
		priv = NULL;
		return NULL;
	}

	memset(buffer, 0x00, sizeof(buffer));
	req.u.data.length = 0L;
	req.u.data.pointer = (caddr_t)buffer;
	req.u.data.flags = 0;
	strncpy(req.ifr_name, ifname, IFNAMSIZ);
	if (ioctl(sock, priv[index].cmd, &req) < 0) {
		free(priv);
		close(sock);
		priv=NULL;
		return NULL;
	}

	if((priv[index].get_args & IW_PRIV_TYPE_MASK) &&
			(priv[index].get_args & IW_PRIV_SIZE_MASK)) {
		int len;
		char *data=NULL;
		if ((priv[index].get_args & IW_PRIV_SIZE_FIXED) && (wireless_get_priv_size(priv[index].get_args) <= IFNAMSIZ)){
			data = req.u.name;
			len = priv[index].get_args & IW_PRIV_SIZE_MASK;
		} else {
			data = buffer;
			len = req.u.data.length;
		}

		list = decode_scan_results(data, len);
		if (list == NULL) {
			free(priv); //dsl
			priv=NULL;
			close(sock);
			return NULL;
		}
	}

	free(priv);
	priv=NULL;

	close(sock);

	return list;	
}

static int decode_connect_status(char *str, int len, char *ifname, char *ssid, char *bssid)
{
	char *pos = NULL;
	char *end = NULL;

	if (str == NULL || len < 0 || ifname == NULL) {
		return 0;
	}

	pos = str;
	skip_white_space(&pos);	
	if (strstartwith(pos, "Connected") == 0) {
		return 0;
	}

	if (ssid != NULL && bssid != NULL) {
		skip_next_colon(&pos);
		if(pos == NULL) {
			return 0;
		}
		skip_white_space(&pos);
		end = pos;
		while (end < str + len) {
			if (*end == '[') {
				memcpy(ssid, pos, end - pos);
				ssid[end - pos] = '\0';
				break;
			}
			end++;
		}

		if (*end++ == '[') {
			pos = end;
			while (end < str + len) {
				if (*end == ']') {
					memcpy(bssid, pos, end - pos);
					bssid[end - pos] = '\0';
					break;
				}
				end++;
			}
		}
	}

	return 1; /*connected*/
}

int wireless_stat(char *ifname, char *ssid, char *bssid)
{
	int index;
	int num = 0;
	int sock = -1;
	int subcmd = 0;
	int status = 0;
	char buffer[IWIO_BUFSIZ] ;
	struct iwreq req;
	struct iw_priv_args *priv = NULL;

	if (ifname == NULL) {
		return 0;
	}

	sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (sock < 0) {
		return 0;
	}

	priv = wireless_get_private_command(sock, ifname, &num);
	if (priv == NULL) {
		close(sock);
		return 0;
	}

	index = 0;
	while (index < num) {
		if (strcmp(priv[index].name, "connStatus") == 0) {
			break;
		}
		index++;
	}

	if(priv[index].cmd < SIOCDEVPRIVATE) {
		int	i = 0;

		while (i < num) {
			if (priv[i].name[0] != '\0' || priv[i].set_args != priv[index].set_args || 
					priv[i].get_args != priv[index].get_args) {
				i++;
			} else {
				break;
			}
		}

		if(i == num) {
			close(sock);
			free(priv);
			priv=NULL;
			return 0;
		}

		subcmd = priv[index].cmd;
		index = i;
	}

	if (num == index) {
		close(sock);
		free(priv);
		priv=NULL;
		return 0;
	}

	memset(buffer, 0x00, sizeof(buffer));
	req.u.data.length = 0L;
	req.u.data.pointer = (caddr_t)buffer;
	req.u.data.flags = subcmd;
	strncpy(req.ifr_name, ifname, IFNAMSIZ);
	if (ioctl(sock, priv[index].cmd, &req) < 0) {
		free(priv);
		priv=NULL;
		close(sock);
		return 0;
	}

	if((priv[index].get_args & IW_PRIV_TYPE_MASK) &&
			(priv[index].get_args & IW_PRIV_SIZE_MASK)) {
		int len;
		char *data;
		if ((priv[index].get_args & IW_PRIV_SIZE_FIXED) && (wireless_get_priv_size(priv[index].get_args) <= IFNAMSIZ)){
			data = req.u.name;
			len = priv[index].get_args & IW_PRIV_SIZE_MASK;
		} else {
			data = buffer;
			len = req.u.data.length;
		}

		status = decode_connect_status(data, len, ifname, ssid, bssid);
	}

	free(priv);
	priv=NULL;

	close(sock);

	return status;	
}

int wireless_connect(iw_list *list, char *ifname, char *ssid, char *passwd)
{
	int i = 0;
	char buf[WIRELESS_IOMIN_BUFSIZ] = {0};
	char pwd[WIRELESS_IOMAX_BUFSIZ] = {0};

	//printf("____%s%d___ifname=%s,ssid=%s,passwd=%s\n", __FILE__, __LINE__, ifname, ssid, passwd);
	if (list == NULL || ifname == NULL || ssid == NULL) {
		return -1;
	}

	i = 0;
	while (i < list->num) {
		if (strcmp(list->node[i].ssid, ssid) == 0) {
			break;
		}
		i++;
	}

	if (i == list->num) {
		wireless_set_private(ifname, "SiteSurvey=1");
		return -1;
	}

	if (list->node[i].net == NET_ADHOC) {
		strcpy(buf, "NetworkType=Adhoc");
	} else {
		strcpy(buf, "NetworkType=Infra");
	}
	if (wireless_set_private(ifname, buf) < 0) {
		return -1;
	}

	sprintf(buf, "SSID=%s", ssid);
	if (wireless_set_private(ifname, buf) < 0) {
		return -1;
	}

	switch (list->node[i].auth) {
		case AUTH_OPEN:
			sprintf(buf, "AuthMode=OPEN");
			break;
			
		case AUTH_SHARED:
			sprintf(buf, "AuthMode=SHARED");
			break;

		case AUTH_WEPAUTO:
			sprintf(buf, "AuthMode=WEPAUTO");
			break;

		case AUTH_WPAPSK:
		case AUTH_WPA1PSKWPA2PSK:
			sprintf(buf, "AuthMode=WPAPSK");
			break;

		case AUTH_WPA2PSK:
			sprintf(buf, "AuthMode=WPA2PSK");
			break;

		default:
			sprintf(buf, "AuthMode=SHARED");
			break;
	}
	if (wireless_set_private(ifname, buf) < 0) {
		return -1;
	}

	switch (list->node[i].enc) {
		case ENC_WEP:
			sprintf(buf, "EncrypType=WEP");
			break;

		case ENC_AES:
			 sprintf(buf, "EncrypType=AES");
			 break;

		case ENC_TKIP:
		case ENC_TKIPAES:
			 sprintf(buf, "EncrypType=TKIP");
			 break;

		default:
			 sprintf(buf, "EncrypType=NONE");
			 break;
	}
	if (wireless_set_private(ifname, buf) < 0) {
		return -1;
	}

	if (strlen(passwd) > 0) {
		if (list->node[i].auth == AUTH_SHARED || list->node[i].auth == AUTH_WEPAUTO) {
			if (wireless_set_private(ifname, "DefaultKeyID=1") < 0) {
				return -1;
			}
			sprintf(pwd, "Key1=%s", passwd);
		} else if (list->node[i].auth == AUTH_WPAPSK || list->node[i].auth == AUTH_WPA2PSK ||
				list->node[i].auth == AUTH_WPA1PSKWPA2PSK) {
			sprintf(pwd, "WPAPSK=%s", passwd);
		}
		
		if (strlen(pwd) > 0 && wireless_set_private(ifname, pwd) < 0) {
			return -1;
		}
		//printf("____%s%d___ifname=%s,ssid=%s,pwd=%s\n", __FILE__, __LINE__, ifname, ssid, pwd);
	}

	return 0;
}

void wireless_free_results(iw_list *list)
{
	if (list == NULL) {
		return;
	}

	if (list->node) {
		free(list->node);
		list->node = NULL;
	}

	free(list);
}


