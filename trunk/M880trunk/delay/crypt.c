#include <stdio.h>
#include <crypt.h>
#define _XOPEN_SOURCE
#include <unistd.h>

int main(int argc, char **argv)
{	
	printf("key=%s  password=%s\n", "visual1", crypt("solokey", "$1$"));
}
