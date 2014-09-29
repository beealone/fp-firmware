/* Board/System/Debug information/definition ---------------- */

#define DM9000_ID		0x90000A46

#define DM9000_REG00		0x00
#define DM9000_REG05		0x30	/* SKIP_CRC/SKIP_LONG */
#define DM9000_REG08		0x27
#define DM9000_REG09		0x38
#define DM9000_REG0A		0xff 
#define DM9000_REGFF		0x83	/* IMR */

#define DM9000_PHY		0x40	/* PHY address 0x01 */
#define DM9000_PKT_MAX		1536	/* Received packet max size */
#define DM9000_PKT_RDY		0x01	/* Packet ready to receive */
#define DM9000_MIN_IO		0x0C000000
#define DM9000_MAX_IO		0x0C000070
#define DM9000_INT_MII		0x00
#define DM9000_EXT_MII		0x80

#define CONFIG_DM991X_BASE DM9000_MIN_IO

#define DM9000_VID_L		0x28
#define DM9000_VID_H		0x29
#define DM9000_PID_L		0x2A
#define DM9000_PID_H		0x2B

#define DM9801_NOISE_FLOOR	0x08
#define DM9802_NOISE_FLOOR	0x05

#define DMFE_SUCC       	0
#define MAX_PACKET_SIZE 	1514
#define DMFE_MAX_MULTICAST 	14

#define DM9000_RX_INTR		0x01
#define DM9000_TX_INTR		0x02
#define DM9000_OVERFLOW_INTR	0x04

#define DM9000_DWORD_MODE	1
#define DM9000_BYTE_MODE	2
#define DM9000_WORD_MODE	0

#define TRUE			1
#define FALSE			0

#define DMFE_TIMER_WUT  jiffies+(HZ*2)	/* timer wakeup time : 2 second */
#define DMFE_TX_TIMEOUT (HZ*2)		/* tx packet time-out time 1.5 s" */

enum DM9000_PHY_mode {
	DM9000_10MHD   = 0, 
	DM9000_100MHD  = 1, 
	DM9000_10MFD   = 4,
	DM9000_100MFD  = 5, 
	DM9000_AUTO    = 8, 
	DM9000_1M_HPNA = 0x10 
};

enum DM9000_NIC_TYPE {
	FASTETHER_NIC = 0, 
	HOMERUN_NIC   = 1, 
	LONGRUN_NIC   = 2 
};

/* Structure/enum declaration ------------------------------- */
typedef struct board_info {

	U32 runt_length_counter;	/* counter: RX length < 64byte */ 
	U32 long_length_counter;	/* counter: RX length > 1514byte */ 
	U32 reset_counter;		/* counter: RESET */ 
	U32 reset_tx_timeout;		/* RESET caused by TX Timeout */ 
	U32 reset_rx_status;		/* RESET caused by RX Statsus wrong */ 

	U16 ioaddr;			/* Register I/O base address */
	U16 io_data;			/* Data I/O address */
	U16 irq;			/* IRQ */

	U16 tx_pkt_cnt;
	U16 queue_pkt_len;
	U16 queue_start_addr;
	U16 dbug_cnt;
	U8 reg0, reg5, reg8, reg9, rega;/* registers saved */
	U8 op_mode;			/* PHY operation mode */
	U8 io_mode;			/* 0:word, 2:byte */
	U8 phy_addr;
	U8 link_failed;			/* Ever link failed */
	U8 device_wait_reset;		/* device state */
	U8 nic_type;			/* NIC type */
//	struct timer_list timer;
//	struct net_device_stats stats;
	unsigned char srom[128];
//	spinlock_t lock;
} board_info_t;

int dm9_test(void);

