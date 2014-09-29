/* I2C device address of OPP03 */
#include "arca.h"

#define UINT32  unsigned int
#define UINT16 unsigned short
#define UINT8 unsigned char

#define TRUE 1
#define FALSE 0

/* registers of OPP03*/
#define REG_MODE_C 0x02
#define REG_TITU 0x25
#define REG_TITM 0x26
#define REG_TITL 0x27
#define REG_GREEN_GAIN 0x32	//Gray Scale Gain Control
#define REG_RSU 0x10
#define REG_RSL 0x11
#define REG_CSU 0x12
#define REG_CSL 0x13
#define REG_WHU 0x14
#define REG_WHL 0x15
#define REG_WWU 0x16
#define REG_WWL 0x17

/* GPIO for OPP03 */
#define G_VSCL 0		//CIM_D0
#define G_VSDA 1		//CIM_D1
#define SDATA3 2		//CIM_D2
#define SDATA2 3		//CIM_D3
#define SDATA1 4		//CIM_D4
#define SDATA0 5		//CIM_D5
#define G_VCLK 6		//CIM_D6
/* GPIO for FPD */
#define FPD_CLK 9		//CIM_HSYNC
#define FPD_DAT 8		//CIM_VSYNC
#define FPD_INT 7		//CIM_D7


/* parameters for OPP03 */
#define OPP_OK 0
#define OPP_ERR_INVALID_IMAGE -1
#define OPP_ERR_TIME_OUT -2
#define OPP_ERROR -3
#define FPD_ERROR -4
#define LINE_VL 0xFFFF00B6
#define LINE_SOF 0xFFFF00C7

//#define QUALITY_DIFF_THRES 50

#define MAX_BRIGHTNESS 60000
#define OPP_BRIGHTNESS 500
#define OPP_GAIN 100
#define OPP_CONTRAST 20

#define FRAME_WIDTH 400
#define FRAME_HEIGHT 300
#define FRAME_SIZE 400*300
#define IMAGE_WIDTH 400
#define IMAGE_HEIGHT 300
#define IMAGE_SIZE 400*300

#define IO_OSP_SetBrightness 0x01

enum
{
  IO_OPP_Initialize = 128,
  IO_OPP_GetLiveImage,
  IO_OPP_FPD_Initialize,
  IO_OPP_Set,
  IO_OPP_Start
};

typedef struct
{
  UINT16 brightness;
  UINT8 gain;
  UINT8 contrast;
} OPP_Initialize_Para;

typedef struct
{
//  UINT8 *pBuff;
  UINT8 nTimeOut;
  UINT8 nQuality;
  UINT8 nDetectInterval;
} OPP_LiveImage_Para;

typedef struct
{
  UINT8 addr;
  UINT8 val;
} FPD_Para;

#define HRESULT  int

void Elim_Trape (unsigned char *m_FrameBuffer,unsigned char *destImage);
void I2C_SetData (UINT8 index, UINT8 data);
void I2C_TurnOnOppLed (BOOL bTurnOn);
HRESULT OPP_Initialize (UINT16 brightness, UINT8 gain, UINT8 contrast, UINT8 nTimeOut, UINT8 nQuality, UINT8 nDetectInterval);
HRESULT OSP04_Initialize (UINT8 brightness);
HRESULT OPP_FPD_Initialize (FPD_Para Reg);
HRESULT OPP_GetImage (UINT8 * pBuff);
HRESULT OPP_GetLiveImage (UINT8 * pBuff, UINT32 Len);
HRESULT OPP_SetBrightness (UINT16 nBrightness);
HRESULT OPP_GetBrightness (UINT16 * pBrightness);
HRESULT OPP_SetGain (UINT8 nGain);
HRESULT OPP_GetGain (UINT8 * pGain);
HRESULT OPP_SetContrast (UINT8 nContrast);
HRESULT OPP_GetContrast (UINT8 * pContrast);
HRESULT OPP_CheckImage (void);

void FPD_SetData (UINT8 index, UINT8 data);
UINT8 FPD_GetData (UINT8 index);
HRESULT OPP_Start(int On);
void closedev(void);

