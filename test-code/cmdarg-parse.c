#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void main (void)
{

	char *mystr = "This is a quick test";
	char c;
	char *cmdargs[16];
	char *buf;
	int j = 0, i = 0;

	c = *mystr;

	buf = (char *) malloc(32);

	while (c != '\0') {

	printf("The value of c is %c\n\n",c);
	
	    if (c == ' ') {
		strncpy(cmdargs[i],buf,sizeof(buf));
		i++;
		j = 0;
		free(buf);
		buf = (char *) malloc(32);
		buf[0] = '\0';	
	    } else {
		buf[j] = c;
		j++;
	    }

	    mystr++;
	    c = *mystr;

	}

	for (j = 0; j <= i; j++) {
	    printf("cmdarg %d is %s\n",j,cmdargs[j]);
	}

}
