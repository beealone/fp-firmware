/*
 *	Wireless Tools
*/

#include "iwlib.h"		/* Header */
#include <sys/time.h>
#include "arca.h"

/****************************** TYPES ******************************/
// Scan state and meta-information, used to decode events...
typedef struct iwscan_state
{
	int			ap_num;		/* Access Point number 1->N */
	int			val_index;	/* Value in table 0->(N-1) */
} iwscan_state;

// Bit to name mapping
typedef struct iwmask_name
{
	unsigned int	mask;	/* bit mask for the value */
	const char *	name;	/* human readable name for the value */
} iwmask_name;


// Types of authentication parameters
typedef struct iw_auth_descr
{
	int				value;		/* Type of auth value */
	const char *			label;		/* User readable version */
	const struct iwmask_name *	names;		/* Names for this value */
	const int			num_names;	/* Number of names */
} iw_auth_descr;

/**************************** CONSTANTS ****************************/
#define IW_SCAN_HACK		0x8000
#define IW_EXTKEY_SIZE	(sizeof(struct iw_encode_ext) + IW_ENCODING_TOKEN_MAX)

/* ------------------------ WPA CAPA NAMES ------------------------ */
/*
 * This is the user readable name of a bunch of WPA constants in wireless.h
 * Maybe this should go in iwlib.c ?
 */

#ifndef WE_ESSENTIAL
#define IW_ARRAY_LEN(x) (sizeof(x)/sizeof((x)[0]))

//static const struct iwmask_name iw_enc_mode_name[] = {
//  { IW_ENCODE_RESTRICTED,	"restricted" },
//  { IW_ENCODE_OPEN,		"open" },
//};
//#define	IW_ENC_MODE_NUM		IW_ARRAY_LEN(iw_enc_mode_name)

static const struct iwmask_name iw_auth_capa_name[] = {
	{ IW_ENC_CAPA_WPA,		"WPA" },
	{ IW_ENC_CAPA_WPA2,		"WPA2" },
	{ IW_ENC_CAPA_CIPHER_TKIP,	"CIPHER-TKIP" },
	{ IW_ENC_CAPA_CIPHER_CCMP,	"CIPHER-CCMP" },
};
#define	IW_AUTH_CAPA_NUM	IW_ARRAY_LEN(iw_auth_capa_name)

static const struct iwmask_name iw_auth_cypher_name[] = {
	{ IW_AUTH_CIPHER_NONE,	"none" },
	{ IW_AUTH_CIPHER_WEP40,	"WEP-40" },
	{ IW_AUTH_CIPHER_TKIP,	"TKIP" },
	{ IW_AUTH_CIPHER_CCMP,	"CCMP" },
	{ IW_AUTH_CIPHER_WEP104,"WEP-104" },
};
#define	IW_AUTH_CYPHER_NUM	IW_ARRAY_LEN(iw_auth_cypher_name)

static const struct iwmask_name iw_wpa_ver_name[] = {
	{ IW_AUTH_WPA_VERSION_DISABLED,	"disabled" },
	{ IW_AUTH_WPA_VERSION_WPA,	"WPA" },
	{ IW_AUTH_WPA_VERSION_WPA2,	"WPA2" },
};
#define	IW_WPA_VER_NUM		IW_ARRAY_LEN(iw_wpa_ver_name)

static const struct iwmask_name iw_auth_key_mgmt_name[] = {
	{ IW_AUTH_KEY_MGMT_802_1X, "802.1x" },
	{ IW_AUTH_KEY_MGMT_PSK,	"PSK" },
};
#define	IW_AUTH_KEY_MGMT_NUM	IW_ARRAY_LEN(iw_auth_key_mgmt_name)

static const struct iwmask_name iw_auth_alg_name[] = {
	{ IW_AUTH_ALG_OPEN_SYSTEM,	"open" },
	{ IW_AUTH_ALG_SHARED_KEY,	"shared-key" },
	{ IW_AUTH_ALG_LEAP,		"LEAP" },
};
#define	IW_AUTH_ALG_NUM		IW_ARRAY_LEN(iw_auth_alg_name)

