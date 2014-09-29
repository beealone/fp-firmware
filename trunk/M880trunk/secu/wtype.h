/*************************************************************
 *
 * Author :      SecuGen Corporation
 * Description : wtype.h Source Code Module
 * Copyright(c): 2004 SecuGen Corporation, All rights reserved
 * History : 
 * date        person   comments
 * ======================================================
 *
 *
 *************************************************************/
#ifndef WTYPE_H
#define WTYPE_H


#ifndef _WINDEF_
typedef unsigned long       DWORD;

#ifndef BOOL
typedef int                 BOOL;
#endif

typedef unsigned char       BYTE;
typedef unsigned short      WORD;
#endif

#ifdef _PLUTO_FOR_DOS
#ifndef _DEBUG
   typedef int bool;
   enum { false, true };
#endif
   enum { FALSE, TRUE };

   #ifndef UINT
   	#define UINT unsigned int
   #endif

   #include <string.h>
#endif//_PLUTO_FOR_DOS

#endif
