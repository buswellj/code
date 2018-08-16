/*
 * AppOS Boot Management System (ABMS)
 * Copyright (c) 2002-2008 Spliced Networks LLC.
 * All Rights Reserved.
 *
 * Author: John Buswell <buswellj@splicednetworks.com>
 * Release: 4.0.0
 *
 * LICENSE: GPL v3
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 3
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 * To contact Spliced Networks LLC:
 * 
 * Spliced Networks LLC
 * 4820 Fisher Road
 * Athens, OH 45701
 * USA
 * 
 * Tel: (408) 416-3832
 * email: support@splicednetworks.com
 *
 * http://www.splicednetworks.com
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <linux/unistd.h>
#include <ctype.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <unistd.h>
#include "abms.h"

void abms_loop_getty(void)
{
	char fcmd[255];
	int fpid;
	char *pargv[4];

	pargv[0] = ABMS_SHELL;
	pargv[1] = "-c";

	sprintf(fcmd,"%s%s -l %s%s -f %s 38400 %s",ABMS_BINPATH,ACMD_GETTY,ABMS_BINPATH,ACMD_LOGIN,ABMS_ISSUENET,ABMS_SECTTY);

	pargv[2] = fcmd;
	pargv[3] = NULL;

	fpid = fork();

	if (fpid == 0) {
	    while(1) {
#if !defined(ABMS_DEBUG)
		execve(ABMS_SHELL,pargv,NULL);
#else /* !defined(ABMS_DEBUG) */
		printf("getty exec \n");
		sleep(60);
#endif /* !defined(ABMS_DEBUG) */
	    }
	    exit(1);
	}

	abms_gettypid = fpid;

	return;
}

