#include<stdio.h>
#include<stdlib.h>

string itostr(int n)


    {
    char tstr[12];
    itoa(n, tstr, 10);
    return string(tstr);
}

void main(void)
{
	int x = 102456;
	char *c;

	c = itostr(x);

	printf("\n\nstring = %s\n\n",c);
}


