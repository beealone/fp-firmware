/*************************************************
                                           
 ZEM 200                                          
                                                    
 serial.c 
                                                      
 Copyright (C) 2003-2004, ZKSoftware Inc.      		
                                                      
*************************************************/

#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <time.h>
#include "arca.h"
#include "serial.h"
#include "utils.h"

static int fdRS232 = -1;
static int fdRS0232 = -1;

static int MapBaudrate[V10_NUM_BAUDRATES]={
    B0, B50, B75, B110, B134, B150, B200, B300, B600, B1200, B1800, 
    B2400, B4800, B9600, B19200, B38400, B57600, B115200};


/** This array is used to map the size of the data byte to the value of the
 * target system. The library assume, that we allways have 5-8 bit per byte.
 */
static int MapDatasize[V10_NUM_DATASIZES]={CS5, CS6, CS7, CS8};

int ConvertBaudrate(int baudrate)
{   
    if (baudrate <= 1200)   return V10_B1200;
    if (baudrate >= 115200) return V10_B115200;
    if (baudrate == 1800)   return V10_B1800;    
    if (baudrate == 2400)   return V10_B2400;
    if (baudrate == 4800)   return V10_B4800;
    if (baudrate == 9600)   return V10_B9600;
    if (baudrate == 19200)  return V10_B19200;
    if (baudrate == 38400)  return V10_B38400;
    if (baudrate == 57600)  return V10_B57600;
    
    return V10_B9600;
}

int RS232_SetParameters(int port, int Baudrate, int Datasize, int Parity, int FlowControl)
{
    struct termios options;

    if ((Baudrate < 0) || (Baudrate>=V10_NUM_BAUDRATES)) return 0;    
    if ((Datasize<0) || (Datasize>=V10_NUM_DATASIZES))	return 0;

    //Get the current options for the port...
    tcgetattr(port, &options);

    /* General Setup: the library is design for `raw mode'. Only 1 stop can be
     * configured. There's no special `break handling'. The input stream
     * shouldn't be modified. Therefore parity error aren't marked and the
     * input mapping is disabled.
     */
    cfmakeraw(&options);
    
    /* handle the handshaking according to the open flags */
    if (FlowControl)
	    options.c_cflag |= CRTSCTS;
    else
	    options.c_cflag &= ~CRTSCTS;

    options.c_cflag &= ~HUPCL;
    
    options.c_iflag &= ~(IXON|IXOFF|IXANY);

    /* Decide wether to block while waiting for character or return immediatly.
     */
    options.c_cc[VMIN]  = 0;
    options.c_cc[VTIME] = 50;
    
    /* Mask the character size bits and set data bits according to the
     * parameter.
     */
    options.c_cflag &= ~CSIZE;
    options.c_cflag |= MapDatasize[Datasize]; 

    /* Set the handling of the parity bit.
     */
    switch (Parity){
	case V10_NONE:			     /* disable parity bit */
	    options.c_cflag &= ~PARENB;
	    options.c_iflag &= ~INPCK;
	    break;
	case V10_EVEN:			     /* even parity */
	    options.c_cflag |= PARENB;
	    options.c_cflag &= ~PARODD;
	    options.c_iflag &= ~IGNPAR;
	    options.c_iflag |= INPCK;	     /* removed "|ISTRIP" */
	    break;
	case V10_ODD:			     /* odd parity */
	    options.c_cflag |= PARENB;
	    options.c_cflag |= PARODD;
	    options.c_iflag &= ~IGNPAR;
	    options.c_iflag |= INPCK;	     /* removed "|ISTRIP" */
	    break;
	case V10_IGNORE:		     /* use parity but dont test */
	    options.c_cflag |= PARENB;
	    options.c_iflag |= IGNPAR;
	    break;
	default:
	    return 0;
    }

    /* We have to enable the receiver and set the port to local mode.
     */
    options.c_cflag |= (CLOCAL|CREAD);

    /* Set the baud rates according to the parameter.*/
    cfsetispeed(&options, MapBaudrate[Baudrate]);
    cfsetospeed(&options, MapBaudrate[Baudrate]);

    /* At last we must set the new options for the port.
     */
    tcsetattr(port, TCSANOW, &options);

    return 1;
}

