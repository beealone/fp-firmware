#include <stdio.h>
#include "opp03.h"
#include "oppfpd.h"

#include <stdlib.h>
#include <string.h>

int FPDInitialize(int DetectRange)
{
	FPD_Para Reg;
/*You should put your finger on camera when FPD is being initialized */
	printf("Please place your finger on sensor surface!\n");
//	usleep(1000*1000);

	Reg.addr = FPDCTR; Reg.val = 0x01;	//reset
	OPP_FPD_Initialize(Reg);

/* AFDR saved Auto Finger Detection range. The biger the value is , the easier finger is detected on sensor surface.*/
	Reg.addr = AFDR; Reg.val = DetectRange;  //Default value is 0x1a;	
	OPP_FPD_Initialize(Reg);

/* FFDR saved Auto Finger Detection range. The biger the value is , the easier finger is detected on sensor surface.*/
	Reg.addr = FFDR; Reg.val = DetectRange;	//Default value is 0x15;	
	OPP_FPD_Initialize(Reg);

/* While writing the val, FPD starts to find Optimum Response value from the finger on the surface. From this median value, FPD unit calculates two values with AFDR or LFDR.Therefor,you must place your finger on sensor surface when writing this value . */
	Reg.addr = FPDCTR; Reg.val = 0x42;	
//	Reg.addr = FPDCTR; Reg.val = 0x44;
	OPP_FPD_Initialize(Reg);

/* Turn Finger Detecting On  */
	Reg.addr = FPDCTR; Reg.val = 0x48;	
//	Reg.addr = FPDCTR; Reg.val = 0x50;
	OPP_FPD_Initialize(Reg);
/* Please use the Default value of FPDCHK. */
//	Reg.addr = FPDCHK; Reg.val = 0x51;
//	OPP_FPD_Initialize(Reg);

	printf("Initializing have been finished. thank you!\n");
 	return 0;
}
