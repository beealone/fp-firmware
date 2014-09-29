/*************************************************
                                           
 ZEM 200                                          
                                                    
 NetSpeed.c Setup network speed AUTO/10M/100M                               
                                                      
 Copyright (C) 2003-2004, ZKSoftware Inc.      		
                                                      
*************************************************/

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/sockios.h>
#include <net/if.h>
#include <errno.h>
#include <string.h>
#include "net.h"

#define BMCR_ANENABLE     0x1000  /* Enable auto negotiation */
#define BMCR_SPEED100     0x2000  /* Select 100Mbps          */

int read_phy_reg(unsigned char *dname, unsigned int reg_num, unsigned int *reg_val)
{
  int handle;
  struct ifreq ifr;
  unsigned short *data;
  void *tmpdata;

  if ((handle=socket(AF_INET, SOCK_DGRAM, 0)) == -1)
  {
    DBPRINTF("init socket error!\n");
    return -1;
  }

  memset(&ifr, 0, sizeof(ifr));
  strcpy(ifr.ifr_name, (char *)dname);

  if (ioctl(handle, SIOCGMIIREG, &ifr)==-1)
  {
    DBPRINTF("ioctl[SIOCGMIIREG] error!\n");
    close(handle);
    return -1;
  }
  tmpdata= &ifr.ifr_data;
  //data = (unsigned short *)(&ifr.ifr_data); 
  data = (unsigned short *)tmpdata;
  *(data+1) = (unsigned short)(reg_num&0x0000ffff);

  if (ioctl(handle, SIOCGMIIREG, &ifr)==-1)
  {
    DBPRINTF("ioctl[SIOCGMIIREG] error!\n");
    close(handle);
    return -1;
  }

  *reg_val = *(data+3);
  close(handle);
  return 0;
}

int write_phy_reg(unsigned char *dname, unsigned int reg_num, unsigned int reg_val)
{
  int handle;
  struct ifreq ifr;
  unsigned short *data;
  void *tmpdata;

  if ((handle=socket(AF_INET, SOCK_DGRAM, 0)) == -1)
  {
    DBPRINTF("init socket error!\n");
    return -1;
  }

  memset(&ifr, 0, sizeof(ifr));
  strcpy(ifr.ifr_name,(char *) dname);

  if (ioctl(handle, SIOCGMIIREG, &ifr)==-1)
  {
    DBPRINTF("ioctl[SIOCGMIIREG] error!\n");
    close(handle);
    return -1;
  }
  tmpdata= &ifr.ifr_data;
  //data = (unsigned short *)(&ifr.ifr_data); 
  data = (unsigned short *)tmpdata;
  *(data+1) = (unsigned short)(reg_num&0x0000ffff);
  *(data+2) = (unsigned short)(reg_val&0x0000ffff);

  if (ioctl(handle, SIOCSMIIREG, &ifr)==-1)
  {
    DBPRINTF("ioctl[SIOCGMIIREG] error!\n");
    close(handle);
    return -1;
  }

  close(handle);
  return 0;
}

// Speed = 0: AUTO 1: 100M 2: 10M
int set_network_speed(unsigned char *net_device, int net_speed)
{
  unsigned int reg_num, reg_val;
  reg_num = 0;
  int speed;
  
  if ((net_speed==ETH_10MHD)||(net_speed==ETH_10MFD))
      speed=2;
  else if ((net_speed==ETH_100MHD)||(net_speed==ETH_100MFD))
      speed=1;
  else
      speed=0;
  
  if (read_phy_reg(net_device, reg_num, &reg_val) == -1)
    return -1;

  if (speed)
    reg_val &= ~BMCR_ANENABLE;
  else
    reg_val |= BMCR_ANENABLE;

  if (write_phy_reg(net_device, reg_num, reg_val) == -1)
    return -1;

  switch (speed)
  {
    case 0:
      return 0;
    case 1:
      reg_val &= ~BMCR_ANENABLE;
      reg_val |= BMCR_SPEED100;
      break;
    case 2:
      reg_val &= ~(BMCR_ANENABLE|BMCR_SPEED100);
      break;
    default:
      return -1;
  }

  if (write_phy_reg(net_device, reg_num, reg_val) == -1)
    return -1;

  return 0;
}

/*
int main(int argc, char* argv[])
{
  unsigned int reg_val, reg_num;

  DBPRINTF("set network speed to 10M ");
  if (set_network_speed(NET_DEVICE_NAME, 2) == -1)
    DBPRINTF("failed.\n\n");
  else
    DBPRINTF("ok.\n\n");

  getchar();

  DBPRINTF("set network speed to auto ");
  if (set_network_speed(NET_DEVICE_NAME, 0) == -1)
    DBPRINTF("failed.\n\n");
  else
    DBPRINTF("ok.\n\n");

  getchar();

  DBPRINTF("set network speed to 100M ");
  if (set_network_speed(NET_DEVICE_NAME, 1) == -1)
    DBPRINTF("failed.\n\n");
  else
    DBPRINTF("ok.\n\n");

  getchar();

  return 0;
}
*/