int RS232_SetStopbits (int port, int Stops)
{
    struct termios options;

    /* Get the current options for the port... */
    if (Stops == 1){
	tcgetattr(port, &options);
	options.c_cflag &= ~CSTOPB;
	tcsetattr(port, TCSANOW, &options);
    }
    else if (Stops == 2){
	tcgetattr(port, &options);
	options.c_cflag |= CSTOPB;
	tcsetattr(port, TCSANOW, &options);
    }
    else
	return 0;

    return 1;
}

int RS232_SetTimeouts (int port, int TenthOfSeconds)
{
    struct termios options;

    if ( TenthOfSeconds <= 0 ) return 0;

    tcgetattr(port, &options);
    options.c_cc[VMIN] = 0;		     /* we want `interchar timeouts' */
    options.c_cc[VTIME] = TenthOfSeconds;
    tcsetattr(port, TCSANOW, &options);
    
    return 1;
}

// Initialise serial port at the request baudrate. 
static int arca_serial_init(int BaudRate, int DataSize, int Parity, int FlowControl)
{   
    if (fdRS232 > 0) close(fdRS232);
    fdRS232 = open("/dev/ttyS3", O_RDWR | O_NOCTTY);
    if ( fdRS232 < 0) 
	return -1;
    
    RS232_SetParameters(fdRS232, ConvertBaudrate(BaudRate), DataSize, Parity, FlowControl);
    RS232_SetStopbits(fdRS232, 1);
    return 0;
}

/* Flush serial input queue. 
 * Returns 0 on success or negative error number otherwise
 */
static int arca_serial_flush_input(void)
{    
    tcflush(fdRS232, TCIFLUSH);
    return 0;
}

/* Flush output queue. 
 * Returns 0 on success or negative error number otherwise
 */
static int arca_serial_flush_output(void)
{
    //tcflush(fdRS232, TCOFLUSH);
    DelayUS(1000);
    return 0;
}

/* Check if there is a character available to read. 
 * Returns 1 if there is a character available, 0 if not, 
 */
static int arca_serial_poll(void)
{
    int CharsWaiting = 0;
                                                                                                               
    ioctl(fdRS232, FIONREAD, &CharsWaiting);
    
    //if (CharsWaiting >= 1) CharsWaiting = 1;
    if (CharsWaiting < 0) CharsWaiting = 0;
    
    return CharsWaiting;   
}

/* read one character from the serial port. return character (between
 * 0 and 255) on success, or negative error number on failure. this
 * function is not blocking */
static int arca_serial_read(void)
{
    unsigned char TheData;
                                                                                                               
    if (read(fdRS232, &TheData, 1) != 1)
    {
        return -1;
    }
    return (int)TheData;
}

/* write character to serial port. return 0 on success, or negative
 * error number on failure. this function is blocking
 */
static int arca_serial_write(int c)
{
    unsigned char TheData;
    
    TheData = c&0xff;
    
    if (write(fdRS232, &TheData, 1) != 1)
    {
        return -1;
    }
    
    return 0;
}

static void arca_serial_free(void)
{
    if (fdRS232 > 0) close(fdRS232);
}

/* export serial driver */
serial_driver_t ff232 = {
	arca_serial_init,
	arca_serial_read,
	arca_serial_write,
	arca_serial_poll,
	arca_serial_flush_input,
	arca_serial_flush_output,
	arca_serial_free
};

// Initialise serial port at the request baudrate. 
static int arca0_serial_init(int BaudRate, int DataSize, int Parity, int FlowControl)
{   
    if (fdRS0232 > 0) close(fdRS0232);
    fdRS0232 = open("/dev/ttyUART0", O_RDWR | O_NOCTTY);
    if ( fdRS0232 < 0) 
	return -1;
    
    RS232_SetParameters(fdRS0232, ConvertBaudrate(BaudRate), DataSize, Parity, FlowControl);
    RS232_SetStopbits(fdRS0232, 1);
    return 0;
}

/* Flush serial input queue. 
 * Returns 0 on success or negative error number otherwise
 */
static int arca0_serial_flush_input(void)
{    
    tcflush(fdRS0232, TCIFLUSH);
    return 0;
}

/* Flush output queue. 
 * Returns 0 on success or negative error number otherwise
 */
static int arca0_serial_flush_output(void)
{
    //tcflush(fdRS0232, TCOFLUSH);
    DelayUS(1000);
    return 0;
}

/* Check if there is a character available to read. 
 * Returns 1 if there is a character available, 0 if not, 
 * and negative error number on failure.
 */
static int arca0_serial_poll(void)
{
    int CharsWaiting = 0;
                                                                                                               
    ioctl(fdRS0232, FIONREAD, &CharsWaiting);
    
    if (CharsWaiting >= 1) CharsWaiting = 1;
    
    return CharsWaiting;   
}

