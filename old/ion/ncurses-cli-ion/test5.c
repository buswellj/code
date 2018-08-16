#include<stdio.h>
#include<ncurses.h>

int
main(int argc, char *argv[])
{

	char ch[50];

	while((ch = getch()) != KEY_F(1))
	{
		switch(ch) {
		case KEY_DOWN:
			printf(" Key down \n");
			break;

		case KEY_UP:
			printf(" Key up \n");
			break;
		}
	}

}
