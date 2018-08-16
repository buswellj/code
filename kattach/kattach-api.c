/*
 * kattach (kernel attach)
 * Copyright (c) 2009 - 2010 Carbon Mountain LLC.
 * All Rights Reserved.
 *
 * John Buswell <buswellj@carbonmountain.com>
 * version 0.6.0
 *
 * LICENSE: GPL v2
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
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
 *
 * To contact Carbon Mountain LLC please visit http://www.carbonmountain.com
 *
 * Source File :                kattach-api.c
 * Purpose     :                reusable code for kattach
 * Callers     :                multiple
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
#include "kattach_types.h"
#include "kattach.h"

/*
 * function: kattach_strlcat
 * purpose : secure string merge
 *
 * passed  : char *dst, const char *src, size
 * returns : size_t length
 *
 * result  : adds string src to the end of dst
 *
 */
size_t
kattach_strlcat(char *dst, const char *src, size_t size)
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
 * function: kattach_strlcpy
 * purpose : secure string copy
 *
 * passed  : char *dst, const char *src, size
 * returns : size_t length
 *
 * result  : copies string from src to dst
 *
 */
size_t
kattach_strlcpy(char *dst, const char *src, size_t size) 
{
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
 * function: kattach_sysexec
 * purpose : executes system commands
 *
 * passed  : char *cmd
 * returns : u8 result code
 *
 * result  : executes system command, displays debug information
 *
 */
u8
kattach_sysexec(char *cmd)
{
        u8 rc;					/* result code */
        char *lcmd;				/* linux command */
        int slen;				/* string length */
#if defined(KATTACH_DEBUG)
        char kattach_debug[255];
#endif /* defined(KATTACH_DEBUG) */

        slen = strlen(cmd) + 1;
        lcmd = (char *) malloc(slen);
        kattach_strlcpy(lcmd, cmd, strlen(cmd)+1);        

#if defined(KATTACH_DEBUG)
        sprintf(kattach_debug," [*] %s",lcmd);
        printf("%s\n", kattach_debug);
#endif /* defined(KATTACH_DEBUG) */

        rc = kattach_exec(lcmd);

        free(lcmd);
        return(rc);
}

/*
 * function: kattach_exec
 * purpose : executes system command with custom shell
 *
 * passed  : const char *cmd
 * returns : u8 result code
 * 
 * result  : forks and executes a command without needing /bin/sh
 * 
 */
u8
kattach_exec(char *cmd)
{
        int status;				/* child status */
        int fpid;				/* forked process id */
        char pcmd[256];				/* process command */
        char *pargv[4];				/* process argv */

        pargv[0] = KATTACH_SHELL;		/* set the shell */
        pargv[1] = "-c";			/* set the command option */

        sprintf(pcmd,"%s",cmd);

        pargv[2] = pcmd;
        pargv[3] = NULL;

        fpid = fork();				/* fork the command */

#if defined(KATTACH_DEBUG_VERBOSE)
        printf("fexec(%d): %s -c %s %c\n",getpid(),KATTACH_SHELL,pcmd, 0);
#endif /* defined(KATTACH_DEBUG_VERBOSE) */

        if (fpid < 0) {
            printf("\n\n*** DEBUG *** Fork failed %s\n\n",pcmd);
            return RC_FAIL;
        }

        if (fpid == 0) {
#if !defined(KATTACH_DEBUG)
            execve(KATTACH_SHELL,pargv,NULL);
#else /* !defined(KATTACH_DEBUG) */
            printf("exec(%d): %s -c %s %c\n",getpid(),KATTACH_SHELL,pcmd, 0);
#endif /* !defined(KATTACH_DEBUG) */
            exit(1);
        }

        while (wait(&status) != fpid);

        return RC_OK;           /* we should probably handle errors here */

}

/*
 * function: kattach_bkexec
 * purpose : executes system commands in the background
 *
 * passed  : char *cmd
 * returns : int pid
 *
 * result  : executes system command, displays debug information
 *
 */
int
kattach_bkexec(char *cmd, char *kargv)
{
        int rc;					/* result code */
        char *lcmd;				/* linux command */
	char *largv;				/* argv */
        int slen;				/* string length */

        slen = strlen(cmd) + 1;
        lcmd = (char *) malloc(slen);
	slen = strlen(kargv) + 1;
	largv = (char *) malloc(slen);
        kattach_strlcpy(lcmd, cmd, strlen(cmd)+1);
	kattach_strlcpy(largv, kargv, strlen(kargv)+1);

        rc = kattach_execbk(lcmd, largv);

        free(lcmd);
	free(largv);
        return(rc);
}

/*
 * function: kattach_execbk
 * purpose : executes system command with custom shell in background
 *
 * passed  : const char *cmd
 * returns : process id
 * 
 * result  : forks and executes a command without needing /bin/sh
 * 
 */
int
kattach_execbk(char *cmd, char *kargv)
{
        int fpid;				/* forked process id */
	char *pargv[128];
	char *buf;
	char c = '\0';
	int i = 0, x = 0, n = 1;

        fpid = fork();				/* fork the command */

        if (fpid < 0) {
            printf("\n\n*** DEBUG *** Fork failed %s %s\n\n",cmd,kargv);
            return RC_FAIL;
        }

        if (fpid == 0) {
		pargv[0] = cmd;
		buf = (char *) malloc(strlen(kargv)+1);
		for (i = 0; i <= strlen(kargv); i++) {
			c = kargv[i];
			if ((c != ' ') && (c != '\0')) {
				buf[x] = c;
				x++;
			} else {
				if (x == 0) continue;
				buf[x] = '\0';
				pargv[n] = (char *) malloc(strlen(buf)+1);
				sprintf(pargv[n],"%s",buf);
				n++;
				x = 0;
				memset(buf,0,sizeof(buf));
			}
		}
		free(buf);
		pargv[n] = NULL;
		x = execve(cmd,pargv,NULL);
		printf("\n\n*** DEBUG *** Exec failed %s %s\n\n",cmd,kargv);
		for (i = 0; i <= n; i++) {
			printf(" arg %u is %s\n",i,pargv[i]);
		}
		exit(1);
        }
        return fpid;
}

/*
 * function: kattach_hash
 * purpose : dbj2 hash algorithm to hash a string
 *
 * passed  : unsigned char *str
 * returns : unsigned long (hash)
 * 
 * result  : generates a hash from a string
 * 
 */
unsigned long 
kattach_hash(char *str)
{
        unsigned long hash = 5381;
        int c;

        while ((c = *str++))
                hash = ((hash << 5) + hash) + c;

        return hash;
}


/*
 * function: kattach_getbuuid
 * purpose : grabs the linux kernel boot_id UUID
 *
 * passed  : void
 * returns : void
 * 
 * result  : stores the boot_id in kattach_buuid[]
 * 
 */
void
kattach_getbuuid(void)
{
        FILE *stream;
        char *buf;
        int j = 0;
        char c = '\0';

        stream = fopen(KATTACH_PROC_BOOTID,"r");
        buf = (char *) malloc(sizeof(kattach_buuid));

        while (!feof(stream)) {
                c = (char) fgetc(stream);
                buf[j] = c;
                j++;
        }
	buf[j] = '\0';

	memset(kattach_buuid,0,sizeof(kattach_buuid));
        strncpy(kattach_buuid,buf,strlen(buf));
        fclose(stream);
        free(buf);
        return;

}

/*
 * function: kattach_getruuid
 * purpose : grabs the linux kernel random UUID
 *
 * passed  : void
 * returns : void
 * 
 * result  : stores the random UUID in kattach_ruuid[]
 * 
 */
void
kattach_getruuid(void)
{
        FILE *stream;
        char *buf;
        int j = 0;
        char c = '\0';

        stream = fopen(KATTACH_PROC_UUID,"r");
        buf = (char *) malloc(sizeof(kattach_ruuid));

        while (!feof(stream)) {
                c = (char) fgetc(stream);
                buf[j] = c;
                j++;
        }
	buf[j] = '\0';

	memset(kattach_ruuid,0,sizeof(kattach_ruuid));
        strncpy(kattach_ruuid,buf,strlen(buf));
	kattach_ruuid[36] = '\0';
        fclose(stream);
        free(buf);
        return;
}