/* read one character from the serial port. return character (between
 * 0 and 255) on success, or negative error number on failure. this
 * function is not blocking */
static int arca0_serial_read(void)
{
    unsigned char TheData;
                                                                                                               
    if (read(fdRS0232, &TheData, 1) != 1)
    {
        return -1;
    }
    return (int)TheData;
}

/* write character to serial port. return 0 on success, or negative
 * error number on failure. this function is blocking
 */
static int arca0_serial_write(int c)
{
    unsigned char TheData;
    
    TheData = c&0xff;
    
    if (write(fdRS0232, &TheData, 1) != 1)
    {
        return -1;
    }
    
    return 0;
}

static void arca0_serial_free(void)
{
    if (fdRS0232 > 0) close(fdRS0232);
}

/* export serial driver */
serial_driver_t st232 = {
	arca0_serial_init,
	arca0_serial_read,
	arca0_serial_write,
	arca0_serial_poll,
	arca0_serial_flush_input,
	arca0_serial_flush_output,
	arca0_serial_free
};

//工作方式:
//平时RS485工作在接收模式,收到主机发送的数据包后,验证后才进入发送模式,
//请在判断FIFO发送缓冲区空后返回接收模式 
void RS485_ChangeStatus(serial_driver_t *serial, U32 SendMode)
{
    if (SendMode){
	gpio_reg->gpio_pddr |= (RS485_SEND);
	serial->flush_input();  // RS485 Send mode
    }else{
	gpio_reg->gpio_pddr &= ~(RS485_SEND);
	serial->flush_output(); // RS485 Receive mode
    }
    DelayUS(1000);
}

void RS485_setmode(U32 SendMode)
{
    RS485_ChangeStatus(&ff232, SendMode);
}

/*
 * Write a null terminated string to the serial port.
 */
void SerialOutputString(serial_driver_t *serial, const char *s)
{
	while(*s != 0)
		serial->write(*s++);
} /* SerialOutputString */

int SerialInputString(serial_driver_t *serial, char *s, const int len, const int timeout)
{
	U32 startTime, currentTime;
	int c;
	int i;
	int numRead;
	int skipNewline = 1;
	int maxRead = len - 1;
	
	startTime = clock();

	for(numRead = 0, i = 0; numRead < maxRead;){
		/* try to get a byte from the serial port */
		while(serial->poll() == 0){
			currentTime = clock();

			/* check timeout value */
			if((currentTime - startTime) > (timeout * CLOCKS_PER_SEC)){
				/* timeout */
				s[i++] = '\0';
				return(numRead);
			}
		}
		c = serial->read();

		/* check for errors */
		if(c < 0) {
			s[i++] = '\0';
			return c;
		}
		printf("%d ", c);
		//serial->write(c);
		/* eat newline characters at start of string */
		if((skipNewline == 1) && (c != '\r') && (c != '\n'))
			skipNewline = 0;

		if(skipNewline == 0) {
			if((c == '\r') || (c == '\n')) {
				s[i++] = '\0';
				return(numRead);
			} else {
				s[i++] = (char)c;
				numRead++;
			}
		}
	}

	return(numRead);
}


void SerialPrintf(serial_driver_t *serial, char * fmt, ...)
{
    char buffer[256];
    va_list ap;

    va_start(ap,fmt);
    doPrint(buffer,fmt,ap);
    SerialOutputString(serial, buffer);
}

void TestSerial(void)
{
	char buf[50];
	int rdnum;

	RS485_setmode(TRUE);
	SerialOutputString(&ff232,"\r\nInput string 'String 101':");
	DelayUS(1000*20);
	RS485_setmode(FALSE);
	rdnum=SerialInputString(&ff232, buf, 50, 5);
	RS485_setmode(TRUE);
	if(rdnum==strlen("String 101"))
	{
		if(strcmp("String 101",buf)==0)
		{
			SerialOutputString(&ff232, "\r\n\tSerial port is OK!\r\n");
		}
		else
		{
			SerialPrintf(&ff232, "\r\n\tError: Serial port input is '%s'\r\n", buf);
		}
	}
	else if(rdnum)
	{
		SerialPrintf(&ff232, "\r\n\tError: Serial port input is '%s'\r\n", buf);
	}
	else
		SerialOutputString(&ff232, "\r\nNo input!\r\n");
}
	
