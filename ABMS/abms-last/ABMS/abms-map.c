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

#include <stdio.h>
#include <string.h>
#include "abms.h"
#include "abms-map.h"

extern abms_loop_t abms_loop_table;
extern abms_sdt_t abms_app_table;
extern abms_dmt_t abms_storage_table;
extern abms_prodip_t abms_prodip_table;
extern abms_mgmt_t abms_mnet_table;

void abms_map_init(void)
{
	abms_loop_t *alp;
	abms_sdt_t *asp;
	abms_dmt_t *adp;
	abms_mgmt_t *amp;
	abms_prodip_t *app;

	alp = &abms_loop_table;
	asp = &abms_app_table;
	adp = &abms_storage_table;
	amp = &abms_mnet_table;
	app = &abms_prodip_table;

	memset(alp,'\0',sizeof(abms_loop_table));
	memset(asp,'\0',sizeof(abms_app_table));
	memset(adp,'\0',sizeof(abms_storage_table));
	memset(amp,'\0',sizeof(abms_mnet_table));
	memset(app,'\0',sizeof(abms_prodip_table));

	abms_loop_table.nxt_idx++;
}

u8 abms_map_loopadd(u8 fs, u8 type, u8 crypto, char *src)
{
	u8 idx = abms_loop_table.nxt_idx;
	char nod[64];


	if (idx == ABMS_LOOP_MAX) {
	    if (abms_loop_table.free_ooo[abms_loop_table.fr_idx]) {
		idx = abms_loop_table.free_ooo[abms_loop_table.fr_idx];
		abms_loop_table.free_ooo[abms_loop_table.fr_idx] = 0;
		abms_loop_table.fr_idx++;
	    } else {
		return(0);	/* no more devices */
	    }
	}

	if ((idx) && (idx < ABMS_LOOP_MAX)) {
	    abms_loop_table.map[idx].id = idx;
	    abms_loop_table.map[idx].fs = fs;
	    abms_loop_table.map[idx].type = type;
	    abms_loop_table.map[idx].crypto = crypto;
	    abms_strlcpy(abms_loop_table.map[idx].src, src, strlen(src) + 1);
	    abms_loop_table.nxt_idx++;
	    sprintf(nod,"/dev/loop%u",idx);
	    abms_mknod(nod,ABMS_BLOCKDEV,7,idx);
	    if ((type == ABMS_LOOP_T_STACK) || (type == ABMS_LOOP_T_CFG) || (type == ABMS_LOOP_T_COMMON)) {
		abms_losetup(nod,src);
	    }
	    return(idx);
	}
	return(0);
}
void abms_map_dump(void)
{
	u8 idx = abms_app_table.nxt_idx;
	u8 i;

	if (idx) {
	    for (i = 0; i < idx; i++) {
		printf("\n App Index = %u %s-%s\n", i, abms_app_table.stack[i].app.name, abms_app_table.stack[i].app.ver);
		printf("uid = %llu gid = %llu\n", abms_app_table.stack[i].user.uid, abms_app_table.stack[i].user.gid);
		printf("ip = %llx tcp = %llu udp = %llu\n", abms_app_table.stack[i].net.ip, abms_app_table.stack[i].net.tcp, abms_app_table.stack[i].net.udp);

	    }
	}

	idx = abms_storage_table.nxt_idx;

	if (idx) {
	    for (i = 0; i < idx; i++) {
		printf("\n Storage Index = %u %s %x %x\n", i, abms_storage_table.index[i].name, abms_storage_table.index[i].type, abms_storage_table.index[i].fs);
		if (abms_storage_table.index[i].type == ABMS_STORAGE_LOCAL) {
			printf("%s %d %d %d %d\n",abms_storage_table.local[i].sdev, abms_storage_table.local[i].pread, abms_storage_table.local[i].pwrite, 
				abms_storage_table.local[i].pexec, abms_storage_table.local[i].atime);
		}
	    }
	}
}
