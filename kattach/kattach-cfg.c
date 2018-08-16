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
 * Source File :		kattach-cfg.c    
 * Purpose     :      		Read configuration
 * Callers     :                kattach.c
 *
 */

#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "kattach_types.h"
#include "kattach.h"

void
kattach_kcmdline(void)
{
	u8 rc = 0;

	rc = kattach_parse_cmdline();

	if (rc == RC_FAIL) {
		/* no command line configuration found, run in unconfigured mode */
		kattach_cfg.mode = KATTACH_MODE_NOTCFG;
		kattach_setup = 0;
	} else {
		/* command line was found and mode is already set */
		switch (kattach_cfg.mode) {

			case KATTACH_MODE_UNKNOWN:
				kattach_setup = 0;
				break;

			default:
				kattach_setup = 1;
				break;
		}
	}
	return;
}

u8
kattach_parse_cmdline(void)
{
        char ftmp[255], *fcmd, *buf, *ipstr, c = '\0', lc = '\0';
        FILE *stream;
        u8 i = 0, j = 0, skip = 0, found = 0, match = 0, mac = 0, macc = 0, cc = 0, parsed = 0;

	/* When in debug mode use ./cmdline for testing */
#if !defined(KATTACH_DEBUG)
        sprintf(ftmp,"/proc/cmdline");
#else /* !defined(KATTACH_DEBUG) */
        sprintf(ftmp,"./cmdline");
#endif /* !defined(KATTACH_DEBUG) */

        fcmd = (char *) malloc(sizeof(ftmp));

        kattach_strlcpy(fcmd, ftmp, sizeof(ftmp));

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
                    		if (!j) continue;
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
                            			kattach_cfg.mode = 0;
                            			i++;
                            			continue;
                        		} else {
                            			if (c == '0') {
                                			kattach_cfg.mode = KATTACH_MODE_NOTCFG;
							kattach_bootmode = KATTACH_MODE_NOTCFG;
                            			} else if (c == '1') {
                                			kattach_cfg.mode = KATTACH_MODE_KAOS;
							kattach_bootmode = KATTACH_MODE_KAOS;
                            			} else if (c == '2') {
                                			kattach_cfg.mode = KATTACH_MODE_KAOS;
							kattach_bootmode = KATTACH_MODE_KAOS;
                            			} else if (c == '8') {
                                			kattach_cfg.mode = KATTACH_MODE_KAOS;
							kattach_bootmode = KATTACH_MODE_RECOVERY;
							kattach_recovery = 1;
                            			} else if (c == '9') {
                                			kattach_cfg.mode = KATTACH_MODE_VKAOS;
							kattach_bootmode = KATTACH_MODE_VKAOS;
                            			}

                            			i++;
                        		}
                        		break;

		   		case 1:
					/* partition */
					if (skip) {
						skip = 0;
						sprintf(kattach_cfg.storedev,"%s","sda3");
						i++;
						continue;
					}
					if (c == ':') {
						buf[j] = '\0';
                            			strcpy(kattach_cfg.storedev,buf);
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


                   		case 2:
                        		/* mac address */
                        		if (skip) {
                            			u8 foo;
                            			skip = 0;
                            			for (foo = 0; foo < 6; foo++) {
                                			kattach_cfg.mac[foo] = 0;
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
                            			sscanf(buf,"%x",&kattach_cfg.mac[macc]);
                            			macc++;
                        		}
                        		if (macc != 6) {
                           			j++;
                        		} else {
                           			j = 0;
                           			i++;
                        		}
                        		break;

                    		case 3:
                        		/* ip address, if net = 1, but ip = 0, and mac = val, use dhcp */
                        		if (skip) {
                            			skip = 0;
                            			kattach_cfg.ip = 0;
			    			kattach_cfg.dhcp = 1;
                            			i++;
                            			continue;
                        		}
                        		if ((c != ':') && (c != ' ') && (c != '\0')) {
                            			buf[j] = c;
                            			j++;
                        		} else {
                            			buf[j] = '\0';
                            			j = 0;
                            			i++;
                            			kattach_cfg.ip = (u32) inet_network(buf);
                            			free(buf);
                            			buf = (char *) malloc(sizeof(ftmp));
                            			buf[0] = '\0';
						if (((kattach_cfg.ip >> 24) & 0xff) == 0) {
							kattach_cfg.slash = 0;
							kattach_cfg.gw = 0;
							kattach_cfg.dns = 0;
							kattach_cfg.dhcp = 1;
							i++;
							found = 0;
							match = 0;
							parsed = 1;
							continue;
						} else {
							kattach_cfg.dhcp = 0;
						}
                            			continue;
                        		}
                        		break;

                    		case 4:
                        		/* slash notation for mask */
                        		if (skip) {
                            			skip = 0;
                            			kattach_cfg.slash = 0;
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
                            			kattach_cfg.slash = (unsigned long) atoi(buf);
                            			free(buf);
                            			buf = (char *) malloc(sizeof(ftmp));
                            			buf[0] = '\0';
                        		}
                        		break;

                    		case 5:
                        		/* gw address */
                        		if (skip) {
                            			skip = 0;
                            			kattach_cfg.gw = 0;
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
                            			kattach_cfg.gw = (u32) inet_network(buf);
                            			free(buf);
                            			buf = (char *) malloc(sizeof(ftmp));
                            			buf[0] = '\0';
                            			continue;
                        		}
                        		break;

                    		case 6:
                        		/* dns address */
                        		if (skip) {
                            			skip = 0;
                            			kattach_cfg.dns = 0;
                            			i++;
                            			continue;
                        		}
                        		if ((c != ':') && (c != ' ') && (c != '\0')) {
                            			buf[j] = c;
                            			j++;
                        		} else {
                            			buf[j] = '\0';
                            			j = 0;
                            			i++;
                            			kattach_cfg.dns = (u32) inet_network(buf);
                            			free(buf);
                            			buf = (char *) malloc(sizeof(ftmp));
                            			buf[0] = '\0';
						i++;
						found = 0;
						match = 0;
						parsed = 1;
						continue;
					}
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

		/* Look for the kaos keyword in the cmdline */
            	switch(c) {
                	case 'k':
                	    	if ((lc == '\0') || (lc == ' ')) {
                        		match = 1;
                    		} else {
                        		match = 0;
                    		}
                    		break;

                	case 'a':
                    		if ((lc == 'k') && (match == 1)) {
                        		match = 2;
                    		} else {
                        		match = 0;
                    		}
                    		break;

                	case 'o':
                    		if ((lc == 'a') && (match == 2)) {
                        		match = 3;
                    		} else {
                        		match = 0;
                    		}
                    		break;

                	case 's':
                    		if ((lc == 'o') && (match == 3)) {
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

#if defined(KATTACH_DEBUG)
        printf("\nResults: \n\n");

        printf("mode      = %u\n", kattach_cfg.mode);
	printf("storage   = %s\n", kattach_cfg.storedev);
        printf("ip        = %lu.%lu.%lu.%lu\t %llx\n",(u16) ((kattach_cfg.ip >> 24) & 0xff), (u16) ((kattach_cfg.ip >> 16) & 0xff), (u16) ((kattach_cfg.ip >> 8) & 0xff), (u16) ((kattach_cfg.ip) & 0xff), kattach_cfg.ip);
        printf("slash     = %lu\n",kattach_cfg.slash);
        printf("gw        = %lu.%lu.%lu.%lu\t %llx\n",(u16) ((kattach_cfg.gw >> 24) & 0xff), (u16) ((kattach_cfg.gw >> 16) & 0xff), (u16) ((kattach_cfg.gw >> 8) & 0xff), (u16) ((kattach_cfg.gw) & 0xff), kattach_cfg.gw);
        printf("dns       = %lu.%lu.%lu.%lu\t %llx\n",(u16) ((kattach_cfg.dns >> 24) & 0xff), (u16) ((kattach_cfg.dns >> 16) & 0xff), (u16) ((kattach_cfg.dns >> 8) & 0xff), (u16) ((kattach_cfg.dns) & 0xff), kattach_cfg.dns);
        printf("mac       = %02x:%02x:%02x:%02x:%02x:%02x\n",kattach_cfg.mac[0],kattach_cfg.mac[1],kattach_cfg.mac[2],kattach_cfg.mac[3],kattach_cfg.mac[4],kattach_cfg.mac[5]);
#endif /* defined(KATTACH_DEBUG) */

        if (parsed) {
            return(RC_OK);
        } else {
            return(RC_FAIL);
        }
}
