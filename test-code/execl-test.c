#include <stdio.h>

void main(void)
{
	execl("/bin/sh","sh","-c","ip addr",(char *) 0);

}
