/* -*-linux-c-*-
 * $Id: usbdpfp.h,v 5.2 2005/12/22 08:54:23 david Exp $
 * Copyright (c) 2002-2005 DigitalPersona, Inc.
 * CONFIDENTIAL
 */

enum ft_image_type {
	FT_BLACKNWHITE,
	FT_GRAYSCALE,
	FT_COLOR,
	FT_UNKNOWN_IMG_TYPE
};

struct ft_image_resolution {
	int xdpi;
	int ydpi;
};

struct ft_image_size {
	int width;		/* in pixels */
	int height;		/* in pixels */
};

enum ft_orientation {
	FT_PORTRAIT,
	FT_LANDSCAPE
};

struct ft_device_info {		/* device information */
	unsigned long long devType;
	unsigned long long devID;
	char devName[64];
	enum ft_image_type imgType;
	struct ft_image_resolution imgResolution;
	unsigned short numIntensityLevels;
	struct ft_image_size imgSize;
	enum ft_orientation imgOrientation;
	unsigned long maxImgSize;
};

/* The USBDPFP_IOCTL_GET_INFO ioctl is used to retrieve the device
   information structure struct ft_device_info. To use it you may
   call:
   
   struct ft_device_info ft;
   ioctl(fd, USBDPFP_IOCTL_GET_INFO, &ft);

   If the ioctl returns success, ft will contain the data for the device 
*/
#define   USBDPFP_IOCTL_GET_INFO         	_IOR('U', 0x20, struct ft_device_info)

/* The USBDPFP_IOCTL_SET_RESOLUTION ioctl is used to adjust the output
   image resolution.  To use it, perform the following:
   
   struct ft_device_info ft;
   ft.imgResolution.xdpi = ft.imgResolution.ydpi = desired_resolution;
   ioctl(fd, USBDPFP_IOCTL_SET_RESOLUTION, &ft);

   If the ioctl call returns success, ft will contain the adjusted
   parameters (image size and maximum image size) for the device with
   the selected resolution. Make sure that buffers are allocated based
   on this returned data, and not on data retrieved by calling
   USBDPFP_IOCTL_GET_INFO. The buffer that is allocated for the
   read(3) and libusbdpfp_process_image() calls must be allocated
   based on the ft_device_info.maxImgSize field, and NOT by (width *
   height * sizeof(unsigned char)).

   Note that in the current implementation xdpi must equal ydpi, and
   if not set explicitly as shown the ioctl will fail.

   The minimum resolution supported is 100 dpi and the maximum is 1000
   dpi. Values outside that range will cause the ioctl call to fail.
*/
#define   USBDPFP_IOCTL_SET_RESOLUTION          _IOWR('U', 0x23, struct ft_device_info)
