#include<stdio.h>
#include<stdlib.h>
#include<termios.h>
#include<unistd.h>
#include<fcntl.h>

int
main(int argc, char *argv[])
{
	char buf[128];
	int i = 0;
	int b = 0;
	char ch;
	struct termios orig_term;
	struct termios raw_term;

	if (tcgetattr(0,&orig_term) != 0) b = 1;
	if (tcsetattr(0,TCSANOW,&raw_term) != 0) b = 2;

	fcntl(0, F_SETFL, O_NONBLOCK);

	while(1)
	{
		ch = getc(stdin);
		//if (ch == 255 || ch == 3) continue;
		if (ch == 'a' || ch == 13 || ch == 10) break;
		printf("(%c) (%d) (0x%x) \n", ch, ch, ch);
		if (!isalpha(ch)) continue;
		if (i > 127) break;
		else
		buf[i++] = ch;
	}

	if (tcsetattr(0,TCSANOW, &orig_term) != 0) return 1;


}
