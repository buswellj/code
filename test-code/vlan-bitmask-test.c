#include <stdio.h>

void main(void)
{
	unsigned int mask = 0;
	unsigned int vlan = 20;
	unsigned int vlanlow = 1;
	unsigned int vlanhigh = 4095;
	unsigned long test = 0;
	unsigned int crap = 2102;

	printf("\n\nVLAN test bitmask\n\n");

	test = (1 << vlan) | (1 << vlanlow) | (1 << vlanhigh);

	printf("stored value is %lx, vlan value is %lu, is true is %d\n",test,vlan,(test & (1 << vlan)));
	printf("stored value is %lx, vlan value is %lu, is true is %d\n",test,vlanlow,(test & (1 << vlanlow)));
	printf("stored value is %lx, vlan value is %lu, is true is %lu\n",test,vlanhigh,(test & (1 << vlanhigh)));
	printf("stored value is %lx, vlan value is %lu, is true is %lu\n",test,crap,(test & (1 << crap)));

	test ^= (1 << vlan);
	printf("\n\nstored value is %lx, vlan value is %lu, is true is %d\n",test,vlan,(test & (1 << vlan)));



}
