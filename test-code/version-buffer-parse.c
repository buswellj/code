#include <stdio.h>
#include <string.h>

void main(void)
{
	char *foo = "/vsitypes/1/";
	char *c;
	char z;
	int s = strlen(foo);
	int x = 0;
	int y = 0;
	char version[32];
	char id[32];

	printf("%d\n\n",s);

	for (c = foo; *c; c++) {
		z = (char) c[0];
		if (y == 2) {
		    if (z != '/') {
		    	version[x] = z;
		        x++;
		    }
		}
		if (y == 3) {
		    id[x] = z;
		    x++;
		}
		if (z == '/') {
		    y++;
		    if (y == 2) {
			x = 0;
		    } else if (y == 3) {
			version[x] = '\0';
		        x = 0;
		    }
		}		
	}
	id[x] = '\0';

	printf("\n version is %s\nid is %s\n idlen = %d",version,id,strlen(id));

	printf("\n\n");

}
