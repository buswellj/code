#include <stdio.h>

void 
main(void)
{
	int foo = 1;
	int notfoo = 0;

	if (!!foo) {
	   printf("\n\n This is not not foo, foo is 1 so not foo is 0 and not not foo is 1\n\n");
	}


	if (!!notfoo) {
	   printf("\n\n This is not not notfoo, notfoo is 0 so not foo is 1 and not not foo is 0\n\n");
	}



}
