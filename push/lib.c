#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <errno.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <dirent.h>
#include <ctype.h>
#include <sys/wait.h>
#include <fcntl.h>

#include "fw_api.h"

/* 
** if_name like "ath0", "eth0". Notice: call this function
** need root privilege.
** return value:
** -1 -- error , details can check errno
** 1 -- interface link up
** 0 -- interface link down.
*/
int interface_is_up(char *ifname)
{
	int sock = -1;
	struct ifreq ifr;

	if (ifname == NULL) {
		return -1;
	}
	sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (sock < 0) {
		return -1;
	}

	strcpy(ifr.ifr_name, ifname);
	if(ioctl(sock, SIOCGIFFLAGS, (caddr_t) &ifr) < 0) {
		close(sock);
		return -1;
	}
	
	close(sock);
	return ((ifr.ifr_flags & IFF_UP) && (ifr.ifr_flags & IFF_RUNNING));
}

int interface_get_stat(int family)
{
	int		sockfd, len, lastlen;
	char		*buf,*ptr;
	struct ifreq *ifr;
	struct ifconf	ifc;

	if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == 0) {
		return -1;
	}

	lastlen = 0;
	len = 100 * sizeof(struct ifreq);	/* initial buffer size guess */
	for ( ; ; ) {
		buf = malloc(len);
		if (buf == NULL) {
			return -2;
		}
		ifc.ifc_len = len;
		ifc.ifc_buf = buf;
		if (ioctl(sockfd, SIOCGIFCONF, &ifc) < 0) {
			if (errno != EINVAL || lastlen != 0) {
				printf("[PUSH SDK]    (ioctl(sockfd, SIOCGIFCONF, &ifc) < 0)\n\n");

                if( buf != NULL)
                {
                    free(buf);
                    buf = NULL;
                }
                if(sockfd != 0)
                {
                    close(sockfd);
                    sockfd = 0;
                }

				return -3;
			}
		} else {
			if (ifc.ifc_len == lastlen)
				break;		/* success, len has not changed */
			lastlen = ifc.ifc_len;
		}
		len += 10 * sizeof(struct ifreq);	/* increment */
		
    	if( buf != NULL)
        {
            free(buf);
            buf = NULL;
        }
		
	}
	for (ptr = buf; ptr < buf + ifc.ifc_len; ) {
		ifr = (struct ifreq *) ptr;
#ifdef	HAVE_SOCKADDR_SA_LEN
		len = max(sizeof(struct sockaddr), ifr->ifr_addr.sa_len);
#else
		switch (ifr->ifr_addr.sa_family) {
#ifdef	IPV6
		case AF_INET6:	
			len = sizeof(struct sockaddr_in6);
			break;
#endif
		case AF_INET:	
		default:	
			len = sizeof(struct sockaddr);
			break;
		}
#endif
		
		ptr += sizeof(ifr->ifr_name) + len;
		if (ifr->ifr_addr.sa_family != family) {
			continue;
		}
		
		if (interface_is_up(ifr->ifr_name) == 1) {
		
		    if(sockfd != 0)
            {
                close(sockfd);
                sockfd = 0;
            }
			
    		if( buf != NULL)
            {
                free(buf);
                buf = NULL;
            }
            
    		return 1;
		}	
	}
	
	if( buf != NULL) 
	{
        free(buf);
        buf = NULL;
    }
	if(sockfd != 0)
    {
        close(sockfd);
        sockfd = 0;
    }
    
	return 0;
	
}

char *url_check(char *url, char *purl)
{
	char *sptr;
	int purllength = 0;
      char *urlptr = url;
      
	if ((sptr = strchr(url, ' ')) == NULL) {
               strncpy (purl, url, 1023);
               purl[1023] = '\0';
       } else {
           purl[0] = '\0';
           do {
			purllength += sptr-urlptr + 3;
			if (purllength >= 1023) {
					break;
			}
			strncat (purl, urlptr, sptr-urlptr);
			strcat (purl, "%20");
			urlptr = sptr + 1;
           } while ((sptr = strchr (urlptr, ' ')) != NULL);
           strcat (purl, urlptr);
       }
	return purl;
}


