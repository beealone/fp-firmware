#ifndef __SRBFUN_H__
#define __SRBFUN_H__

#define SRB_BINARY_LEN		56

#define OLD_SRB_MODE		0
#define NEW_SRB_MODE		1

BOOL WiegandSRBOutput(int srbtype, unsigned char lockdelay);

#endif
