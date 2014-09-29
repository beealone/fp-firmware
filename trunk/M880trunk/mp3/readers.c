#include <stdlib.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "mpg123.h"
#include "common.h"

static int get_fileinfo(struct reader *,char *buf);
static void readers_add_data(struct reader *rds,unsigned char *buf,int len);

/* can hold 4096-1 = 4095 bytes! */
#define BACKBUF_SIZE (8192)

/*******************************************************************
 * stream based operation
 */
static int bufdiff(struct reader *rds,int start, int end) 
{
  return (end >= start) ? end - start : rds->bufsize + end - start;
}

static int fullread(struct reader *rds,int fd,unsigned char *buf,int count)
{
    int ret,cnt=0;

    while(cnt < count) {
        int toread = count-cnt;
        int num = bufdiff(rds,rds->bufpos,rds->bufend);
  
        /* if we have some data in the backbuffer .. use it first */
        if(num > 0) {
           int part1,part2;

           if(toread > num)
              toread = num;

           part1 = rds->bufsize - rds->bufpos;
           if(part1 > toread)
               part1 = toread;
           part2 = toread - part1;
           memcpy(buf+cnt,&rds->backbuf[rds->bufpos],part1);
           if(part2 > 0)
             memcpy(buf+cnt+part1,&rds->backbuf[0],part2);
           rds->bufpos += toread;
           if(rds->bufpos >= rds->bufsize)
               rds->bufpos -= rds->bufsize;
           ret = toread;

           if(!rds->mark)
             rds->bufstart = rds->bufpos;
        }
        else {
	  ret = read(fd,buf+cnt,toread);

	  if(ret < 0)
	    return ret;
	  if(ret == 0)
	    break;

          if(rds->mark) {
            readers_add_data(rds,buf+cnt,ret);
            rds->bufpos = rds->bufend;
          }

        }
	cnt += ret;
    }
    return cnt; 
}

static void readers_add_data(struct reader *rds,unsigned char *buf,int len) 
{
  int diff,part1,part2,store = len;

            if(store >= rds->bufsize)
               store = rds->bufsize - 1;

            /* check whether the new bytes would overwrite the buffer front */
            diff = bufdiff(rds,rds->bufstart,rds->bufend);
            if(diff+store >= rds->bufsize) {
              fprintf(stderr,"Warning: backbuffer overfull %d %d\n",diff+store,rds->bufsize);
              /* +1 because end should never be the same as start if the is data in the buffer */
              rds->bufstart += diff + store + 1 - rds->bufsize;
              if(rds->bufstart >= rds->bufsize)
                   rds->bufstart -= rds->bufsize;
            }

            part1 = rds->bufsize - rds->bufend;
            if(part1 > store)
              part1 = store;
            part2 = store - part1;

            memcpy(rds->backbuf+rds->bufend,&buf[len-part1+part2],part1);
            if(part2 > 0)
               memcpy(rds->backbuf,&buf[len-part2],part2);

            rds->bufend += store;
            if(rds->bufend >= rds->bufsize)
               rds->bufend -= rds->bufsize;


}

void readers_pushback_header(struct reader *rds,unsigned long aLong) 
{
  unsigned char buf[4];

  if(rds->mark || (rds->bufpos != rds->bufend) ) {
    rds->bufpos -= 4;
    if(rds->bufpos < 0)
        rds->bufpos += rds->bufsize;
  }
  else {
    buf[0] = (aLong>>24) & 0xff;
    buf[1] = (aLong>>16) & 0xff;
    buf[2] = (aLong>>8)  & 0xff;
    buf[3] = (aLong>>0)  & 0xff;
  }
  
  readers_add_data(rds,buf,4);
}

void readers_mark_pos(struct reader *rds) {
 /* fprintf(stderr,"M%d ",rds->bufpos); */
  rds->bufstart = rds->bufpos;
  rds->mark     = 1;
}

void readers_goto_mark(struct reader *rds) {
 /*  fprintf(stderr,"G%d ",rds->bufstart); */
  rds->mark = 0;
  rds->bufpos = rds->bufstart;
}


static int default_init(struct reader *rds)
{
    char buf[128];

    rds->mark = 0;
    rds->bufend   = 0;
    rds->bufstart = 0;
    rds->bufpos   = 0;
    rds->bufsize = BACKBUF_SIZE;

    rds->backbuf = (unsigned char *) MALLOC(rds->bufsize);

    rds->filepos = 0;
    rds->filelen = get_fileinfo(rds,buf);
  
    if(rds->filelen > 0) {
	if(!strncmp(buf,"TAG",3)) {
	    rds->flags |= READER_ID3TAG;
	    memcpy(rds->id3buf,buf,128);
	}
    }
    return 0;
}

void stream_close(struct reader *rds)
{
    if (rds->flags & READER_FD_OPENED)
        close(rds->filept);
}

