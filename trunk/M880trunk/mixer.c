/*************************************************
                                           
 ZEM 200                                          
                                                    
 mixer.c function of setting mixer                              
                                                      
 Copyright (C) 2003-2006, ZKSoftware Inc.      		
                                                      
**************************************************/
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <linux/soundcard.h>
#include <string.h>
#include "arca.h"

int SetMixer(char *dev, int vol)
{
	/* names of available mixer devices */
	const char *sound_device_names[] = SOUND_DEVICE_NAMES;

	int fd;                  /* file descriptor for mixer device */
	int devmask, stereodevs; /* bit masks of mixer information */

	int left, right, level;  /* gain settings */
	int status;              /* return value from system calls */
	int device;              /* which mixer device to set */
	int i;                   /* general purpose loop counter */

	/* open mixer, read only */
	fd = open("/dev/mixer", O_RDONLY);
	if (fd==-1) 
	{
		DBPRINTF("unable to open %s\n", "/dev/mixer");
		return 0;
	}
  
	/* get needed information about the mixer */
	status = ioctl(fd, SOUND_MIXER_READ_DEVMASK, &devmask);
	if (status==-1)
	{
		DBPRINTF("SOUND_MIXER_READ_DEVMASK ioctl failed\n");
		close(fd);
		return 0;
	}
	status = ioctl(fd, SOUND_MIXER_READ_STEREODEVS, &stereodevs);
	if (status==-1)
	{
		DBPRINTF("SOUND_MIXER_READ_STEREODEVS ioctl failed\n");
		close(fd);
		return 0;
	}
	/* figure out which device to use */
	for (i = 0 ; i < SOUND_MIXER_NRDEVICES ; i++)
		if (((1 << i) & devmask) && !strcmp(dev, sound_device_names[i]))
			break;
	if (i == SOUND_MIXER_NRDEVICES) /* didn't find a match */
	{
		DBPRINTF("%s is not a valid mixer device\n", dev);
		close(fd);
		return 0;
	}

	/* we have a valid mixer device */
	device = i;

	/* left and right are the same */
	left  = vol;
	right = vol;
  
	/* display warning if left and right gains given for non-stereo device */
	if ((left != right) && !((1 << i) & stereodevs)) 
	{
		DBPRINTF("warning: %s is not a stereo device\n", dev);
		close(fd);
		return 0;
	}
  
	/* encode both channels into one value */
	level = (right << 8) + left;
  
	/* set gain */
	status = ioctl(fd, MIXER_WRITE(device), &level);
	if (status == -1) {
		DBPRINTF("MIXER_WRITE ioctl failed\n");
		close(fd);
		return 0;
	}

	/* unpack left and right levels returned by sound driver */
	left  = level & 0xff;
	right = (level & 0xff00) >> 8;

	/* display actual gain setting */
	DBPRINTF("%s gain set to %d%% / %d%% (1-100)\n", dev, left, right);

	/* close mixer device and exit */
	close(fd);
	return 1;
}

