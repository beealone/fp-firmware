#include <stdio.h>
#include "serial.h"

int main(int argc, char **argv)
{
	char buf[128];
	unsigned char cmd[128];
	int rdnum,i;

	ARCA_IO_Init();

	ff232.init(38400, V10_8BIT, V10_NONE, 0);

	//RS485_setmode(FALSE);
	//RS485_setmode(TRUE);

	strcpy(cmd, "AT\r");
	//cmd[2]=13;
	//cmd[3]='\0';	

	//ff232.flush_input();

	SerialOutputString(&ff232, cmd);

	DelayUS(300*1000);
	
	printf("Numbers %d \n", ff232.poll());

	rdnum=ff232.poll();
	i=0;
	while(i<rdnum)
	{
		printf("%d ", ff232.read());
		i++;	
	}

	printf("\n");

	//TestSerial();

	//rdnum=SerialInputString(&ff232, buf, 50, 5);

	//printf("%s %d\n", buf, rdnum);

	ff232.free();

	ARCA_IO_Free();
}
