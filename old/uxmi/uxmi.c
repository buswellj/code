/*
 * uxmi.c
 *
 * UxMI - User e(x)tendable Management Interface
 * 
 * All Rights Reserved.
 * Copyright (c) 2002-2005 Spliced Networks LLC
 *
 */

#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <sys/types.h>
#include "uxmi.h"

void uxmi_print_usage(void);
void uxmi_color_reset(void);
void uxmi_color_bg(int uxmi_color);
void uxmi_color_fg(int uxmi_color);
int uxmi_system(char *xcmd);

int main(int argc, char *argv[])
{
	uid_t uxmi_ckuid = getuid();

	if (uxmi_ckuid && uxmi_ckuid != 1001) {
	    uxmi_print_usage();
	    printf("This program must be run as root, executed with UID(%d).\n\n",uxmi_ckuid);
	    exit(-1);
	}

	if (argc < 2) {
		uxmi_print_usage();
		exit(-1);
	}

	if ((strlen(argv[1]) == 1) && isdigit(argv[1][0])) {
		u8 rlvl = strtol(argv[1], (char **) NULL, 10);

		if (rlvl > UXMI_RL_REBOOT) {
		    uxmi_print_usage();
		    exit(-1);
		}

		printf("\nRunlevel - %d\n\n", rlvl);


		switch (rlvl) {
			case UXMI_RL_HALT:
			
				uxmi_halt();
				exit(0);

				break;

			case UXMI_RL_MAINT:

				uxmi_maint();
				break;

			case UXMI_RL_MULTIUSER:

				uxmi_multiuser();
				break;

			case UXMI_RL_UPGRADE:

				uxmi_upgrade();
				break;

			case UXMI_RL_REBOOT:

				uxmi_reboot();
				break;

			case UXMI_RL_UNUSED_TWO:
			case UXMI_RL_UNUSED_FIVE:

				break;
		}
		exit(1);
	} else {
		if (!strncmp(argv[1],"sysinit",7)) {
			uxmi_sysinit();
		} else {
			uxmi_print_usage();
			exit(-1);		   
		}
	}

	exit(0);
}

void
uxmi_print_usage()
{
	printf("\nUxMI %s\n",UXMI_VERSION);
	printf("%s\n\n",UXMI_COPYRIGHT);
	printf("Usage:\n\t uxmi <cmd> [options]\n\n");
	printf("\t <level> = switch to runlevel <level>\n");
	printf("\t sysinit = sysinit\n");
	printf("\n\n");
}

void
uxmi_color_reset()
{
	printf("\033[0;m");
}

void
uxmi_color_bg(int uxmi_color)
{
	int sel_color = 40;

	switch (uxmi_color) {

		case UXMI_BLACK:
			sel_color = 40;
			break;

		case UXMI_BLUE:
			sel_color = 44;
			break;

		case UXMI_GREEN:
			sel_color = 42;
			break;

		case UXMI_CYAN:
			sel_color = 46;
			break;

		case UXMI_RED:
			sel_color = 41;
			break;

		case UXMI_PURPLE:
			sel_color = 45;
			break;

		case UXMI_BROWN:
			sel_color = 43;
			break;

		default:
			sel_color = 47;
			break;

	}

	printf("\033[%dm", sel_color);
	return;
}

void
uxmi_color_fg(int uxmi_color)
{
	int sel_opt = 1;
	int sel_color = 37;

	switch (uxmi_color) {

		case UXMI_BLACK:
			sel_opt = 0;
			sel_color = 30;
			break;

		case UXMI_BLUE:
			sel_opt = 0;
			sel_color = 34;
			break;

		case UXMI_GREEN:
			sel_opt = 0;
			sel_color = 32;
			break;

		case UXMI_CYAN:
			sel_opt = 0;
			sel_color = 36;
			break;

		case UXMI_RED:
			sel_opt = 0;
			sel_color = 31;
			break;

		case UXMI_PURPLE:
			sel_opt = 0;
			sel_color = 35;
			break;

		case UXMI_BROWN:
			sel_opt = 0;
			sel_color = 33;
			break;

		case UXMI_LGRAY:
			sel_opt = 0;
			sel_color = 37;
			break;

		case UXMI_DGRAY:
			sel_opt = 1;
			sel_color = 30;
			break;

		case UXMI_LBLUE:
			sel_opt = 1;
			sel_color = 34;
			break;

		case UXMI_LGREEN:
			sel_opt = 1;
			sel_color = 32;
			break;

		case UXMI_LCYAN:
			sel_opt = 1;
			sel_color = 36;
			break;

		case UXMI_LRED:
			sel_opt = 1;
			sel_color = 31;
			break;

		case UXMI_LPURPLE:
			sel_opt = 1;
			sel_color = 35;
			break;

		case UXMI_YELLOW:
			sel_opt = 1;
			sel_color = 33;
			break;

		default:
			sel_opt = 1;
			sel_color = 37;
			break;

	}

	printf("\033[%d;%dm", sel_opt, sel_color);
	return;
}

int
uxmi_system(char *xcmd)
{
	int rc = 0;
	char scmd[255];

	sprintf(scmd,"%s 2>/dev/null 1>/dev/null",xcmd);
#if !defined(UXMI_DEBUG_NOEXEC)
	rc = system(scmd);
#else /* !defined(UXMI_DEBUG_NOEXEC) */
#if defined(UXMI_DEBUG_OUTPUT)
	printf("\n%s\n",scmd);
#endif /* defined(UXMI_DEBUG_OUTPUT) */
#endif /* !defined(UXMI_DEBUG_NOEXEC) */

	return(rc);
}
