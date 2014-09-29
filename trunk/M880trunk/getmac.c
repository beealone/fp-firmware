#include <sys/socket.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <netinet/if_ether.h>
#include <net/if.h>
#include <linux/sockios.h>
int getmac(char *macaddr1)
{
    char* device="eth0";
    unsigned char macaddr[ETH_ALEN];
   
    int s=socket(AF_INET,SOCK_DGRAM,0);
    struct ifreq req;
    int err;
   
    strcpy(req.ifr_name,device);
    err=ioctl(s,SIOCGIFHWADDR,&req);
    close(s);
    //int i=0;
    if(err!=-1)
    {

        memcpy(macaddr,req.ifr_hwaddr.sa_data,ETH_ALEN);
	sprintf((char *)macaddr1,"%.2x:%.2x:%.2x:%.2x:%.2x:%.2x",macaddr[0],macaddr[1],macaddr[2],macaddr[3],macaddr[4],macaddr[5]);
	return 1;
    }
    else
    {
	return 0;
    }
}
