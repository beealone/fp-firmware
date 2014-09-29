/*************************************************
                                           
 ZEM 200                                          
                                                    
 serial.h 
                                                      
 Copyright (C) 2003-2004, ZKSoftware Inc.      		
                                                      
*************************************************/

#ifndef _SERIAL_H_
#define _SERIAL_H_

#include "arca.h"

enum __ARCA_BAUDRATE
{
    V10_B0=0,				     /* used to drop DTR */
    V10_B50,				     /* 50 baud */
    V10_B75,				     /* 75 baud */
    V10_B110,				     /* 110 baud */
    V10_B134,				     /* 134.5 baud */
    V10_B150,				     /* 150 baud */
    V10_B200,				     /* 200 baud */
    V10_B300,				     /* 300 baud */
    V10_B600,				     /* 600 baud */
    V10_B1200,			     /* 1200 baud */
    V10_B1800,			     /* 1800 baud */
    V10_B2400,			     /* 2400 baud */
    V10_B4800,			     /* 4800 baud */
    V10_B9600,			     /* 9600 baud */
    V10_B19200,			     /* 19200 baud */
    V10_B38400,			     /* 38400 baud */
    V10_B57600,			     /* 57,600 baud */
    V10_B115200,		     /* 115,200 baud */
    V10_NUM_BAUDRATES		     /* the number of entries */    
};


enum __ARCA_DATASIZE
{
    V10_5BIT=0,
    V10_6BIT,
    V10_7BIT,
    V10_8BIT,
    V10_NUM_DATASIZES		     /* number of datasize values */
};


enum __ARCA_PARITY_FLAGS
{
    V10_NONE=0,			     /* disable parity bit */
    V10_EVEN,			     /* even parity */
    V10_ODD,			     /* odd parity */
    V10_IGNORE			     /* use parity but do not test it */
};

typedef int (*serial_init_func_t)(int, int, int, int);
typedef int (*serial_read_func_t)(void);
typedef int (*serial_write_func_t)(int);
typedef int (*serial_poll_func_t)(void);
typedef int (*serial_flush_input_func_t)(void);
typedef int (*serial_flush_output_func_t)(void);
typedef void (*serial_free_func_t)(void);
typedef int (*serial_tcdrain_func_t)(void);
typedef int (*serial_get_lsr_func_t)(void);
typedef int (*serial_write_buf_func_t)(unsigned char*,int);
typedef int (*serial_read_buf_func_t)(unsigned char*,int);
typedef struct {
	serial_init_func_t init;

	serial_read_func_t read;
	serial_write_func_t write;

	serial_poll_func_t poll;

	serial_flush_input_func_t flush_input;
	serial_flush_output_func_t flush_output;
	
	serial_tcdrain_func_t tcdrain;
	
	serial_free_func_t free;
	
	serial_get_lsr_func_t get_lsr;
	serial_write_buf_func_t write_buf;
	serial_read_buf_func_t read_buf;
} serial_driver_t;

/* implemented serial drivers */

extern serial_driver_t ff232;
extern serial_driver_t st232;
extern serial_driver_t ttl232;
serial_driver_t bt232;
extern serial_driver_t ct232;
extern serial_driver_t usb232;

void RS485_setmode(U32 SendMode);

void SerialOutputString(serial_driver_t *serial, const char *s);
int  SerialInputString(serial_driver_t *serial, char *s, const int len, const int timeout);
void SerialPrintf(serial_driver_t *serial, char * fmt, ...);
void TestSerial(void);

#endif