/**************************************** 
 * HACK,HACK,HACK: step back <num> frames 
 * can only work if the 'stream' isn't a real stream but a file
 */
static int stream_back_bytes(struct reader *rds,int bytes)
{
    if(lseek(rds->filept,-bytes,SEEK_CUR) < 0)
	return -1;
    return 0;
}

static int stream_back_frame(struct reader *rds,struct frame *fr,int num)
{
    long bytes;
    int skipped;

    if(!fr->firsthead)
	return 0;

    bytes = (fr->framesize+8)*(num+2);

    if(lseek(rds->filept,-bytes,SEEK_CUR) < 0)
	return -1;

    sync_stream(rds,fr,0xffff,&skipped);

    read_frame(rds,fr);
    read_frame(rds,fr);

    if(fr->lay == 3) {
	set_pointer(fr->sideInfoSize,512);
    }

    return 0;
}

static int stream_head_read(struct reader *rds,unsigned long *newhead)
{
    unsigned char hbuf[4];

    if(fullread(rds,rds->filept,hbuf,4) != 4)
	return FALSE;
  
    *newhead = ((unsigned long) hbuf[0] << 24) |
	((unsigned long) hbuf[1] << 16) |
	((unsigned long) hbuf[2] << 8)  |
	(unsigned long) hbuf[3];
  
    return TRUE;
}

static int stream_head_shift(struct reader *rds,unsigned long *head)
{
    unsigned char hbuf;

    if(fullread(rds,rds->filept,&hbuf,1) != 1)
	return 0;
    *head <<= 8;
    *head |= hbuf;
    *head &= 0xffffffff;
    return 1;
}

static int stream_skip_bytes(struct reader *rds,int len)
{
    return lseek(rds->filept,len,SEEK_CUR);
}

static int stream_read_frame_body(struct reader *rds,unsigned char *buf,
				  int size)
{
    long l;

    if( (l=fullread(rds,rds->filept,buf,size)) != size)
	{
	    if(l <= 0)
		return 0;
	    memset(buf+l,0,size-l);
	}

    return 1;
}

static long stream_tell(struct reader *rds)
{
    return lseek(rds->filept,0,SEEK_CUR);
}

static void stream_rewind(struct reader *rds)
{
    lseek(rds->filept,0,SEEK_SET);
}

/*
 * returns length of a file (if filept points to a file)
 * reads the last 128 bytes information into buffer
 */
static int get_fileinfo(struct reader *rds,char *buf)
{
    int len;

    if((len=lseek(rds->filept,0,SEEK_END)) < 0) {
	return -1;
    }
    if(lseek(rds->filept,-128,SEEK_END) < 0)
	return -1;
    if(fullread(rds,rds->filept,(unsigned char *)buf,128) != 128) {
	return -1;
    }
    if(!strncmp(buf,"TAG",3)) {
	len -= 128;
    }
    if(lseek(rds->filept,0,SEEK_SET) < 0)
	return -1;
    if(len <= 0)
	return -1;
    return len;
}

/*****************************************************************
 * read frame helper
 */

struct reader *rd;
struct reader readers[] = {
    { default_init,
      stream_close,
      stream_head_read,
      stream_head_shift,
      stream_skip_bytes,
      stream_read_frame_body,
      stream_back_bytes,
      stream_back_frame,
      stream_tell,
      stream_rewind } ,
    { NULL, }
};


/* open the device to read the bit stream from it */

struct reader *open_stream(char *bs_filenam,int fd)
{
    int i;
    int filept_opened = 1;
    int filept;

    if (!bs_filenam) {
	if(fd < 0) {
	    filept = 0;
	    filept_opened = 0;
	}
	else
	    filept = fd;
    }
    else if (!strncasecmp(bs_filenam, "http://", 7)){ 
	filept = http_open(bs_filenam);
	if(filept < 0) return NULL;
    }
    else if (!strncasecmp(bs_filenam, "ftp://", 6)){
        filept = http_open(bs_filenam);
	if(filept < 0) return NULL;
    }

#ifndef O_BINARY
#define O_BINARY (0)
#endif
    else if ( (filept = open(bs_filenam, O_RDONLY|O_BINARY)) < 0) {
	perror (bs_filenam);
	return NULL;
    }

    rd = NULL;
    for(i=0;;i++) {
	readers[i].filelen = -1;
	readers[i].filept  = filept;
	readers[i].flags = 0;
	if(filept_opened)
	    readers[i].flags |= READER_FD_OPENED;
	if(!readers[i].init) {
	    fprintf(stderr,"Fatal error!\n");
	    return NULL;
	}
	if(readers[i].init(readers+i) >= 0) {
	    rd = &readers[i];
	    break;
	}
    }

    if(rd && rd->flags & READER_ID3TAG) {
	print_id3_tag(rd->id3buf);
    }

    return rd;
}

