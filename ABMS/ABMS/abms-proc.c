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
#include <fcntl.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "abms.h"

void abms_cmdline(void)
{
	int rc = 0;

	rc = abms_parse_cmdline();

	if (rc == RC_FAIL) {
	    /* no command line was found, so we set mode to unconfigured */
	    abms_cfg.mode = 0xff;
	}
}

u32 abms_parse_cmdline(void)
{
	char ftmp[255], *fcmd, *buf, *ipstr, c = '\0', lc = '\0';
        FILE *stream;
        u8 i = 0, j = 0, skip = 0, found = 0, match = 0, mac = 0, macc = 0, cc = 0, parsed = 0;

#if !defined(ABMS_DEBUG)
        sprintf(ftmp,"/proc/cmdline");
#else /* !defined(ABMS_DEBUG) */
        sprintf(ftmp,"./cmdline");
#endif /* !defined(ABMS_DEBUG) */

	fcmd = (char *) malloc(sizeof(ftmp));

	abms_strlcpy(fcmd, ftmp, sizeof(ftmp));

	ipstr = (char *) malloc(sizeof(ftmp));
        buf = (char *) malloc(sizeof(ftmp));
        stream = fopen(fcmd,"r");

	if (stream == (FILE *)0) {
            free(buf);
            free(fcmd);
            return(RC_FAIL);
        }

        while (!feof(stream)) {
            cc++;
            lc = c;
            c = (char) fgetc(stream);
            if ((found) && ((c != ' ') || (c != '\n') || (c != '\0'))) {
                if ((c == ':') && (lc != ':')) {
                    if (!j) {
                        continue;
                    }
                } else if ((c == ':') && (lc == ':')) {
                    skip = 1;
                }
                /* This switch statement does all the processing. 
                 * If the option is skipped we default to factory
                 * settings, otherwise we process it.
                 */
                switch(i) {
                    case 0:
                        /* first cmdline option is mode */
                        if ((skip) || ((c != '0') && (c != '1') && (c != '2'))) {
                            skip = 0;
                            abms_cfg.mode = 0;
                            i++;
                            continue;
                        } else {
                            if (c == '0') {
                                abms_cfg.mode = 0;
                            } else if (c == '1') {
                                abms_cfg.mode = 1;
                            } else if (c == '2') {
                                abms_cfg.mode = 2;
                            }
                            i++;
                        }
                        break;

		    case 1:
                        /* ip address, if net = 1, but ip = 0, and mac = val, use dhcp */
                        if (skip) {
			    skip = 0;
                            abms_cfg.ip = 0;
                            i++;
                            continue;
                        }
                        if (c != ':') {
                            buf[j] = c;
                            j++;
                        } else {
			    buf[j] = '\0';
                            j = 0;
                            i++;
                            abms_cfg.ip = (u32) inet_network(buf);
			    free(buf);
			    buf = (char *) malloc(sizeof(ftmp));
			    buf[0] = '\0';
                            continue;
                        }
                        break;

		    case 2:
                        /* slash notation for mask */
                        if (skip) {
			    skip = 0;
                            abms_cfg.slash = 0;
                            i++;
                            continue;
                        }
                        if (c != ':') {
                            buf[j] = c;
                            j++;
                        } else {
                            buf[j] = '\0';
                            j = 0;
                            i++;
                            abms_cfg.slash = (unsigned long) atoi(buf);
			    free(buf);
			    buf = (char *) malloc(sizeof(ftmp));
                            buf[0] = '\0';
                        }
                        break;

		    case 3:
                        /* gw address */
                        if (skip) {
			    skip = 0;
                            abms_cfg.gw = 0;
                            i++;
                            continue;
                        }
                        if (c != ':') {
                            buf[j] = c;
                            j++;
                        } else {
			    buf[j] = '\0';
                            j = 0;
                            i++;
                            abms_cfg.gw = (u32) inet_network(buf);
			    free(buf);
			    buf = (char *) malloc(sizeof(ftmp));
                            buf[0] = '\0';
                            continue;
                        }
                        break;

		    case 4:
                        /* dns address */
                        if (skip) {
			    skip = 0;
                            abms_cfg.dns = 0;
                            i++;
                            continue;
                        }
                        if (c != ':') {
                            buf[j] = c;
                            j++;
                        } else {
			    buf[j] = '\0';
                            j = 0;
                            i++;
                            abms_cfg.dns = (u32) inet_network(buf);
			    free(buf);
			    buf = (char *) malloc(sizeof(ftmp));
                            buf[0] = '\0';
                            continue;
                        }
                        break;

		    case 5:
                        /* ip abacus address */
                        if (skip) {
			    skip = 0;
                            abms_cfg.ipab = 0;
                            i++;
                            continue;
                        }
                        if (c != ':') {
                            buf[j] = c;
                            j++;
                        } else {
			    buf[j] = '\0';
                            j = 0;
                            i++;
                            abms_cfg.ipab = (u32) inet_network(buf);
			    free(buf);
			    buf = (char *) malloc(sizeof(ftmp));
                            buf[0] = '\0';
                            continue;
                        }
                        break;

                   case 6:
                        /* mac address */
                        if (skip) {
                            u8 foo;
                            skip = 0;
                            for (foo = 0; foo < 6; foo++) {
                                abms_cfg.mac[foo] = 0;
                            }
                            i++;
                            continue;
                        }
                        if (mac == 0) {
                            mac++;
                            continue;
                        } else if (mac == 1) {
                            mac = 0;
                            sprintf(buf,"%c%c",tolower(lc),tolower(c));
                            sscanf(buf,"%x",&abms_cfg.mac[macc]);
                            macc++;
                        }
                        if (macc != 6) {
                           j++;
                        } else {
                           j = 0;
                           i++;
                        }
                        break;
                            
                    case 7:
                        /* flashdev */
                        if (skip) {
                            skip = 0;
                            abms_cfg.flashdev = "hda2";
                            i++;
                            continue;
                        }
                        if (c == ':') {
                            buf[j] = '\0';
                            abms_cfg.flashdev = (char *) malloc(sizeof(ftmp));
                            strcpy(abms_cfg.flashdev,buf);
                            j = 0;
			    free(buf);
			    buf = (char *) malloc(sizeof(ftmp));
                            buf[0] = '\0';
                            i++;
                        } else {
                            buf[j] = tolower(c);
                            j++;
                        }
                        break;

                   case 8:
                        /* image slot */
                        if (skip) {
                            skip = 0;
                            abms_cfg.slot = 1;
                            i++;
                            continue;
                        }
                        sprintf(buf,"%c",c);
                        abms_cfg.slot = (u8) atoi(buf);
                        i++;
			found = 0;
			match = 0;
			parsed = 1;
                        break;

                   default:
                        printf(" ** Unknown Option in cmdline ** \n\n");
                        break;


                }
                continue;
            } else if (found) {
                break;
            }

            if ((!found) && ((c == '\n') || (c == '\0'))) {
                break;
            }

	    if ((!found) && (parsed)) {
		break;
	    }

            switch(c) {
                case 'a':
                    if ((lc == '\0') || (lc == ' ')) {
                        match = 1;
                    } else {
                        match = 0;
                    }
                    break;

                case 'b':
                    if ((lc == 'a') && (match == 1)) {
                        match = 2;
                    } else {
                        match = 0;
                    }
                    break;

                case 'm':
                    if ((lc == 'b') && (match == 2)) {
                        match = 3;
                    } else {
                        match = 0;
                    }
                    break;

                case 's':
                    if ((lc == 'm') && (match == 3)) {
                        match = 4;
                    } else {
                        match = 0;
                    }
                    break;

                case '=':
                    if ((lc == 's') && (match == 4)) {
                        match = 5;
                        found = 1;
                    }
                    break;

                default:
                    match = 0;
                    found = 0;
                    break;

            }
            lc = c;
        }
        free(ipstr);
        free(buf);
        free(fcmd);
        fclose(stream);

#if defined(ABMS_DEBUG)
	printf("\nResults: \n\n");

	printf("mode      = %u\n", abms_cfg.mode);
	printf("ip        = %lu.%lu.%lu.%lu\t %llx\n",(u16) ((abms_cfg.ip >> 24) & 0xff), (u16) ((abms_cfg.ip >> 16) & 0xff), (u16) ((abms_cfg.ip >> 8) & 0xff), (u16) ((abms_cfg.ip) & 0xff), abms_cfg.ip);
	printf("slash     = %lu\n",abms_cfg.slash);
	printf("gw        = %lu.%lu.%lu.%lu\t %llx\n",(u16) ((abms_cfg.gw >> 24) & 0xff), (u16) ((abms_cfg.gw >> 16) & 0xff), (u16) ((abms_cfg.gw >> 8) & 0xff), (u16) ((abms_cfg.gw) & 0xff), abms_cfg.gw);
	printf("dns       = %lu.%lu.%lu.%lu\t %llx\n",(u16) ((abms_cfg.dns >> 24) & 0xff), (u16) ((abms_cfg.dns >> 16) & 0xff), (u16) ((abms_cfg.dns >> 8) & 0xff), (u16) ((abms_cfg.dns) & 0xff), abms_cfg.dns);
	printf("ipab      = %lu.%lu.%lu.%lu\t %llx\n",(u16) ((abms_cfg.ipab >> 24) & 0xff), (u16) ((abms_cfg.ipab >> 16) & 0xff), (u16) ((abms_cfg.ipab >> 8) & 0xff), (u16) ((abms_cfg.ipab) & 0xff), abms_cfg.ipab);
	printf("mac       = %02x:%02x:%02x:%02x:%02x:%02x\n",abms_cfg.mac[0],abms_cfg.mac[1],abms_cfg.mac[2],abms_cfg.mac[3],abms_cfg.mac[4],abms_cfg.mac[5]);
	printf("flash     = /dev/%s\n",abms_cfg.flashdev);
	printf("slot      = %u\n\n\n",abms_cfg.slot);
#endif /* defined(ABMS_DEBUG) */

	if (parsed) {
            return(RC_OK);
        } else {
            return(RC_FAIL);
        }
}
