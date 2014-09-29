/* 3DES加密/解密算法 */
#ifndef __DES_H__
#define __DES_H__

enum{ENCRYPT, DECRYPT};

/* ENCRYPT:加密, DECRYPT:解密
 输出缓冲区(Out)的长度 >= ((datalen+7)/8)*8, 即比datalen大的且是8的倍数的最小正整数
 In可以=Out,此时加/解密后将覆盖输入缓冲区(In)的内容
 当keylen>8时系统自动使用3次DES加/解密,否则使用标准DES加/解密.超过16字节后只取前16字节*/
int Encrypt_Des(char *Out, char *In, long datalen, const char *Key, int keylen, int Type);

#endif

