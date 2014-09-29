#ifndef __CIM_H__
#define __CIM_H__

int cim_open(int width, int height, int bpp);
void cim_close(void);
int cim_read(unsigned char *buf, int size);

#endif /* __CIM_H__ */

