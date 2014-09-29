/* 3DESº”√‹/Ω‚√‹À„∑® */
#ifndef __3DES_H__
#define __3DES_H__

enum{DECRYPT, ENCRYPT};

void MACCAL_KEY16(unsigned char* kBuf,unsigned char *pRadom,unsigned char pRanAdd,unsigned char *data_in,short dlen,unsigned char *pMac);

void ThreeDES_DAtA16(unsigned char *DoubleKeyStr,unsigned char *Data,unsigned char *Out,int flag);

#endif

