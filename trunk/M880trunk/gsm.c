#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <termios.h>
#include <time.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <ifaddrs.h>
#include <crypt.h>
#define _XOPEN_SOURCE
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "utils.h"
#include "arca.h"
#include "flash.h"
#include "options.h"
#include "rtc.h"
#include "net.h"
#include "netspeed.h"
#include <netinet/in.h>
#include <arpa/inet.h>
#include <dirent.h>

// Returns 1 if found, 0 otherwise. needle must be null-terminated.
// strstr might not work because WebBox sends garbage before the first OK
int findInBuf(char* buf, int len, char* needle) {
  int i;
  int needleMatchedPos=0;
  
  if (needle[0] == '\0') {
    return 1;
  }

  for (i=0;i<len;i++) {
    if (needle[needleMatchedPos] == buf[i]) {
      needleMatchedPos++;
      if (needle[needleMatchedPos] == '\0') {
	// Entire needle was found
	return 1; 
      }      
    } else {
      needleMatchedPos=0;
    }
  }
  return 0;
}


/* Sends an AT-command to a given serial port and waits
* for reply.
*
* PARAMS:
* fd  - file descriptor
* cmd - command
* to  - how many microseconds to wait for response (this is done 100 times)
* RETURNS:
* 1 on success (OK-response), 0 otherwise
*/
int at_command(int fd, char *cmd, int to, char *rev_buf)
{
	fd_set rfds;
	struct timeval timeout;
	unsigned char buf[1024];
	int sel, len, i;
	int returnCode = 0;
	int wrote = 0;


	/*
	if(_debug)
		syslog(LOG_DEBUG, "is in %s\n", __FUNCTION__);
	*/
	wrote = write(fd, cmd, strlen(cmd));

	/*
	if(_debug)
		syslog(LOG_DEBUG, " wrote  %d \n", wrote);
	*/
	tcdrain(fd);
	sleep(1);
	//memset(buf, 0, sizeof(buf));
	//len = read(fd, buf, sizeof(buf));

	for (i = 0; i < 100; i++)
	{

		FD_ZERO(&rfds);
		FD_SET(fd, &rfds);

		timeout.tv_sec = 0;
		timeout.tv_usec = to;

		if ((sel = select(fd + 1, &rfds, NULL, NULL, &timeout)) > 0)
		//if ((sel = select(fd + 1, &rfds, NULL, NULL, NULL)) > 0)
		{

			if (FD_ISSET(fd, &rfds))
			{
				memset(buf, 0, sizeof(buf));
				len = read(fd, buf, sizeof(buf));
				/*
				if(_debug)
					syslog(LOG_DEBUG, " read %d bytes == %s\n", len, buf);
					*/

				//if (strstr(buf, "\r\nOK\r\n") != NULL)
				if(len >0)
				{
					if (findInBuf((char *)buf, len, "OK"))
					{
						returnCode = 1;
						strcpy(rev_buf,(char *) buf);
						break;
					}
					if (findInBuf((char *)buf, len, "ERROR"))
						break;
				}
			}
		}

	}

	return returnCode;
}


