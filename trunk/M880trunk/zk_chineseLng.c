/*************************************************
 sensor.c
                                                      
 Copyright (C) 2003-2009, ZKSoftware Inc.      		

 Common interface code for fingerprint reader library libfpsensor.so

 $Log $
 Revision 1.01  2009/08/05 13:49:09  Zhang Honggen
 Spupport liblanguage.so,remove fingerprint algorithm function

*************************************************/

#include <stdio.h>
#include <stdlib.h>
#include "dlfcn.h"


#include "libzklanguage.h"
#include "zk_chineseLng.h"

void* m_FPHandle;
char zk_initLanguageFlag=0;

int zk_loadSymbol(void *handle)
{
	initChineseLanguage_35 = dlsym(handle,"initChineseLanguage_35");
	initChineseLanguage_8 = dlsym(handle,"initChineseLanguage_8");
	initChineseLanguage_iface = dlsym(handle,"initChineseLanguage_iface");
	initARM_RCS = dlsym(handle,"initARM_RCS");
	initMipsel_RCS = dlsym(handle,"initMipsel_RCS");

	return 0;
}

int zk_initLanguage()
{
	m_FPHandle = dlopen("liblanguage.so", RTLD_LAZY);
	if(m_FPHandle == NULL)
	{
		printf("Cannot open library: %s\n",dlerror());
		zk_initLanguageFlag=0;
		return FALSE;
	}	
	zk_loadSymbol(m_FPHandle);
	zk_initLanguageFlag=1;

	return TRUE;
}

void zk_initChineseLanguage_35(char pLngCachedStr[][128], unsigned short *pLngCachedStrIndex)
{
	if(zk_initLanguageFlag==0)
		zk_initLanguage();

	if(zk_initLanguageFlag)
		initChineseLanguage_35(pLngCachedStr, pLngCachedStrIndex);
	else
		printf("%s() Error: doesn't open library:liblanguage.so\n", __FUNCTION__);
}

void zk_initChineseLanguage_8(char pLngCachedStr[][128], unsigned short *pLngCachedStrIndex)
{
	if(zk_initLanguageFlag==0)
		zk_initLanguage();

	if(zk_initLanguageFlag)
		initChineseLanguage_8(pLngCachedStr, pLngCachedStrIndex);
	else
		printf("%s() Error: doesn't open library:liblanguage.so\n", __FUNCTION__);
}
void zk_initChineseLanguage_iface(char pLngCachedStr[][128], unsigned short *pLngCachedStrIndex)
{
	if(zk_initLanguageFlag==0)
		zk_initLanguage();

	if(zk_initLanguageFlag)
		initChineseLanguage_iface(pLngCachedStr, pLngCachedStrIndex);
	else
		printf("%s() Error: doesn't open library:liblanguage.so\n", __FUNCTION__);
}

void zk_initMipsel_RCS()
{

	if(zk_initLanguageFlag==0)
		zk_initLanguage();

	if(zk_initLanguageFlag)
		initMipsel_RCS();
	else
		printf("%s() Error: doesn't open library:liblanguage.so\n", __FUNCTION__);
}

void zk_initARM_RCS()
{
	if(zk_initLanguageFlag==0)
		zk_initLanguage();

	if(zk_initLanguageFlag)
		initARM_RCS();
	else
		printf("%s() Error: doesn't open library:liblanguage.so\n", __FUNCTION__);
}

int getZKLibLanguageState()
{
	return zk_initLanguageFlag;
}