static const struct iw_auth_descr	iw_auth_settings[] = {
  { IW_AUTH_WPA_VERSION, "WPA version", iw_wpa_ver_name, IW_WPA_VER_NUM },
  { IW_AUTH_KEY_MGMT, "Key management", iw_auth_key_mgmt_name, IW_AUTH_KEY_MGMT_NUM },
  { IW_AUTH_CIPHER_PAIRWISE, "Pairwise cipher", iw_auth_cypher_name, IW_AUTH_CYPHER_NUM },
  { IW_AUTH_CIPHER_GROUP, "Pairwise cipher", iw_auth_cypher_name, IW_AUTH_CYPHER_NUM },
  { IW_AUTH_TKIP_COUNTERMEASURES, "TKIP countermeasures", NULL, 0 },
  { IW_AUTH_DROP_UNENCRYPTED, "Drop unencrypted", NULL, 0 },
  { IW_AUTH_80211_AUTH_ALG, "Authentication algorithm", iw_auth_alg_name, IW_AUTH_ALG_NUM },
  { IW_AUTH_RX_UNENCRYPTED_EAPOL, "Receive unencrypted EAPOL", NULL, 0 },
  { IW_AUTH_ROAMING_CONTROL, "Roaming control", NULL, 0 },
  { IW_AUTH_PRIVACY_INVOKED, "Privacy invoked", NULL, 0 },
};
#define	IW_AUTH_SETTINGS_NUM		IW_ARRAY_LEN(iw_auth_settings)

#if 0
/* Values for the IW_ENCODE_ALG_* returned by SIOCSIWENCODEEXT */
static const char *iw_encode_alg_name[] = {
	"none",
	"WEP",
	"TKIP",
	"CCMP",
	"unknown"
};
#define	IW_ENCODE_ALG_NUM		IW_ARRAY_LEN(iw_encode_alg_name)
#endif

#ifndef IW_IE_CIPHER_NONE
/* Cypher values in GENIE (pairwise and group) */
#define IW_IE_CIPHER_NONE	0
#define IW_IE_CIPHER_WEP40	1
#define IW_IE_CIPHER_TKIP	2
#define IW_IE_CIPHER_WRAP	3
#define IW_IE_CIPHER_CCMP	4
#define IW_IE_CIPHER_WEP104	5
/* Key management in GENIE */
#define IW_IE_KEY_MGMT_NONE	0
#define IW_IE_KEY_MGMT_802_1X	1
#define IW_IE_KEY_MGMT_PSK	2
#endif	/* IW_IE_CIPHER_NONE */
#if 0
/* Values for the IW_IE_CIPHER_* in GENIE */
static const char *iw_ie_cypher_name[] = {
	"none",
	"WEP-40",
	"TKIP",
	"WRAP",
	"CCMP",
	"WEP-104",
};
#define	IW_IE_CYPHER_NUM	IW_ARRAY_LEN(iw_ie_cypher_name)

/* Values for the IW_IE_KEY_MGMT_* in GENIE */
static const char *iw_ie_key_mgmt_name[] = {
	"none",
	"802.1x",
	"PSK",
};
#define	IW_IE_KEY_MGMT_NUM	IW_ARRAY_LEN(iw_ie_key_mgmt_name)
#endif
#endif	/* WE_ESSENTIAL */
/*------------------------------------------------------------------*/
/*
 * Perform a scanning on one device
 */
