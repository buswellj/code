#include<stdio.h>

void main(void)
{
	int x = 0;

	printf("\n\n");

	for (x = 1; x <= 255; x++) {
		printf("/cfg/ip/route/add 192.168.46.%d 255.255.255.255 192.168.99.9 9\n",x);
	}

	printf("\n\n");
}	
