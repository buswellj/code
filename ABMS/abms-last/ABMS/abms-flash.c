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
#include "abms-map.h"

extern abms_loop_t abms_loop_table;
extern abms_sdt_t abms_app_table;
extern abms_dmt_t abms_storage_table;

extern abms_prodip_t abms_prodip_table;
extern abms_mgmt_t abms_mnet_table;


void abms_flash_init(void)
{
	char lcmd[255];
	char ftmp[255], *fcmd, *buf, c = '\0', lc = '\0';
	FILE *stream;
	u8 index = 0, j = 0, found = 0, cc = 0, parsed = 0, pentry = 0, tentry = 0, tparsed = 0, devcnt = 0, match = 0, mac = 0, macc = 0;

	sprintf(lcmd,"/dev/%s",abms_cfg.flashdev);
	abms_mkdir(ABMS_FLASHDST,1);
	abms_mountf(lcmd,ABMS_FLASHFS,ABMS_FLASHDST,1);

#if !defined(ABMS_DEBUG)
        sprintf(ftmp,"%s/%s/%u/%s",ABMS_FLASHDST,ABMS_FLASHSLOT,abms_cfg.slot,ABMS_FLASHCFG);
#else /* !defined(ABMS_DEBUG) */
        sprintf(ftmp,"./%s",ABMS_FLASHCFG);
#endif /* !defined(ABMS_DEBUG) */

	fcmd = (char *) malloc(sizeof(ftmp));
        abms_strlcpy(fcmd, ftmp, sizeof(ftmp));
	stream = fopen(fcmd,"r");

	if (stream == (FILE *)0) {
	    free(fcmd);
	    sqfs_gen();					/* default sqfs */
	    abms_deployed = 0xff;			/* not properly deployed */
	    return;
	} 

	abms_map_init();

	buf = (char *) malloc(sizeof(ftmp));

	/* read and process the config here -- this code makes more sense after beer! :) */

	while (!feof(stream)) {
	    cc++;
	    lc = c;
	    c = (char) fgetc(stream);

	    /* look for commands %storage or %stack */
	    if (c == '%') {
		found = 0;
		while ((!found) && (!feof(stream))) {
		    cc++;
		    lc = c;
		    c = (char) fgetc(stream);
		    switch (c) {
			case 'n':
			    if (!match) 
			        match = 1;
			    break;

			case 's':
			    match = 1;
			    break;

			case 't':
			    if (match == 2) {
				match = 3;
			    } else {
			        match = 2;
			    }
			    break;

			case 'o':
			    if (match == 2) {
				match = 3;
			    }
			    break;
			
			case 'a':
			    if (match == 2) {
				match = 3;
			    }
			    break;

			case 'r':
			    if (match == 3) {
				match = 4;
			    }
			    break;
	
			case 'c':
			    if (match == 3) {
				match = 4;
			    }
			    break;

			case 'e':
			    if (match == 1) {
			        match = 2;
			    }
			    if (match == 4) {
				match = 5;
			    }
			    break;

			case 'k':
			    if (match == 4) {
				match = 5;
			    }
			    break;

			case '=':
			    if (match == 5) {
				if (lc == 'e') found = 1;
				if (lc == 'k') found = 2;
			    } else if (match == 3) {
				if (lc == 't') found = 3;
			    }
			    break;

			default:
			    break;
		    }
		}
		if (found) match = 0;
		/* these are for debug remove */
#if 0
		if (found == 1) printf("\nFound Store\n");
		if (found == 2) printf("\nFound Stack\n");
#endif /* 0 */
	    } else if (c == '\n') {
		found = 0;		
	    } else if (found == 1) {
		index = abms_storage_table.nxt_idx;
		if (abms_storage_table.nxt_idx < ABMS_DMT_MAX)
		    abms_storage_table.nxt_idx++;
		pentry = 0;
		parsed = 0;
		if (c != ':') {
		    buf[j] = c;
		    j++;
		}
		while ((parsed == 0) && (!feof(stream))) {
		    lc = c;
		    c = (char) fgetc(stream);
		    switch(pentry) {
			case 0:
			    if (c != ':') {
			        buf[j] = c;
			        j++;
			    } else {
				buf[j] = '\0';
				j = 0;
				pentry++;
				sprintf(abms_storage_table.index[index].name,"%s",buf);
				free(buf);
				buf = (char *) malloc(sizeof(ftmp));
				buf[0] = '\0';
				continue;
			    }
			    break;

			case 1:
			    if (c != ':') {
			        buf[j] = c;
			        j++;
			    } else {
				buf[j] = '\0';
				j = 0;
				pentry++;
				if (!strncmp(buf,"md",2)) {
				    abms_storage_table.index[index].type = ABMS_STORAGE_MD;
				} else if (!strncmp(buf,"lvm",3)) {
				    abms_storage_table.index[index].type = ABMS_STORAGE_LVM;
				} else if (!strncmp(buf,"nfs",3)) {
				    abms_storage_table.index[index].type = ABMS_STORAGE_NFS;
				} else if (!strncmp(buf,"loop",4)) {
				    abms_storage_table.index[index].type = ABMS_STORAGE_LOOP;
				} else if (!strncmp(buf,"iscsi",5)) {
				    abms_storage_table.index[index].type = ABMS_STORAGE_ISCSI;
				} else if (!strncmp(buf,"local",5)) {
				    abms_storage_table.index[index].type = ABMS_STORAGE_LOCAL;
				}
				abms_storage_table.index[index].index = index;			/* XXX: bit of a hack for now */
				free(buf);
				buf = (char *) malloc(sizeof(ftmp));
				buf[0] = '\0';
				continue;
			    }
			    break;

			case 2:
			    /* depending on the storage type, we have different values to grab */
			    switch (abms_storage_table.index[index].type) {
				case ABMS_STORAGE_MD:
				    /* dev:num_dev:dev_list */
				    buf[j] = c;
				    j++;
				    while ((!tparsed) && (!feof(stream))) {
					lc = c;
					c = (char) fgetc(stream);
					switch (tentry) {
					    case 0:				/* md device name without path */
						if (c != ':') {
					            buf[j] = c;
					    	    j++;
						} else {
					    	    buf[j] = '\0';
					    	    j = 0;
						    tentry++;
						    sprintf(abms_storage_table.md[index].sdev,"%s",buf);
						    free(buf);
						    buf = (char *) malloc(sizeof(ftmp));
						    buf[0] = '\0';
						}
						break;

					    case 1:				/* number of devices */
						if (c != ':') {
						    buf[j] = c;
						    j++;
						} else {
					    	    buf[j] = '\0';
					    	    j = 0;
						    tentry++;
						    abms_storage_table.md[index].numdev = (u8) atoi(buf);
						    free(buf);
						    buf = (char *) malloc(sizeof(ftmp));
						    buf[0] = '\0';
						}
						break;

					    case 2:
						if (c != ':') {
						    buf[j] = c;
						    j++;
						} else {
					    	    buf[j] = '\0';
					    	    j = 0;
						    sprintf(abms_storage_table.md[index].devlist[devcnt].dev,"%s",buf);
						    free(buf);
						    buf = (char *) malloc(sizeof(ftmp));
						    buf[0] = '\0';
						    if (devcnt == (abms_storage_table.md[index].numdev - 1)) {
							tparsed = 1;
						    } else {
							devcnt++;
						    }
						}
						break;
						
					    default:
						break;	

					}
					
				    }
				    break;

				case ABMS_STORAGE_LVM:
				    /* vg:id:fs:read:write:execute:source */
				    buf[j] = c;
				    j++;
				    tparsed = 0;
				    tentry = 0;
				    while ((!tparsed) && (!feof(stream))) {
					lc = c;
					c = (char) fgetc(stream);
					switch (tentry) {
					    case 0:
						if (c != ':') {
					            buf[j] = c;
					    	    j++;
						} else {
					    	    buf[j] = '\0';
					    	    j = 0;
						    tentry++;
						    sprintf(abms_storage_table.lvm[index].vg,"%s",buf);
						    free(buf);
						    buf = (char *) malloc(sizeof(ftmp));
						    buf[0] = '\0';
						}
						break;

					    case 1:		
						if (c != ':') {
					            buf[j] = c;
					    	    j++;
						} else {
					    	    buf[j] = '\0';
					    	    j = 0;
						    tentry++;
						    sprintf(abms_storage_table.lvm[index].id,"%s",buf);
						    free(buf);
						    buf = (char *) malloc(sizeof(ftmp));
						    buf[0] = '\0';
						}
						break;

					    case 2:				/* fs type */
						if (c != ':') {
						    buf[j] = c;
						    j++;
						} else {
					    	    buf[j] = '\0';
					    	    j = 0;
						    tentry++;
						    if (!strncmp(buf,"nfs",3)) {
							abms_storage_table.index[index].fs = ABMS_FS_NFS;
						    } else if (!strncmp(buf,"xfs",3)) {
							abms_storage_table.index[index].fs = ABMS_FS_XFS;
						    } else if (!strncmp(buf,"gfs",3)) {
							abms_storage_table.index[index].fs = ABMS_FS_GFS;
						    } else if (!strncmp(buf,"jfs",3)) {
							abms_storage_table.index[index].fs = ABMS_FS_JFS;
						    } else if (!strncmp(buf,"ext2",4)) {
							abms_storage_table.index[index].fs = ABMS_FS_EXT2;
						    } else if (!strncmp(buf,"ext3",4)) {
							abms_storage_table.index[index].fs = ABMS_FS_EXT3;
						    } else if (!strncmp(buf,"sqfs",3)) {
							abms_storage_table.index[index].fs = ABMS_FS_SQFS;
						    } else {
							abms_storage_table.index[index].fs = ABMS_FS_UNKNOWN;
						    }
						    free(buf);
						    buf = (char *) malloc(sizeof(ftmp));
						    buf[0] = '\0';
						}
						break;

					    case 3:
						if (c == ':') continue;
						if (c == '1') {
						    abms_storage_table.local[index].pread = 1;
						} else {
						    abms_storage_table.local[index].pread = 0;
						}
						tentry++;
						break;

					    case 4:
						if (c == ':') continue;
						if (c == '1') {
						    abms_storage_table.local[index].pwrite = 1;
						} else {
						    abms_storage_table.local[index].pwrite = 0;
						}
						tentry++;
						break;

					    case 5:
						if (c == ':') continue;
						if (c == '1') {
						    abms_storage_table.local[index].pexec = 1;
						} else {
						    abms_storage_table.local[index].pexec = 0;
						}
						tentry++;
						break;

					    case 6:		
						if (c != ':') {
					            buf[j] = c;
					    	    j++;
						} else {
					    	    buf[j] = '\0';
					    	    j = 0;
						    tentry++;
						    sprintf(abms_storage_table.lvm[index].src,"%s",buf);
						    free(buf);
						    buf = (char *) malloc(sizeof(ftmp));
						    buf[0] = '\0';
						}
						tparsed = 1;
						break;
					}
				    }
				    break;

				case ABMS_STORAGE_NFS:
				    /* merge for 4.0.1.0 -- add sanity checking for network fs */
				    break;

				case ABMS_STORAGE_LOOP:
				    /* merge for 4.0.1.0 */
				    break;

				case ABMS_STORAGE_ISCSI:
				    /* merge for 4.0.1.0 */
				    break;

				case ABMS_STORAGE_LOCAL:
				    /* dev:fs:read:write:execute:atime:monitor */
				    buf[j] = c;
				    j++;
				    tparsed = 0;
				    tentry = 0;
				    while ((!tparsed) && (!feof(stream))) {
					lc = c;
					c = (char) fgetc(stream);
					switch (tentry) {
					    case 0:				/* local disk device name without path */
						if (c != ':') {
					            buf[j] = c;
					    	    j++;
						} else {
					    	    buf[j] = '\0';
					    	    j = 0;
						    tentry++;
						    sprintf(abms_storage_table.local[index].sdev,"%s",buf);
						    free(buf);
						    buf = (char *) malloc(sizeof(ftmp));
						    buf[0] = '\0';
						}
						break;

					    case 1:				/* fs type */
						if (c != ':') {
						    buf[j] = c;
						    j++;
						} else {
					    	    buf[j] = '\0';
					    	    j = 0;
						    tentry++;
						    if (!strncmp(buf,"nfs",3)) {
							abms_storage_table.index[index].fs = ABMS_FS_NFS;
						    } else if (!strncmp(buf,"xfs",3)) {
							abms_storage_table.index[index].fs = ABMS_FS_XFS;
						    } else if (!strncmp(buf,"gfs",3)) {
							abms_storage_table.index[index].fs = ABMS_FS_GFS;
						    } else if (!strncmp(buf,"jfs",3)) {
							abms_storage_table.index[index].fs = ABMS_FS_JFS;
						    } else if (!strncmp(buf,"ext2",4)) {
							abms_storage_table.index[index].fs = ABMS_FS_EXT2;
						    } else if (!strncmp(buf,"ext3",4)) {
							abms_storage_table.index[index].fs = ABMS_FS_EXT3;
						    } else if (!strncmp(buf,"sqfs",3)) {
							abms_storage_table.index[index].fs = ABMS_FS_SQFS;
						    } else {
							abms_storage_table.index[index].fs = ABMS_FS_UNKNOWN;
						    }
						    free(buf);
						    buf = (char *) malloc(sizeof(ftmp));
						    buf[0] = '\0';
						}
						break;

					    case 2:
						if (c == ':') continue;
						if (c == '1') {
						    abms_storage_table.local[index].pread = 1;
						} else {
						    abms_storage_table.local[index].pread = 0;
						}
						tentry++;
						break;

					    case 3:
						if (c == ':') continue;
						if (c == '1') {
						    abms_storage_table.local[index].pwrite = 1;
						} else {
						    abms_storage_table.local[index].pwrite = 0;
						}
						tentry++;
						break;

					    case 4:
						if (c == ':') continue;
						if (c == '1') {
						    abms_storage_table.local[index].pexec = 1;
						} else {
						    abms_storage_table.local[index].pexec = 0;
						}
						tentry++;
						break;

					    case 5:
						if (c == ':') continue;
						if (c == '1') {
						    abms_storage_table.local[index].atime = 1;
						} else {
						    abms_storage_table.local[index].atime = 0;
						}
						tentry++;
						break;

					    default:
					    case 6:
						if (c == ':') continue;
						tentry++;
						tparsed = 1;
						break;
					}
				    }
				    break;

				default:
				    /* XXX: problem should do something to rescue the idiots */
				    break;
			    }
			    found = 0;
			    parsed = 1;		/* exit strategy */
			    break;

		    }
		    
		}
		/* process data config */
	    } else if (found == 2) {
		/* process stack config */
		/* name:version:uid:gid:ip:tcp-port:udp-port:storage_profile:read:write:execute */
		index = abms_app_table.nxt_idx;
		if (abms_app_table.nxt_idx < ABMS_SDT_MAX)
		    abms_app_table.nxt_idx++;
		pentry = 0;
		parsed = 0;
		if (c != ':') {
		    buf[j] = c;
		    j++;
		}
		while ((parsed == 0) && (!feof(stream))) {
		    lc = c;
		    c = (char) fgetc(stream);
		    switch(pentry) {
			case 0:
			    if (c != ':') {
			        buf[j] = c;
			        j++;
			    } else {
				buf[j] = '\0';
				j = 0;
				pentry++;
				sprintf(abms_app_table.stack[index].app.name,"%s",buf);
				free(buf);
				buf = (char *) malloc(sizeof(ftmp));
				buf[0] = '\0';
				continue;
			    }
			    break;

			case 1:
			    if (c != ':') {
			        buf[j] = c;
			        j++;
			    } else {
				buf[j] = '\0';
				j = 0;
				pentry++;
				sprintf(abms_app_table.stack[index].app.ver,"%s",buf);
				free(buf);
				buf = (char *) malloc(sizeof(ftmp));
				buf[0] = '\0';
				continue;
			    }
			    break;

			case 2:
			    if (c != ':') {
			        buf[j] = c;
			        j++;
			    } else {
				buf[j] = '\0';
				j = 0;
				pentry++;
				abms_app_table.stack[index].user.uid = (u32) atoi(buf);
				free(buf);
				buf = (char *) malloc(sizeof(ftmp));
				buf[0] = '\0';
				continue;
			    }
			    break;

			case 3:
			    if (c != ':') {
			        buf[j] = c;
			        j++;
			    } else {
				buf[j] = '\0';
				j = 0;
				pentry++;
				abms_app_table.stack[index].user.gid = (u32) atoi(buf);
				free(buf);
				buf = (char *) malloc(sizeof(ftmp));
				buf[0] = '\0';
				continue;
			    }
			    break;

			case 4: /* ip */
			    if (c != ':') {
			        buf[j] = c;
			        j++;
			    } else {
				buf[j] = '\0';
				j = 0;
				pentry++;
				abms_app_table.stack[index].net.ip = (u32) inet_network(buf);
				free(buf);
				buf = (char *) malloc(sizeof(ftmp));
				buf[0] = '\0';
				continue;
			    }
			    break;

			case 5:
			    if (c != ':') {
			        buf[j] = c;
			        j++;
			    } else {
				buf[j] = '\0';
				j = 0;
				pentry++;
				abms_app_table.stack[index].net.tcp = (u32) atoi(buf);
				free(buf);
				buf = (char *) malloc(sizeof(ftmp));
				buf[0] = '\0';
				continue;
			    }
			    break;

			case 6:
			    if (c != ':') {
			        buf[j] = c;
			        j++;
			    } else {
				buf[j] = '\0';
				j = 0;
				pentry++;
				abms_app_table.stack[index].net.udp = (u32) atoi(buf);
				free(buf);
				buf = (char *) malloc(sizeof(ftmp));
				buf[0] = '\0';
				continue;
			    }
			    break;

			case 7: 
			    if (c != ':') {
			        buf[j] = c;
			        j++;
			    } else {
				buf[j] = '\0';
				j = 0;
				pentry++;
				sprintf(abms_app_table.stack[index].data.profile,"%s",buf);
				free(buf);
				buf = (char *) malloc(sizeof(ftmp));
				buf[0] = '\0';
				continue;
			    }
			    break;

			case 8:
			    if (c != ':') {
			        buf[j] = c;
			        j++;
			    } else {
				buf[j] = '\0';
				j = 0;
				pentry++;
				abms_app_table.stack[index].data.pread = (u8) atoi(buf);
				free(buf);
				buf = (char *) malloc(sizeof(ftmp));
				buf[0] = '\0';
				continue;
			    }
			    break;

			case 9:
			    if (c != ':') {
			        buf[j] = c;
			        j++;
			    } else {
				buf[j] = '\0';
				j = 0;
				pentry++;
				abms_app_table.stack[index].data.pwrite = (u8) atoi(buf);
				free(buf);
				buf = (char *) malloc(sizeof(ftmp));
				buf[0] = '\0';
				continue;
			    }
			    break;

			case 10:
			    if (c != ':') {
			        buf[j] = c;
			        j++;
			    } else {
				buf[j] = '\0';
				j = 0;
				pentry++;
				abms_app_table.stack[index].data.size = (u8) atoi(buf);
				free(buf);
				buf = (char *) malloc(sizeof(ftmp));
				buf[0] = '\0';
				continue;
			    }
			    break;

			case 11:
			    if (c != ':') {
			        buf[j] = c;
			        j++;
			    } else {
				buf[j] = '\0';
				j = 0;
				pentry++;
				abms_app_table.stack[index].data.pexec = (u8) atoi(buf);
				free(buf);
				buf = (char *) malloc(sizeof(ftmp));
				buf[0] = '\0';
				continue;
			    }
			    found = 0;
			    parsed = 1;
			    break;

		    }
		}
	    } else if (found == 3) {
		parsed = 0;
		pentry = 0;
		tentry = 0;
		j = 0;
		if (c == 'm') {
		    while ((c != ':') && (!feof(stream))) {
		        lc = c;
		        c = (char) fgetc(stream);
		    	switch (c) {
			    case 'm':
			    	if (pentry == 1) {
			            pentry++;
			    	} else {
				    found = 0;
				    parsed = 1;
				    break;
			    	}
			    	break;

			    case 'g':
			    	if (pentry == 0) {
				    pentry++;
			    	} else {
				    found = 0;
				    parsed = 1;
				    break;
			    	}
			        break;

			    case 't':
			    	if (pentry == 2) {
				    pentry = 0;
			    	} else {
				    found = 0;
				    parsed = 1;
				    break;
			        }
			        break;
		       	}
		    }
		    index = abms_mnet_table.nxt_idx;
		    if (abms_mnet_table.nxt_idx < ABMS_NET_MGMTMAX)
		    	abms_mnet_table.nxt_idx++;
		    while ((parsed == 0) && (!feof(stream))) {
			lc = c;
			c = (char) fgetc(stream);
			switch (pentry) {
			    case 0:
				if (c != ':') {
				    buf[j] = c;
				    j++;
				} else {
				    buf[j] = '\0';
                                    j = 0;
                                    pentry++;
				    abms_mnet_table.mnet[index].mgmtip = (u32) inet_network(buf);
				    free(buf);
				    buf = (char *) malloc(sizeof(ftmp));
				    buf[0] = '\0';
				    continue;
			        }
				break;

			    case 1:
				if ((c != ':') && (c != '\n')) {
				    buf[j] = c;
				    j++;
				} else {
				    buf[j] = '\0';
                                    j = 0;
                                    pentry++;
				    abms_mnet_table.mnet[index].slash = (u16) atoi(buf);
				    free(buf);
				    buf = (char *) malloc(sizeof(ftmp));
				    buf[0] = '\0';
				    parsed = 1;
			        }
				break;
			}
		    }
		} else if (c == 'p') {
		    while ((c != ':') && (!feof(stream))) {
		        lc = c;
		        c = (char) fgetc(stream);
		    	switch (c) {
			    case 'o':
			    	if (pentry == 1) {
			            pentry++;
			    	} else {
				    found = 0;
				    parsed = 1;
				    break;
			    	}
			    	break;

			    case 'r':
			    	if (pentry == 0) {
				    pentry++;
			    	} else {
				    found = 0;
				    parsed = 1;
				    break;
			    	}
			        break;

			    case 'd':
			    	if (pentry == 2) {
				    pentry = 0;
			    	} else {
				    found = 0;
				    parsed = 1;
				    break;
			        }
			        break;
		       	}
		    }
		    index = abms_prodip_table.nxt_idx;
		    if (abms_prodip_table.nxt_idx < ABMS_NET_PRODIPMAX)
		    	abms_prodip_table.nxt_idx++;
		    while ((parsed == 0) && (!feof(stream))) {
			lc = c;
			c = (char) fgetc(stream);
			switch (pentry) {
			    case 0:
				if (mac == 0) {
				    mac++;
				    continue;
				} else if (mac == 1) {
				    mac = 0;
				    sprintf(buf,"%c%c",tolower(lc),tolower(c));
				    sscanf(buf,"%x",&abms_prodip_table.ipnet[index].mac[macc]);
				    macc++;
				}
				if (macc != 6) {
				    j++;
				} else {
				    j = 0;
				    pentry++;
				    free(buf);
				    buf = (char *) malloc(sizeof(ftmp));
				    buf[0] = '\0';
				    c = (char) fgetc(stream);		/* stupid hack */
				    continue;
				}
				break;

			    case 1:
				if (c != ':') {
				    buf[j] = c;
				    j++;
				} else {
				    buf[j] = '\0';
                                    j = 0;
                                    pentry++;
				    abms_prodip_table.ipnet[index].gwip = (u32) inet_network(buf);
				    free(buf);
				    buf = (char *) malloc(sizeof(ftmp));
				    buf[0] = '\0';
				    continue;
			        }
				break;

			    case 2:
				if (c != ':') {
				    buf[j] = c;
				    j++;
				} else {
				    buf[j] = '\0';
                                    j = 0;
                                    pentry++;
				    abms_prodip_table.ipnet[index].slash = (u16) atoi(buf);
				    free(buf);
				    buf = (char *) malloc(sizeof(ftmp));
				    buf[0] = '\0';
			        }
				break;

			    case 3:
				if (c != ':') {
				    buf[j] = c;
				    j++;
				} else {
				    buf[j] = '\0';
                                    j = 0;
                                    pentry++;
				    abms_prodip_table.ipnet[index].ipcount = (u16) atoi(buf);
				    if (abms_prodip_table.ipnet[index].ipcount > ABMS_NET_PRODIPMAX) {
					abms_prodip_table.ipnet[index].ipcount = ABMS_NET_PRODIPMAX;
				    }
				    free(buf);
				    buf = (char *) malloc(sizeof(ftmp));
				    buf[0] = '\0';
			        }
				break;

			    case 4:
				if ((c != ':') && (c != '\n')) {
				    buf[j] = c;
				    j++;
				} else if (tentry < abms_prodip_table.ipnet[index].ipcount) {
				    buf[j] = '\0';
				    j = 0;
				    abms_prodip_table.ipnet[index].iplist[tentry] = (u32) inet_network(buf);
				    free(buf);
				    buf = (char *) malloc(sizeof(ftmp));
				    buf[0] = '\0';
				    tentry++;
				    if (tentry == abms_prodip_table.ipnet[index].ipcount) {
					tentry = 0;
					j = 0;
					free(buf);
					buf = (char *) malloc(sizeof(ftmp));
					buf[0] = '\0';
					macc = 0;
					parsed = 1;
				    }
				} else {
				    tentry = 0;
				    j = 0;
				    free(buf);
				    buf = (char *) malloc(sizeof(ftmp));
				    buf[0] = '\0';
				    macc = 0;
				    parsed = 1;
				}
				break;

			    default:
				break;

			}
		    }
#if defined(ABMS_DEBUG_ANNOYING)
				    printf("\n\nAdded IP setup index %u\n",index);
				    printf("MAC = %02x:%02x:%02x:%02x:%02x:%02x\n", abms_prodip_table.ipnet[index].mac[0],
				        abms_prodip_table.ipnet[index].mac[1],
				        abms_prodip_table.ipnet[index].mac[2],
				        abms_prodip_table.ipnet[index].mac[3],
				        abms_prodip_table.ipnet[index].mac[4],
				        abms_prodip_table.ipnet[index].mac[5]);
				    printf("gwip = %llx slash = %lx ipcount = %lx",abms_prodip_table.ipnet[index].gwip, abms_prodip_table.ipnet[index].slash, abms_prodip_table.ipnet[index].ipcount);
				    printf("\nip = %llx\n\n",abms_prodip_table.ipnet[index].iplist[0]);
#endif /* defined(ABMS_DEBUG_ANNOYING) */

		} else {
		    found = 0;
		    parsed = 0;
		}
	    }
	}

	free(buf);
	free(fcmd);
        fclose(stream);	
	
	abms_flash_emfs();		/* do emfs */

#if defined(ABMS_DEBUG_ANNOYING)
	abms_map_dump();		/* debug thing */
#endif /* defined(ABMS_DEBUG_ANNOYING) */

	return;
}

