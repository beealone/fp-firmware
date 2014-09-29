//编译器相关符号定义
#ifndef __CCC_H__
#define __CCC_H__

//定义紧凑型结构及其字节对齐指针
#define GCC_PACKED __attribute__((packed))
#define GCC_ALIGN0 __attribute__((aligned(1)))


#ifndef NULL
#ifdef __cplusplus
#define NULL 0
#else
#define NULL ((void *)0)
#endif
#endif
/*
typedef void(*FnPVOID)(void);

typedef unsigned int        UINT,     *PUINT;    // The size is not important
*/
typedef unsigned long long  UINT64,   *PUINT64;
typedef unsigned int        UINT32,   *PUINT32;
typedef unsigned short      UINT16,   *PUINT16;
typedef unsigned char       UINT8,    *PUINT8;
typedef unsigned char       UCHAR,	  *PUCHAR;
//typedef unsigned long DWORD;

typedef int                 INT,      *PINT;    // The size is not important
typedef long long           INT64,    *PINT64;
typedef int                 INT32,    *PINT32;
typedef short               INT16,    *PINT16;
typedef char                INT8,     *PINT8;
typedef char                CHAR,     *PCHAR;
#define _HAVE_TYPE_LONG
#define _HAVE_TYPE_DWORD
#define _HAVE_TYPE_BYTE
#define _HAVE_TYPE_WORD

#ifndef VOID
typedef void                VOID,     *PVOID;
#endif

//typedef volatile  UINT      VUINT,    *PVUINT;    // The size is not importan
typedef volatile  UINT64    VUINT64,  *PVUINT64;
typedef volatile  UINT32    VUINT32,  *PVUINT32;
typedef volatile  UINT16    VUINT16,  *PVUINT16;
typedef volatile  UINT8     VUINT8,   *PVUINT8;

typedef volatile  UCHAR     VUCHAR,   *PVUCHAR;

typedef volatile  INT       VINT,     *PVINT;    // The size is not important
typedef volatile  INT64     VINT64,   *PVINT64;
typedef volatile  INT32     VINT32,   *PVINT32;
typedef volatile  INT16     VINT16,   *PVINT16;
typedef volatile  INT8      VINT8,    *PVINT8;
typedef volatile  CHAR      VCHAR,    *PVCHAR;

#undef offsetof		//add by jazzy 2009.06.06
#define offsetof(s,m) (size_t)&(((s *)0)->m)

#ifndef GET_BYTE
#define GET_BYTE(_data_, _var_, _off_) \
	(_var_ = *( ((UCHAR *)_data_) + (_off_) ) )
#endif

#ifndef GET_WORD
#define GET_WORD(_data_, _var_, _off_)                      \
	(_var_ = *( ((UCHAR *)_data_) + (_off_) ) |         \
	*( ((UCHAR *)_data_) + (_off_) + 1 ) << 8)
#endif

#ifndef GET_DWORD
#define GET_DWORD(_data_, _var_, _off_)                         \
	(_var_ = *( ((UCHAR *)_data_) + (_off_))             |  \
	*( ((UCHAR *)_data_) + (_off_) + 1 ) << 8   |  \
	*( ((UCHAR *)_data_) + (_off_) + 2 ) << 16  |  \
	*( ((UCHAR *)_data_) + (_off_) + 3 ) << 24)
#endif

#define GETWORD(_data) \
  (((((U8*)(_data))[0])+(((U8*)(_data))[1])*256))

#define GETDWORD(_data) \
	(((U8*)(_data))[0]+(((U8*)(_data))[1])*256+(((U8*)(_data))[2])*256*256+(((U8*)(_data))[3])*256*256*256)

#ifndef GET_BYTES
#define GET_BYTES(_data_, _var_, _size_, _off_) \
	memcpy((void *)(_var_), (void*)(((UCHAR *)_data_)+(_off_)),_size_)
#endif

#ifndef SET_BYTE
#define SET_BYTE(_data_, _val_, _off_) \
	(*( ((UCHAR *)_data_) + (_off_) ) = _val_)
#endif

#ifndef SET_WORD
#define SET_WORD(_data_, _val_, _off_)                                   \
	do {                                                                 \
	*( ((UCHAR *)_data_) + (_off_) )     = (_val_)         & 0xFF; \
	*( ((UCHAR *)_data_) + (_off_) + 1 ) = ((_val_) >> 8)  & 0xFF; \
	} while (0)
#endif

#ifndef SET_DWORD
#define SET_DWORD(_data_, _val_, _off_)                                  \
	do {                                                                 \
	*( ((UCHAR *)_data_) + (_off_) )     = (_val_)         & 0xFF; \
	*( ((UCHAR *)_data_) + (_off_) + 1 ) = ((_val_) >> 8)  & 0xFF; \
	*( ((UCHAR *)_data_) + (_off_) + 2 ) = ((_val_) >> 16) & 0xFF; \
	*( ((UCHAR *)_data_) + (_off_) + 3 ) = ((_val_) >> 24) & 0xFF; \
	} while (0)
#endif

#ifndef SET_BYTES
#define SET_BYTES(_data_, _var_, _size_, _off_) \
	memcpy((void *)(((UCHAR *)_data_)+(_off_)), (void *)(_var_), _size_)
#endif

#define LNG_CH
#define LNG_ENG
                                                                                                               
#endif
