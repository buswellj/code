#include <stdlib.h>

int main(int argc, char *argv[])
{
	int x = 0;
	char *cmd = "/usr/bin/passwd";

	x = system(cmd);

	return(0);

}
