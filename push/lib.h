#ifndef LIB_H_
#define LIB_H_
#include <stdio.h>
#include "thread.h"
#ifndef  NULL
#define NULL (void *)0
#endif

#ifndef XL_DEBUG
#define ERROR_MSG(format, arg...) 	\
do { \
	fprintf(stderr,"%s:%s:%d:",__FILE__,__FUNCTION__,__LINE__);	\
	fprintf(stderr,"error:"); 			\
	fprintf(stderr,format, ## arg); 	\
} while (0)
#else 
#define ERROR_MSG(format, arg...) 	
#endif


#define assert_debug(p) do { \
		if (!(p)) { \
			printf("%s:%d Warning: "#p" not Expect.\n", __func__, __LINE__);\
		} \
	}while (0)



int interface_get_stat(int family);
char *url_check(char *url, char *purl);
struct hostent *get_host_by_name_cache(const char *name);
char *url_to_host_port (char *url, char **hname, unsigned short *port);
int file_get_length(FILE *file_stream);
int file_get_length_by_name(char *name);
void set_bit( int *map,int bit) ;
void clear_bit( int *map,int bit) ;
int get_bit( int map,int bit) ;
int checkIntKey(char *buffer, char *key, unsigned int *value);
int checkStringKey(char *buffer, char *key, char *sValue,int l);
void write_string_to_file(char *file_name,char *string);
int  file_copy(char *dest_file,char *src_file);
int  write_file_with_offset(int filefp, int file_offset, char  *file_content, int size);
int  read_file_with_offset(int filefp, int file_offset, char  *file_content, int size);
void clean_tmp_file(char *dir);
int extractValueSample(char* s, char *key, char sp, char *value, int maxlen);
int extractValueInt(char* s, char *key, char sp, int defValue);
unsigned int  file_get_size(int fd);
int system_vfork(char * CmdString);
void file_clr_data(char *filename);
#endif