char *url_to_host_port (char *url, char **hname, unsigned short *port)
{
	char *h, *p;
	char *hostptr;
	char *r_hostptr;
	char *pathptr;
	char *portptr;
	char *p0;
	size_t stringlength;

	p = url;
	if (strncasecmp(p, "http://", 7) == 0)
		p += 7;

        if (strncasecmp(p, "ftp://", 6) == 0)
               p += 6;

	hostptr = p;
	while (*p && *p != '/')
		p++;
	pathptr = p;

	r_hostptr = --p;
	while (*p && hostptr < p && *p != ':' && *p != ']')
		p--;

	if (!*p || p < hostptr || *p != ':') {
		portptr = NULL;
	}
	else{
		portptr = p + 1;
		r_hostptr = p - 1;
	}
	if (*hostptr == '[' && *r_hostptr == ']') {
		hostptr++;
		r_hostptr--;
	}

	stringlength = r_hostptr - hostptr + 1;
	h = malloc(stringlength + 1); /* removed the strndup for better portability */
	if (h == NULL) {
		*hname = NULL;
		*port = 0;
		return NULL;
	}
	strncpy(h, hostptr, stringlength);
	*(h+stringlength) = '\0';
	*hname = h;

	if (portptr) {
		stringlength = (pathptr - portptr);
		if(!stringlength) portptr = NULL;
	}
	if (portptr == NULL) {
		portptr = "80";
		stringlength = strlen("80");
	}
	p0 = malloc(stringlength + 1);
	if (p0 == NULL) {
		free(h);
		*hname = NULL;
		*port = 0;
		return NULL;
	}
	strncpy(p0, portptr, stringlength);
	*(p0 + stringlength) = '\0';

	for (p = p0; *p && isdigit((unsigned char) *p); p++) ;

	*p = '\0';
	*port = atoi(p0);
	free(p0);
	return pathptr;
}


int file_get_length_by_name(char *name)
{
	int len = -1;
	FILE * file_stream;
	
	if (name == NULL) {
		return len;
	}
	
	file_stream = fopen(name, "rb");
	if(file_stream != NULL) {
		fseek(file_stream, 0, SEEK_END);
		len = ftell(file_stream);
		fclose(file_stream);
	}
	return len;
}
unsigned int  file_get_size(int fd)
{
    struct stat buf;

    if (fd < 0) {
	return 0;
    }
    if (fstat(fd, &buf)<0) {
        return 0;
    }
    return (unsigned int)(buf.st_size);
}

int file_get_length(FILE *file_stream)
{
	if (file_stream == NULL) {
		return -1;
	}
    	return file_get_size(fileno(file_stream));
}


void set_bit( int *map,int bit) 
{
	unsigned int tmp = 0x01;
	 tmp = tmp << bit;
	 *map = *map | tmp;
}

void clear_bit( int *map,int bit) 
{
	unsigned int tmp = 0x01;
	 tmp = tmp << bit;
	 *map = *map & (~tmp);
}
int get_bit( int map,int bit) 
{
	unsigned int tmp = 0x01;
	 tmp = tmp << bit;
	 return (map & tmp);
}
 int checkIntKey(char *buffer, char *key, unsigned int *value)
{
	int len=strlen(key);
	if(strncmp(buffer, key, len)==0 && buffer[len]=='=')
	{
		len+=1;
		if(buffer[len])
			*value=atoi(buffer+len);
		else
			*value=0;
		return 1;
	}
	return 0;
}


 int checkStringKey(char *buffer, char *key, char *sValue,int l)
{
	int len=strlen(key);
	if(strncmp(buffer, key, len)==0 && buffer[len]=='=')
	{
		len+=1;
		if(buffer[len])
			strncpy(sValue,&buffer[len],l);
		else
			*sValue=0;
		return 1;
	}
	return 0;
}

