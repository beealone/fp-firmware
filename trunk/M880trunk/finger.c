/*************************************************
                                           
 finger.c                              
                                                      
 Copyright (C) 2003-2009, ZKSoftware Inc.      		
 
 add ZKFP V10/V9 algrithm support. Zhang Honggen 2009.9.27

 Add ZKFPV10 V0 db upgrade to V1. Zhang Honggen 2009.11.24
*************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "flashdb.h"
#include "finger.h"
#include "options.h"
#include "sensor.h"
#include "libdlcl.h"
#include "thread.h"
#include "utils.h"

#ifdef WIN32
#else
#include <dlfcn.h>
#endif

#define P_O_WIDTH(s) s[0]
#define P_O_HEIGHT(s) s[1]
#define P_CP0_X(s) ((s[2]))
#define P_CP0_Y(s) ((s[3]))
#define P_CP1_X(s) ((s[4]))
#define P_CP1_Y(s) ((s[5]))
#define P_CP2_X(s) ((s[6]))
#define P_CP2_Y(s) ((s[7]))
#define P_CP3_X(s) ((s[8]))
#define P_CP3_Y(s) ((s[9]))
#define P_TP0_X(s) s[10]
#define P_TP0_Y(s) s[11]
#define P_TP1_X(s) s[12]
#define P_TP1_Y(s) s[13]
#define P_TP2_X(s) s[14]
#define P_TP2_Y(s) s[15]
#define P_TP3_X(s) s[16]
#define P_TP3_Y(s) s[17]
#define P_TP4_X(s) s[18]
#define P_TP4_Y(s) s[19]
#define FP_WIDTH(s) s[20]
#define FP_HEIGHT(s) s[21]


HANDLE fhdl=0;

void *loadSym(void *handle, const char *symbol)
{
	void *api;
	char *errMsg;
#ifdef WIN32
	api = GetProcAddress(handle,symbol);
#else
	api=dlsym(handle, symbol);
	errMsg=dlerror();
	if(errMsg)
	{
		printf("Load function error: %s\n", errMsg);
		return 0;
	}
#endif
	return api;
}

int LoadSymbol()
{
#ifdef WIN32
	HINSTANCE handle;
#else
	void *handle;
#endif

	printf("start dlopen libzkfp.so\n");
#ifdef WIN32
	handle = LoadLibrary("zkfp.10.dll");
#else
	if(gOptions.ZKFPVersion == ZKFPV10)
		handle = dlopen("libzkfp.so.10", RTLD_NOW);
	else
		handle = dlopen("libzkfp.so.3", RTLD_NOW);
		
#endif
	if(!handle)
	{
		printf("Load libzkfp failed,error:%s\n",dlerror());
		return FALSE;
	}
	printf("start load symbol\n");
	BIOKEY_INIT =loadSym(handle,"BIOKEY_INIT");
	if(BIOKEY_INIT)
		printf("load BIOKEY_INIT success\n");
	else
	{
		printf("load BIOKEY_INIT failed\n");
		return FALSE;
	}
	//BIOKEY_INIT_SIMPLE =loadSym(handle,"BIOKEY_INIT_SIMPLE");
	BIOKEY_CLOSE =loadSym(handle,"BIOKEY_CLOSE");
	BIOKEY_EXTRACT =loadSym(handle,"BIOKEY_EXTRACT");
	BIOKEY_GENTEMPLATE =loadSym(handle,"BIOKEY_GENTEMPLATE");
	BIOKEY_VERIFY =loadSym(handle,"BIOKEY_VERIFY");
	BIOKEY_MATCHINGPARAM1 =loadSym(handle,"BIOKEY_MATCHINGPARAM");
	BIOKEY_DB_CLEAR =loadSym(handle,"BIOKEY_DB_CLEAR");
	BIOKEY_DB_ADD =loadSym(handle,"BIOKEY_DB_ADD");
	BIOKEY_DB_DEL =loadSym(handle,"BIOKEY_DB_DEL");
	BIOKEY_DB_FILTERID =loadSym(handle,"BIOKEY_DB_FILTERID");
	BIOKEY_DB_FILTERID_ALL =loadSym(handle,"BIOKEY_DB_FILTERID_ALL");
	BIOKEY_DB_FILTERID_NONE =loadSym(handle,"BIOKEY_DB_FILTERID_NONE");
	BIOKEY_IDENTIFYTEMP =loadSym(handle,"BIOKEY_IDENTIFYTEMP");
	BIOKEY_GETPARAM =loadSym(handle,"BIOKEY_GETPARAM");
	BIOKEY_SETNOISETHRESHOLD =loadSym(handle,"BIOKEY_SETNOISETHRESHOLD");
	BIOKEY_TEMPLATELEN =loadSym(handle,"BIOKEY_TEMPLATELEN");
	BIOKEY_SETTEMPLATELEN =loadSym(handle,"BIOKEY_SETTEMPLATELEN");
       
	BIOKEY_GETLASTQUALITYEx =loadSym(handle,"BIOKEY_GETLASTQUALITY");

	if(gOptions.ZKFPVersion == ZKFPV10)
	{
		BIOKEY_GET_CUSTOMDATA =loadSym(handle,"BIOKEY_GET_CUSTOMDATA");
	        BIOKEY_SET_CUSTOMDATA =loadSym(handle,"BIOKEY_SET_CUSTOMDATA");
		BIOKEY_GETVERSION =loadSym(handle,"BIOKEY_GETVERSION");
		BIOKEY_DB_CLEAREX =loadSym(handle,"BIOKEY_DB_CLEAREX");
		BIOKEY_DB_GET_TEMPLATE =loadSym(handle,"BIOKEY_DB_GET_TEMPLATE");
		BIOKEY_SET_PARAMETER =loadSym(handle,"BIOKEY_SET_PARAMETER");
		BIOKEY_GET_PARAMETER =loadSym(handle,"BIOKEY_GET_PARAMETER");
		BIOKEY_DB_ADDEX =loadSym(handle,"BIOKEY_DB_ADDEX");
		BIOKEY_MERGE_TEMPLATE = loadSym(handle,"BIOKEY_MERGE_TEMPLATE");
		BIOKEY_SPLIT_TEMPLATE = loadSym(handle,"BIOKEY_SPLIT_TEMPLATE");
	}
	if (!biokey_get_param_lock_init()) {
		return FALSE;
	}
	return TRUE;
}

static int fd_Sensor=-1;
int FPBaseInit(char *FingerCacheBuf)
{
	unsigned short sizes[22];
	int isUpgradeDBV10 = FALSE;
	int fp10DBVersion;
	BYTE params[8];
	
	P_O_WIDTH(sizes)=gOptions.ZF_WIDTH;
	P_O_HEIGHT(sizes)=gOptions.ZF_HEIGHT;
	FP_WIDTH(sizes)=gOptions.ZF_WIDTH;
	FP_HEIGHT(sizes)=gOptions.ZF_HEIGHT;
	P_CP2_X(sizes)=(short)gOptions.CPX[0];
	P_CP0_Y(sizes)=(short)gOptions.CPY[0];
	P_CP3_X(sizes)=(short)gOptions.CPX[1];
	P_CP1_Y(sizes)=(short)gOptions.CPY[1];
	P_CP0_X(sizes)=(short)gOptions.CPX[2];
	P_CP2_Y(sizes)=(short)gOptions.CPY[2];
	P_CP1_X(sizes)=(short)gOptions.CPX[3];
	P_CP3_Y(sizes)=(short)gOptions.CPY[3];
	params[0]=1;
	params[1]=50;
	params[2]=65;
	params[3]=110;
	params[4]=0;
	params[5]=255;
#ifdef TESTIMAGE
	P_TP0_X(sizes)=8;
	P_TP0_Y(sizes)=8;
#else
	P_TP0_X(sizes)=180;
	P_TP0_Y(sizes)=138;
#endif	
	if((FingerCacheBuf==NULL) && fhdl) 
		FingerCacheBuf=(char*)fhdl; 
	else if(FingerCacheBuf==NULL) 
		FingerCacheBuf=(char*)MALLOC(1024*1024*2);
		
	if(gOptions.ZKFPVersion == ZKFPV10) 
	{
		fp10DBVersion = LoadInteger("FP10DBVersion",0);
		if(fp10DBVersion!= ZKFPV10_DB_V1)
		{
			systemEx("rm /mnt/mtdblock/tempinfo.dat -rf & sync");
			isUpgradeDBV10 = TRUE;
		}

		if(gOptions.MaxFingerCount*100 > 10000)
		{
			BIOKEY_SET_PARAMETER(fhdl, 5009, (gOptions.MaxFingerCount*100)+1);
		}
	}

	if (!fhdl)
	{
		fhdl=BIOKEY_INIT(fd_Sensor, (WORD*)sizes, params, (BYTE*)FingerCacheBuf, 0x80);	
		//printf("fhdl:%d\n",fhdl);
	if (fhdl)
	{
		int regTmpFmt = 1;
		BIOKEY_MATCHINGPARAM1(fhdl, IDENTIFYSPEED, gOptions.MThreshold);
		BIOKEY_SETNOISETHRESHOLD(fhdl, gOptions.MaxNoiseThr, gOptions.MinMinutiae, gOptions.MaxTempLen, 500);
		regTmpFmt = LoadInteger("ZKFP10RegTmpFmt",ZKFP_REG_FP_FORMAT_STD);
		if(regTmpFmt!= ZKFP_REG_FP_FORMAT_STD)
			BIOKEY_SET_PARAMETER(fhdl,ZKFP_PARAM_CODE_REG_FP_FORMAT,regTmpFmt);	
		if(isUpgradeDBV10)
		{
			FDB_UpgradeTmpV10DB();
			fp10DBVersion = ZKFPV10_DB_V1;
			SaveInteger("FP10DBVersion", ZKFPV10_DB_V1 );
		}
		if(gOptions.ZKFPVersion == ZKFPV10)
		{
			BIOKEY_SET_PARAMETER(fhdl, CFG_MAX_ROTATION, gOptions.FingerMaxRotation);
		}
	}
	else
	{
		printf("Biokey init failed\n");
	}
	}

	return (int)fhdl;
}

int FPDBInit(void)
{
	if(fhdl)
	{
		BIOKEY_DB_CLEAR(fhdl);
		return (FDB_LoadTmp(fhdl));
	}
	return 0;
}

void FPFree(void)
{
	if (fhdl)
	{
		BIOKEY_DB_CLEAR(fhdl);
		FREE(fhdl);
	}
	biokey_get_param_lock_free();
}

int FPInit(char *FingerCacheBuf)
{
	if (gOptions.IsOnlyRFMachine){
		return 0;
	}

	static int isLoadedLib=FALSE;
	if(!isLoadedLib) {
		printf("Load symbol\n");
		int majorv=0,minorv=0;
		int ret=0;
		char sbuf[128]={0};

		if(LoadStr("ZKFP09PARAM",sbuf)) {
			zkfp_SaveLicense(sbuf, 9);
		}

		memset(sbuf,0,sizeof(sbuf));
		if(LoadStr("ZKFP10PARAM",sbuf)) {
			zkfp_SaveLicense(sbuf, 10);
		}

		memset(sbuf,0,sizeof(sbuf));
		if(LoadStr("ZKFP50PARAM",sbuf)) {
			zkfp_SaveLicense(sbuf, 50);
		}

		ret =LoadSymbol();
		if(!ret) {
			return FALSE;
		}
		isLoadedLib = TRUE;
		printf("gOptions.ZKFPVersion = %d\n", gOptions.ZKFPVersion);
		if(gOptions.ZKFPVersion== ZKFPV10 && BIOKEY_GETVERSION)	{
			BIOKEY_GETVERSION(&majorv,&minorv);
			printf("Biokey version: %d.%d\n", majorv, minorv);
			if(majorv == ZKFPV10) {
				gOptions.ZKFPVersion = ZKFPV10;
				if(FingerCacheBuf){
					printf("free FingerCacheBuf in %s\n", __FUNCTION__);
					FREE(FingerCacheBuf);
					FingerCacheBuf=NULL;
				}
			}
		}
	}
	printf("FPBaseInit\n");
	FPBaseInit(FingerCacheBuf);
	return FPDBInit();
}

int CalcThreshold(int NewT)
{
	if(NewT<10) return NewT<0?0:NewT*3;
	else return NewT>50? 70: NewT+20;
}

int CalcNewThreshold(int Thr)
{
	if(Thr<30) return Thr<0?0:Thr/3;
	else return Thr>70? 50: Thr-20;
}

static pthread_mutex_t *biokey_get_param_lock = NULL;
int biokey_get_param_lock_init(void)
{
	if (biokey_get_param_lock == NULL) {
		biokey_get_param_lock = mutex_init();
	}
	if (biokey_get_param_lock == NULL) {
		return FALSE;
	}
	return TRUE;
}

int  BIOKEY_GET_PARAM(HANDLE Handle, int ParameterCode, int *ParameterValue)
{
	int ret ;
	mutexP(biokey_get_param_lock); 
	ret = BIOKEY_GET_PARAMETER(Handle, ParameterCode, ParameterValue); 
	mutexV(biokey_get_param_lock); 
	return ret;
}

void biokey_get_param_lock_free(void)
{
	if (biokey_get_param_lock) {
		mutex_destroy(biokey_get_param_lock);
	}
	biokey_get_param_lock = NULL;
}
