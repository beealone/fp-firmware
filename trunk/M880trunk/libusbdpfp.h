/* -*-linux-c-*-
 * $Id: libusbdpfp.h,v 5.2 2005/12/22 08:54:23 david Exp $
 * Copyright (c) 2002-2005 DigitalPersona, Inc.
 * CONFIDENTIAL
 *
 * Header file for libusbdpfp user mode fingerprint reader helper library
 *
 */

struct point {
	unsigned int x, y;
};

struct extents {
	struct point top_left;
	struct point top_right;
	struct point bottom_left;
	struct point bottom_right;
};

struct ft_image_info {
	struct extents image_shape;
};

/*
 * libusbdpfp_process_image:
 *
 * Image processing helper function. This function is to be called
 * after acquiring an image using the read(2) system call and before
 * processing the image.
 * 
 * Arguments:
 *    fd - File descriptor to /dev/usbdpfpX returned by open(2)
 *    image - Data received from read(2) upon fingerprint image scan
 *    width, height - Dimensions of output image from ioctl(GET_INFO) 
 *    reserved - MUST BE 0 (zero)
 *
 * Return value: Positive (> 0) on success, -1 on failure
 *
 */
int libusbdpfp_process_image(int fd, unsigned char *image, unsigned int width, unsigned int height, int reserved);

/*
 * libusbdpfp_get_image_information
 *
 * This function may be called to retrieve certain image characteristics.
 * In the current implementation, only the dimensions of the trapezoidal
 * shape of the the output image are provided to the caller.
 *
 * NOTE: If ioctl(SET_RESOLUTION) is called on the device, this function
 * should be called subsequently, since changing the image resolution 
 * affects the dimensions of the output image and consequently the 
 * dimensions of the trapezoid.
 *
 * Arguments:
 *    fd - File descriptor to /dev/usbdpfpX returned by open(2)
 *    iinfo - ft_image_info structure returned by this function
 *
 * Return value: 0 on success, -1 on failure.
 *    
 *
 */
int libusbdpfp_get_image_information(int fd, struct ft_image_info *iinfo);
