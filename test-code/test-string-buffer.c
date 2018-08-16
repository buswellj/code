#include <stdio.h>

int
main(void)
{

	int x = 0;
	char c;

	printf("\n\n");

	for (x = 0x61; x <0x7b; x++) {
	    c = x;
	    printf("%c",c);
	}

	printf("\n\n");

	for (x = 0x41; x <0x5b; x++) {
	    c = x;
	    printf("%c",c);
	}
	printf("\n\n");
}