int print_scanning_info(char *ifname, wireless_scan_result* result)		/**/
{
	int skfd;
	struct iwreq wrq;
	struct iw_scan_req scanopt;	/* Options for 'set' */
	int scanflags = 0;		/* Flags for scan */
	unsigned char *	buffer = NULL;	/* Results */
	int buflen = IW_SCAN_MAX_DATA; 	/* Min for compat WE<17 */
	struct iw_range	range;
	int has_range;
	struct timeval tv;		/* Select timeout */
	int timeout = 15000000;		/* 15s */
        char qbuffer[(sizeof(struct iw_quality) + sizeof(struct sockaddr)) * IW_MAX_AP];
        char temp[128];
        struct sockaddr *hwa;
        struct iw_quality *qual;
        int has_qual = 0;
        int n;
	int t=0;
        int i, j;
	wireless_scan_result* fres=NULL;

        if((skfd = iw_sockets_open()) < 0)
        {
                perror("socket");
                return -1;
        }

	/* Get range stuff */
	has_range = (iw_get_range_info(skfd, ifname, &range) >= 0);
	if((!has_range) || (range.we_version_compiled < 14))
	{
	        iw_sockets_close(skfd);
		return(-1);
	}

	/* Init timeout value -> 250ms between set and first get */
	tv.tv_sec = 0;
	tv.tv_usec = 250000;

	memset(&scanopt, 0, sizeof(scanopt));
	wrq.u.data.pointer = (caddr_t) &scanopt;
	wrq.u.data.length = sizeof(scanopt);
	wrq.u.data.flags = scanflags;

	/* Initiate Scanning */
	if(iw_set_ext(skfd, ifname, SIOCSIWSCAN, &wrq) < 0)
	{
		if((errno != EPERM) || (scanflags != 0))
		{
		        iw_sockets_close(skfd);
			return(-1);
		}
		tv.tv_usec = 0;
	}
	timeout -= tv.tv_usec;

	while(1)
	{
		fd_set		rfds;		/* File descriptors for select */
		int		last_fd;	/* Last fd */
		int		ret;

		FD_ZERO(&rfds);
		last_fd = -1;

		ret = select(last_fd + 1, &rfds, NULL, NULL, &tv);
		if(ret < 0)
		{
			if(errno == EAGAIN || errno == EINTR)
				continue;
		        iw_sockets_close(skfd);
			return(-1);
		}

		if(ret == 0)
		{
			unsigned char *	newbuf;
realloc:
			/* (Re)allocate the buffer - realloc(NULL, len) == malloc(len) */
			newbuf = REALLOC(buffer, buflen);
			if(newbuf == NULL)
			{
				if(buffer) FREE(buffer);
			        iw_sockets_close(skfd);
				return(-1);
			}
			buffer = newbuf;
			/* Try to read the results */
			wrq.u.data.pointer = buffer;
			wrq.u.data.flags = 0;
			wrq.u.data.length = buflen;
			if(iw_get_ext(skfd, ifname, SIOCGIWSCAN, &wrq) < 0)
			{
				/* Check if buffer was too small (WE-17 only) */
				if((errno == E2BIG) && (range.we_version_compiled > 16))
				{
					/* Check if the driver gave us any hints. */
					if(wrq.u.data.length > buflen)
						buflen = wrq.u.data.length;
					else
						buflen *= 2;

					/* Try again */
					goto realloc;
				}

				/* Check if results not available yet */
				if(errno == EAGAIN)
				{
					/* Restart timer for only 100ms*/
					tv.tv_sec = 0;
					tv.tv_usec = 100000;
					timeout -= tv.tv_usec;
					if(timeout > 0)
						continue;	/* Try again later */
				}

				/* Bad error */
				FREE(buffer);
			        iw_sockets_close(skfd);
				return(-2);
			}
			else
				/* We have the results, go to process them */
				break;
		}
	}

	if(wrq.u.data.length)
	{
		struct iw_event		iwe;
		struct stream_descr	stream;
		//struct iwscan_state	state = { .ap_num = 1, .val_index = 0 };
		int			ret;
		
		iw_init_event_stream(&stream, (char *) buffer, wrq.u.data.length);
		fres = result;
		do
		{
			/* Extract an event and print it */
			ret = iw_extract_event_stream(&stream, &iwe, range.we_version_compiled);
			if(ret > 0)
			{
				char adbuf[128];
				if(iwe.cmd == SIOCGIWAP)
					sprintf(fres->ap_address,"%s",iw_saether_ntop(&(iwe.u.ap_addr), adbuf));
				else if(iwe.cmd == SIOCGIWESSID)
				{
		                        char essid[IW_ESSID_MAX_SIZE+1];
		                        memset(essid, '\0', sizeof(essid));
					if((iwe.u.essid.pointer) && (iwe.u.essid.length))
		                                memcpy(essid, iwe.u.essid.pointer, iwe.u.essid.length);
		                        if(iwe.u.essid.flags)
		                        {
		                                if((iwe.u.essid.flags & IW_ENCODE_INDEX) > 1)
							sprintf(fres->ap_name,"%s[%d]",essid,(iwe.u.essid.flags&IW_ENCODE_INDEX));
		                                else
							sprintf(fres->ap_name, "%s", essid);
		                        }
                                        fres++;
                                        t++;
				}
			}
		}
		while(ret > 0);
	}

	//get quality.
        wrq.u.data.pointer = (caddr_t)qbuffer;
        wrq.u.data.length = IW_MAX_AP;
        wrq.u.data.flags = 0;
        if(iw_get_ext(skfd, ifname, SIOCGIWAPLIST, &wrq) < 0)
        {
                return(-1);
        }

        n = wrq.u.data.length;
        has_qual = wrq.u.data.flags;

        hwa = (struct sockaddr *) qbuffer;
        qual = (struct iw_quality *) (qbuffer + (sizeof(struct sockaddr) * n));

        if(iw_check_mac_addr_type(skfd, ifname) < 0)
        {
                return(-2);
        }

        for(i = 0; i < n; i++)
        {
                if(has_qual)
                {
			fres=result;
			for(j=0;j<t;j++)
			{
				memset(temp, 0, sizeof(temp));
				if(!strcmp(fres->ap_address, iw_saether_ntop(&hwa[i], temp)))
				{
		        	        /* Deal with quality : always a relative value */
			                if(!(qual->updated&IW_QUAL_QUAL_INVALID))
						fres->qualvalue = qual[i].qual;
				}
				fres++;
	                }
		}
        }

	FREE(buffer);
        iw_sockets_close(skfd);
	return(t);
}