void clean_tmp_file(char *dir)
{
    DIR *dp;
    struct dirent *entry;
    struct stat statbuf;
    char file[128];
    
    if ((dp = opendir(dir)) == NULL) {
        fprintf(stderr,"cannot open directory: %s\n", dir);
        return;
    }
    while ((entry = readdir(dp)) != NULL) {
    	  snprintf(file,sizeof(file),"%s%s,",dir,entry->d_name);
        lstat(file,&statbuf);
        if (! S_ISDIR(statbuf.st_mode)) {
	    	if ((strstr(file,".txt") != NULL) && (strstr(file,".txt.") == NULL)) {
			remove(file);
	    	}
	  }
    }
    closedir(dp);
}


void write_string_to_file(char *file_name,char *string)
{
	FILE *f = fopen(file_name, "a+");

	fprintf(f,"%s\n", string);
	fflush(f);
	fclose(f);
}

int  write_file_with_offset(int filefp, int file_offset, char  *file_content, int size)
{
	int result;
	
	if (filefp < 0)
	{
		return -1;
	}
	
	lseek(filefp,file_offset,SEEK_SET);
	result = write(filefp,file_content,size);
	
	fsync(filefp);
	
	return (result == size);
}
int  read_file_with_offset(int filefp, int file_offset, char  *file_content, int size)
{
	int result;
	
	if (filefp < 0) {
		return TRUE;
	}
	
	lseek(filefp,file_offset,SEEK_SET);
	result = read(filefp,file_content,size);
	
	fsync(filefp);
	
	return (result == size);
}
int extractValueSample(char* s, char *key, char sp, char *value, int maxlen)
{
	char *cookieStart=NULL, *p;
	char cookieExp[200];
	int nl;

	if ((s == NULL) || (key == NULL) || (value == NULL)) {
		return  0;
	}
	
	*value=0;
	nl = snprintf(cookieExp,sizeof(cookieExp),"%s=", key);
	if (strncmp(s,cookieExp+1,nl-1) == 0) {
		cookieStart=s+nl-1;
	} else {
		cookieStart = strstr(s, cookieExp);
		if (cookieStart != NULL) {
			cookieStart += nl;
		}
	}
	if (cookieStart == NULL) {
		*value=0;
		return 0;
	}
	p = value;
	nl = 0;
	while (1) {
		int ch=*cookieStart++;
		if(ch==sp || ch==0)  {
			*p=0;
			return p-value;
		}
		*p++=ch;
		if (++nl >= maxlen) {
			break;
		}
	}
	return 0;
}

int extractValueInt(char* s, char *key, char sp, int defValue)
{
	char value[40];
	int len=extractValueSample(s, key, sp, value,sizeof(value));
	if(len<=0)
		return defValue;
	else
		return atoi(value);

}

int system_vfork(char * CmdString)
{
	int child_pid = -1;
	int status = 0;
	
	if (CmdString == NULL) {
		return -1;
	}
	child_pid = vfork();
	if (child_pid == 0) {
		execl("/bin/sh", "sh", "-c", CmdString, (char *)0);
		_exit(EXIT_SUCCESS);
	} else if (child_pid > 0) {
		waitpid(child_pid,&status,0);
		status = EXIT_SUCCESS;
	} else {
		status = -1;
		perror("vfork fail!!!\n");
	}
	
	return status;
}

void file_clr_data(char *filename)
{
	int fd;
	
	if (filename == NULL) {
		return ;	
	}
	fd = open(filename, O_RDWR|O_CREAT|O_TRUNC|O_SYNC,\
								S_IRWXU|S_IRWXG|S_IRWXO);
	fsync(fd);
	close(fd);
}
