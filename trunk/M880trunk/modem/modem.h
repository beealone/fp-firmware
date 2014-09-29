#ifndef _MODEM_H
#define _MODEM_H
typedef enum {
	MODEM_STATE_UNAVAILABLE = 0x00,
	MODEM_STATE_INITIALIZING,
	MODEM_STATE_INIT_FAILED,
	MODEM_STATE_READY,
	MODEM_STATE_SIM_NOT_READY,
	MODEM_STATE_SIM_ABSENT,
	MODEM_STATE_SIM_LOCKED,
	MODEM_STATE_SIM_READY,
	MODEM_STATE_REQINDICATOR,
	MODEM_STATE_CONNECTING,
	MODEM_STATE_CONNECT_FAILED,
	MODEM_STATE_CONNECTED,
	MODEM_STATE_DISCONNECT
} modem_state_t;

typedef enum {
	SIG_LEVEL_NON = 0x00,
	SIG_LEVEL_1,
	SIG_LEVEL_2,
	SIG_LEVEL_3,
	SIG_LEVEL_4,
	SIG_LEVEL_5
} signal_indicator_t;

typedef enum {
	MODEM_BAND_850 = 0x00,
	MODEM_BAND_900,
	MODEM_BAND_1800,
	MODEM_BAND_1900,
	MODEM_BAND_850_1900,
	MODEM_BAND_900_1800,
	MODEM_BAND_900_1900,
	MODEM_BAND_HYBRID,
	MODEM_BAND_DEFAULT,
} modem_band_t;

typedef enum {
	MODEM_MODULE_UNKNOWN = 0x00,
	MODEM_MODULE_Q24PL,
	MODEM_MODULE_EM560,
	MODEM_MODULE_EM660,
	MODEM_MODULE_EM770,
} modem_module_t;

typedef enum {
	MODEM_HDRMODE_NONE = 0x00,
	MODEM_HDRMODE_CDMA = 0x02,
	MODEM_HDRMODE_HDR = 0x04,
	MODEM_HDRMODE_HYBRID = 0x08,
	MODEM_HDRMODE_UNKNOWN
} modem_hdrmode_t;

typedef enum {
	MODEM_MODE_NONE = 0x00,
	MODEM_MODE_GSM,
	MODEM_MODE_GPRS,
	MODEM_MODE_EDGE,
	MODEM_MODE_WCDMA,
	MODEM_MODE_HSDPA,
	MODEM_MODE_HSUPA,
	MODEM_MODE_HSUPAHSDPA,
	MODEM_MODE_TDSCDMA,
	MODEM_MODE_UNKNOWN
} modem_mode_t;

void modem_process(char *apn, char *dialnumber, char *username, char *password, char *htserver, 
	modem_module_t module, modem_band_t band, void (*modem_power_func)(int));
void modem_disable(void);
void modem_reconnect(void);
int is_modem_online(void);
int modem_stats(int *inbytes, int *outbytes, char *ipaddr);
long modem_connect_time(void);
modem_state_t get_modem_current_state(void);
signal_indicator_t get_modem_signal_indicator(void);
int get_modem_mode(void);
const char *modem_get_version(void);
#endif
