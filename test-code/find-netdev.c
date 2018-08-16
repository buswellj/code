#include <stdio.h>
#include <string.h>

int main(void)
{
	char entry[512];
	char *delimit;
	char *devname;
	FILE *fp;

	if (!(fp = fopen("/proc/net/dev", "r")))
		return 1;

	while((devname = fgets(entry, 512, fp))) {
		while(isspace(devname[0]))
		    devname++;

		delimit = strchr (devname, ':');
		if (delimit) {
		    *delimit = 0;
		    if (strlen(devname) > 3) printf("\n\nlong\n\n");
		    if (!strncmp(devname,"lo",2))
		        printf("%s \n",devname);
		}
	}
	fclose(fp);
	return 0;

}
