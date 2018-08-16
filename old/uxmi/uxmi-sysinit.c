/*
 * uxmi-sysinit.c
 *
 * UxMI sysinit processing
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
#include "uxmi.h"

void uxmi_sysinit(void);
void uxmi_sysinit_disp(char *conlog);
void uxmi_sysinit_testrc(int rc);

void
uxmi_sysinit()
{
	char xcmd[254];
	int rc = 0;

	/* First we mount /proc and /sys */

	uxmi_color_fg(UXMI_LBLUE);
	printf("\nStarting System Initialization....\n\n");

	uxmi_sysinit_disp("\t Mounting /proc filesystem");
	sprintf(xcmd,"%s -n -t proc none /proc ",UXMI_CMD_MOUNT);
	rc = uxmi_system(xcmd);
	uxmi_sysinit_testrc(rc);

	uxmi_sysinit_disp("\t Mounting /sys filesystem");
	sprintf(xcmd,"%s -n -t sysfs none /sys ",UXMI_CMD_MOUNT);
	rc = uxmi_system(xcmd);
	uxmi_sysinit_testrc(rc);

	/* TODO: Add in support for udev here */

	/* Now we read the UXMI_CONFIG file, typically this is stored
	 * in /etc/AppOS/uxmi.cfg.
         */

	uxmi_color_fg(UXMI_LBLUE);
	printf("\nProcessing System Configuration....\n\n");

	sprintf(xcmd,"\t Configuration file is %s ",UXMI_CONFIG);
	uxmi_sysinit_disp(xcmd);
	rc = uxmi_do_cfg();

	uxmi_color_reset();
}

void
uxmi_sysinit_disp(char *conlog)
{
	uxmi_color_fg(UXMI_CYAN);
	printf("%s",conlog);
	return;
}

void
uxmi_sysinit_testrc(int rc)
{
	if (rc) {
	    uxmi_color_fg(UXMI_LRED);
	    printf("\t\t\t FAILED \n");
	    /* handle this */
	} else {
	    uxmi_color_fg(UXMI_LGREEN);
	    printf("\t\t\t OK \n");
	}
	return;
}
