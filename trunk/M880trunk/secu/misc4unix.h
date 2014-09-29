/*************************************************************
 *
 * Author :      SecuGen Corporation
 * Description : misc4unix.h Source Code Module
 * Copyright(c): 2004 SecuGen Corporation, All rights reserved
 * History : 
 * date        person   comments
 * ======================================================
 *
 *
 *************************************************************/
#ifndef _MISC4UNIX_H_
#define _MISC4UNIX_H_

//RILEY 2005-08-12 #ifndef __FDU4LINUX__
//RILEY 2005-08-12 #define __FDU4LINUX__
#ifndef __FDU4_UNIX_LINUX__ //RILEY 2005-08-12
#define __FDU4_UNIX_LINUX__ //RILEY 2005-08-12
#endif 

#include <stdio.h>
//RILEY 2005-08-12 #ifndef __SOLARIS
#ifdef __LINUX
//#include <sys/io.h>
//#include <sys/perm.h>
#include <io.h> //MIPS
#include <perm.h> //MIPS
#endif
#include <sys/types.h>
#include <sys/time.h>
#include <unistd.h>
#include <string.h>

#include "utype.h"
#include "wtype.h"

//#define __LOW_IO__
#ifdef __LOW_IO__

#define outb outb_p
#define outw outw_p
#define outl outl_p

#define inb inb_p
#define inw inw_p
#define inl inl_p

#endif // __LOW_IO__

#define WINAPI // ignore WINAPI directive
#define DLLEXPORT // ignore DLLEXPORT directive

#define FAR    // ignore FAR directive

#define __declspec(x...)  // ignore DLL export declation

typedef int* HWND;  // igrnore HWND
typedef int* HDC;  // igrnore HDC

typedef struct{   // rect structure
  int left, top, right, bottom;
} RECT;

typedef RECT* LPRECT;

typedef void * HANDLE;

#define INVALID_HANDLE_VALUE 0

/*
inline DWORD GetTickCount()
{
  struct timeval tv;
  struct timezone tz;
  gettimeofday(&tv, &tz);
  return (((tv.tv_sec%100)*1000000) + tv.tv_usec);
}
*/
typedef void*   LPVOID;
typedef void*   HINSTANCE;

#endif // _MISC4UNIX_H_
