#ifndef __UPEK_H__
#define __UPEK_H__

#define GCC_PACKED __attribute__((packed))
#define GCC_ALIGN0 __attribute__((aligned(1)))

#ifndef NULL
#ifdef __cplusplus
#define NULL 0
#else
#define NULL ((void *)0)
#endif
#endif

#define uint8 unsigned char
#define uint16 unsigned short
#define uint32 unsigned int
#define sint8 signed char
#define sint16 signed short
#define sint32 signed long
//#define BOOL  signed long

typedef struct SENSOR_DATA {
uint8 SensorVersion; // code identifying the sensor type
uint16 ArrayWidth; // width of the sensor pixel array
uint16 ArrayHeight; // height of the sensor pixel array
} SENSOR_DATA, *PSENSOR_DATA;

typedef enum {
NOMINAL_MODE = 1,
STANDBY_MODE,
SLEEP_MODE
} TCPowerState;

typedef enum {
TCPwr_Void = 0, // uninitialized
TCPwr_OK, // After sensor power has been applied. Normal condition
TCPwr_Off, // After normal sensor power removal.
TCPwr_Latchup // After an overcurrent event. (The power is off.)
} TCPwrSupply;

#define FALSE 0
#define TRUE 1

#define TCS1A 37
#define TCS1C 38
#define TCS2 27

#define STERR_OK 0
#define STERR_ERROR 1
#define STERR_NODEVICE -3
#define STERR_TCPWR -6
#define STERR_BADPARAMETER -5
#define STERR_BUFFER_TOO_SMALL -20

sint32 TCDVDR_ReadRegister (uint8 AddrPort,uint32 * Data);
sint32 TCDVDR_WriteRegister (uint8 AddrPort,uint32 Data);
sint32 TCDVDR_GrabImage (uint8 * Imgbuf,sint32 BufSize) ;
sint32 TCDVDR_Sleep (uint32 ulDelay);
sint32 TCDVDR_NVMRead (uint8 * ptr,uint32 Offset,uint32 Length);
sint32 TCDVDR_NVMWrite (uint8 * ptr,uint32 Offset,uint32 Length);
sint32 TCDVDR_Init ();
sint32 TCDVDR_power_off_TC ();
sint32 TCDVDR_power_on_TC();
sint32 STInitialize (SENSOR_DATA * Data);
sint32 STAuthentify (sint32 * VendorCode);
void STTerminate();
sint32 STGrab (uint8 * ImgBuf,sint32 NumberofRow,sint32 RowOffset,sint32 DeltaRow,
sint32 NumberofCol,sint32 ColOffset,sint32 DeltaCol);
sint32 STInitSensorPar (sint32 SelSet) ;
sint32 STSetChipState (TCPowerState STATE);

sint32 STGetTCIRevision (sint8 * i_pTCIRev ,sint8 * i_szDesc ,sint32 * DescBufSize);
sint32 STQueryDevice ();
sint32 STGetCurrentSetting ();
sint32 STGetTotalSettings ();
sint32 STGetChipState ( );
sint32 STNVMRead (sint32 NVMaddr,sint32 NumBytes,uint8 * Buffer);
sint32 STNVMWrite(sint32 NVMaddr,sint32 NumBytes,uint8 * Buffer);

#endif

