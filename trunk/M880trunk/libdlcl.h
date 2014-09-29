#ifndef __LIC_API__
#define __LIC_API__
#ifndef TRUE
#define TRUE	1
#define FALSE	0
#endif
#ifndef BYTE
#define	BYTE unsigned char
#endif
int zkfp_SaveLicense(char *license, int licenseNumber);
int zkfp_ExtractPackage(char *Package,char *Data,int *DataLen);
#endif