void abms_flash_emfs(void)
{
	char emfs_src[128];
	char lcmd[255];

	sprintf(emfs_src,"%s/%s/%u/%s",ABMS_FLASHDST,ABMS_FLASHSLOT,abms_cfg.slot,ABMS_EMFS_IMAGE);

	abms_mkdir(ABMS_EMFS_DST,1);
	abms_mkdir(ABMS_EMFS_COMBINE,1);
	abms_eloop = abms_map_loopadd(ABMS_FS_SQFS,ABMS_LOOP_T_SYSTEM,0,emfs_src);

	if (!abms_eloop) {
	    sqfs_gen();
	    abms_ramimage = 1;
	}
	
	sprintf(lcmd,"%s%s %s %s -t squashfs -o loop=/dev/loop%u",ABMS_BINPATH,ACMD_MOUNT,emfs_src,ABMS_EMFS_DST,abms_eloop);
	abms_system(lcmd);

	/* at this point we now have the extended AppOS image mounted in /app/os/emfs. we need to merge */
	abms_flash_combine();	/* this call needs to generate abms.sqfs */

	/* umount emfs */
	sprintf(lcmd,"%s%s /dev/loop%u",ABMS_BINPATH,ACMD_UMOUNT,abms_eloop);
	abms_system(lcmd);

	/* remove directory */
	sprintf(lcmd,"%s%s %s",ABMS_BINPATH,ACMD_RMDIR,ABMS_EMFS_DST);
	abms_system(lcmd);

	return;
}

