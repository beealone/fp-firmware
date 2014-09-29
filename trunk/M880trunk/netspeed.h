/*************************************************
                                           
 ZEM 200                                          
                                                    
 netspeed.h                               
                                                      
 Copyright (C) 2003-2005, ZKSoftware Inc.      		
                                                      
*************************************************/

#ifndef	_NETSPEED_H_
#define	_NETSPEED_H_

#define NET_DEVICE_NAME   "eth0"
// Speed = 0: AUTO 1: 100M 2: 10M
int set_network_speed(unsigned char *net_device, int speed);

#endif
