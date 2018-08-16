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

/*
 * Function: abms_strlcpy
 * Purpose : secure string copy
 * 
 * Passed  : char *dst, const char *src, size
 * Returns : size_t length
 *
 * Impact  : copies string from src to dst
 *
 */
size_t 
abms_strlcpy(char *dst, const char *src, size_t size) {
        char *dstptr = dst;
        size_t tocopy = size;
        const char *srcptr = src;

        if (tocopy && --tocopy) {
            do {
                if (!(*dstptr++ = *srcptr++)) break;
            } while (--tocopy);
        }

        if (!tocopy) {
            if (size) *dstptr = 0;
            while (*srcptr++);
        }

        return (srcptr - src - 1);
}

/*
 * Function: abms_strlcat
 * Purpose : secure string cat
 * 
 * Passed  : char *dst, const char *src, size
 * Returns : size_t length
 *
 * Impact  : cats string from src to end of dst
 *
 */
size_t
abms_strlcat(char *dst, const char *src, size_t size)
{
        char *dstptr = dst;
        size_t dstlen, tocopy = size;
        const char *srcptr = src;

        while (tocopy-- && *dstptr) dstptr++;
        dstlen = dstptr - dst;
        if (!(tocopy = size - dstlen)) return (dstlen + strlen(src));
        while (*srcptr) {
            if (tocopy != 1) {
                *dstptr++ = *srcptr;
                tocopy--;
            }
            srcptr++;
        }
        *dstptr = 0;

        return (dstlen + (srcptr - src));
}

/*
 * Function: abms_system
 * Purpose : system cmd wrapper
 * 
 * Passed  : char *cmd
 * Returns : u8 result code
 *
 * Impact  : just calls system, in debug mode, just prints the cmd
 *
 */
u8
abms_system(char *cmd)
{
        u8 rc;
#if defined(ABMS_DEBUG)
        char foo_debug[255];
#endif /* defined(ABMS_DEBUG) */
        char *lcmd;
        int slen;

#if defined(ABMS_SUPPRESS_OUTPUT)
        slen = strlen(cmd) + strlen(LCMD_SUPPRESS) + 2;
        lcmd = (char *) malloc(slen);
        abms_strlcpy(lcmd, cmd, strlen(cmd) + 1);
        abms_strlcat(lcmd, LCMD_SUPPRESS, slen);
#else /* defined(ABMS_SUPPRESS_OUTPUT) */
        slen = strlen(cmd) + 1;
        lcmd = (char *) malloc(slen);
        abms_strlcpy(lcmd, cmd, strlen(cmd)+1);        
#endif /* defined(ABMS_SUPPRESS_OUTPUT) */

#if defined(ABMS_DEBUG)
        sprintf(foo_debug,"[*] %s",lcmd);
        printf("%s\n", foo_debug);
#endif /* defined(ABMS_DEBUG) */

        rc = abms_exec(lcmd);

        free(lcmd);
        return(rc);
}

/*
 * Function: abms_exec
 * Purpose : executes a command with a different shell location
 * 
 * Passed  : const char *cmd
 * Returns : u8
 *
 * Impact  : forks and executes a command, does system without needing /bin/sh
 *
 */
u8
abms_exec(const char *cmd)
{
	int status;
	int fpid;
	char pcmd[256];
	char *pargv[4];

	pargv[0] = ABMS_SHELL;
        pargv[1] = "-c";

	sprintf(pcmd,"%s",cmd);

	pargv[2] = pcmd;
        pargv[3] = NULL;

	fpid = fork();

#if defined(ABMS_DEBUG_VERBOSE)
	printf("fexec(%d): %s -c %s %c\n",getpid(),ABMS_SHELL,pcmd, 0);
#endif /* defined(ABMS_DEBUG_VERBOSE) */

	if (abms_cfg.debug)
	    sleep(5);


	if (fpid < 0) {
	    printf("\n\n*** DEBUG *** Fork failed %s\n\n",pcmd);
	    return RC_FAIL;
	}

	if (fpid == 0) {
#if !defined(ABMS_DEBUG)
	    if (abms_cfg.debug) {
	        printf("exec(%d): %s -c %s %c\n",getpid(),ABMS_SHELL,pcmd, 0);
	    }
	    execve(ABMS_SHELL,pargv,NULL);
#else /* !defined(ABMS_DEBUG) */
	    printf("exec(%d): %s -c %s %c\n",getpid(),ABMS_SHELL,pcmd, 0);
#endif /* !defined(ABMS_DEBUG) */
	    exit(1);
	}

	while (wait(&status) != fpid);

	return RC_OK;		/* we should probably handle errors here */
}
