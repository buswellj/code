#include <stdio.h>

void
main(void)
{
	unsigned long hash = 8021;
	char *str = "27c732ea-56c9-4f66-8c28-328638ccebbe";
	int c = 0;

	while ((c = *str++))
	    hash = ((hash << 5) + hash) + c;

	printf("\n\n final result is %lu %lu\n\n",hash,(hash & 0xffff));

}
