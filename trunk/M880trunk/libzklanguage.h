/****************************************************
	author	: jazzy
	date	: 2009.05.22

 Copyright (C) 2009-2009, ZKSoftware Inc.

*************************************************/
#ifndef __LIBZKLANGUAGE_H__
#define __LIBZKLANGUAGE_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#ifndef TRUE
#define TRUE 1
#define FALSE 0
#endif


#if defined _WIN32 || defined __CYGWIN__
  #ifdef BUILDING_DLL
    #ifdef __GNUC__
      #define DLL_PUBLIC __attribute__((dllexport))
    #else
      #define DLL_PUBLIC __declspec(dllexport) // Note: actually gcc seems to also supports this syntax.
    #endif
  #else
    #ifdef __GNUC__
      #define DLL_PUBLIC __attribute__((dllimport))
    #else
      #define DLL_PUBLIC __declspec(dllimport) // Note: actually gcc seems to also supports this syntax.
    #endif
  #endif
  #define DLL_LOCAL
#else
  #if __GNUC__ >= 4
    #define DLL_PUBLIC __attribute__ ((visibility("default")))
    #define DLL_LOCAL  __attribute__ ((visibility("hidden")))
  #else
    #define DLL_PUBLIC
    #define DLL_LOCAL
  #endif
#endif



#define ZKLANGUAGE_DLOPEN


typedef int (*T_initChineseLanguage_35)(char pLngCachedStr[][128], unsigned short *pLngCachedStrIndex);
typedef int (*T_initChineseLanguage_8)(char pLngCachedStr[][128], unsigned short *pLngCachedStrIndex);
typedef int (*T_initChineseLanguage_iface)(char pLngCachedStr[][128], unsigned short *pLngCachedStrIndex);
typedef void (*T_initARM_RCS)();
typedef void (*T_initMipsel_RCS)();


#ifdef ZKLANGUAGE_DLOPEN
T_initChineseLanguage_35 initChineseLanguage_35;
T_initChineseLanguage_8 initChineseLanguage_8;
T_initChineseLanguage_iface initChineseLanguage_iface;
T_initARM_RCS initARM_RCS;
T_initMipsel_RCS initMipsel_RCS;
#else

DLL_PUBLIC void initChineseLanguage_35(char pLngCachedStr[][128], unsigned short *pLngCachedStrIndex);
DLL_PUBLIC void initChineseLanguage_8(char pLngCachedStr[][128], unsigned short *pLngCachedStrIndex);
DLL_PUBLIC void initChineseLanguage_iface(char pLngCachedStr[][128], unsigned short *pLngCachedStrIndex);
DLL_PUBLIC void initARM_RCS();
DLL_PUBLIC void initMipsel_RCS();

#endif//ZKLANGUAGE_DLOPEN

#endif//__LIBZKLANGUAGE_H__
