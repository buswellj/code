#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include<errno.h>

int
main(int argc, char *argv[])
{

	char cli_banner[50];
	unsigned char c;
	int n;

	printf("\n\nTEST\n\n");
	fprintf(stdout,"ion> ");
	fflush(stdout);

	while (1) {

		if ((n = read(STDIN_FILENO, &c, 1)) < 0) {
			if (errno == EINTR) continue;
			break;
		}

		if (n == 0) break;

		if (c == '\177') exit(EXIT_SUCCESS);

		fprintf(stdout,"%c",c);
		fflush(stdout);


	}
	

}
