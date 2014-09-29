/*************************************************
                                           
 ZEM 200                                          
                                                    
 usb_helper.h 
                                                      
 Copyright (C) 2003-2005, ZKSoftware Inc.      		
                                                      
*************************************************/

#ifndef	_USB_HELPER_H_
#define	_USB_HELPER_H_

#include "options.h"

#define  USB_MOUNTPOINT (LoadStrOld("USBMountDir")?LoadStrOld("USBMountDir"):"/mnt/removable")

int DoMountUdisk(void);
void DoUmountUdisk(void);
int CheckUsbState(void);
	
#endif