void abms_flash_combine(void)
{
	/* copy /app/os/emfs into /app/os/esys, and copy /app/os/sys into /app/os/esys, then generate */

	char lcmd[255];
	char flslot[255];

	sprintf(lcmd,"%s%s -a %s* %s",ABMS_BINPATH,ACMD_CP,ABMS_PATH,ABMS_EMFS_COMBINE);
	abms_system(lcmd);

	sprintf(lcmd,"%s%s -a %s/* %s",ABMS_BINPATH,ACMD_CP,ABMS_EMFS_DST,ABMS_EMFS_COMBINE);
	abms_system(lcmd);

	if (abms_ramimage) {
	    sprintf(lcmd,"%s%s %s",ABMS_BINPATH,ACMD_MKDIR,ABMS_IMAGEDIR);
      	    abms_system(lcmd);

            sprintf(lcmd,"%s%s %s %s%s",ABMS_BINPATH,ACMD_MKSQUASHFS,ABMS_EMFS_COMBINE,ABMS_IMAGEDIR,ABMS_IMAGE);
            abms_system(lcmd);
	} else {
	    sprintf(flslot,"%s/%s/%u/",ABMS_FLASHDST,ABMS_FLASHSLOT,abms_cfg.slot);

            sprintf(lcmd,"%s%s %s %s%s",ABMS_BINPATH,ACMD_MKSQUASHFS,ABMS_EMFS_COMBINE,flslot,ABMS_IMAGE);
            abms_system(lcmd);
	}
	return;
}

void abms_flash_auth(void)
{
	char lcmd[255];
	char flslot[255];

	/* this is a temporary hack -- this will be fixed / improvied by the Unified Auth Feature */

	sprintf(flslot,"%s%s %s/%s/%u/%s/%s /etc",ABMS_BINPATH,ACMD_CP,ABMS_FLASHDST,ABMS_FLASHSLOT,abms_cfg.slot,ABMS_FLAUTH,ABMS_FLPASS);
	abms_system(lcmd);

	sprintf(flslot,"%s%s %s/%s/%u/%s/%s /etc",ABMS_BINPATH,ACMD_CP,ABMS_FLASHDST,ABMS_FLASHSLOT,abms_cfg.slot,ABMS_FLAUTH,ABMS_FLSHADOW);
	abms_system(lcmd);

	sprintf(flslot,"%s%s %s/%s/%u/%s/%s /etc",ABMS_BINPATH,ACMD_CP,ABMS_FLASHDST,ABMS_FLASHSLOT,abms_cfg.slot,ABMS_FLAUTH,ABMS_FLGROUP);
	abms_system(lcmd);

}
