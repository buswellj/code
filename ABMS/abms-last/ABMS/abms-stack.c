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

void abms_stack_init(void)
{
	char lcmd[255];
	char mpath[255];
	char npath[255];
	u8 i = 0, idx = 0;

	idx = abms_app_table.nxt_idx;

	if (!idx) {
	    /* There are no stacks configure */
	    abms_nostacks = 1;
	    return;
	}

	printf("\n\n\nBEGIN STACK\n\n\n");

	abms_mkdir(ABMS_STACK_IMAGES,1);
	abms_mkdir(ABMS_STACK_PRODUCTION,1);

	for (i = 0; i < idx; i++) {
	    sprintf(mpath,"%s%s-%s/",ABMS_STACK_IMAGES,abms_app_table.stack[i].app.name, abms_app_table.stack[i].app.ver);
	    sprintf(lcmd,"%s%s-%s.stk",ABMS_STACK_FLASHIMAGE,abms_app_table.stack[i].app.name, abms_app_table.stack[i].app.ver);
	    abms_mkdir(mpath,1);

	    abms_app_table.stack[i].loopmap.master = abms_map_loopadd(ABMS_FS_SQFS,ABMS_LOOP_T_STACK,0,lcmd);
	    sprintf(npath,"/dev/loop%u",abms_app_table.stack[i].loopmap.master);

	    abms_mount(npath,"squashfs",mpath);

	    sprintf(mpath,"%s%s-%s",ABMS_STACK_PRODUCTION,abms_app_table.stack[i].app.name, abms_app_table.stack[i].app.ver);
	    sprintf(lcmd,"%s%s%s/",mpath,ABMS_STACK_CHROOTSTACK,abms_app_table.stack[i].app.name);
	    abms_mkdir(mpath,1);

	    abms_mountshm(mpath,0);
	    abms_mkdir(lcmd,1);

	    sprintf(lcmd,"%s%s",mpath,ABMS_STACK_CHROOTCOMMON);
	    abms_mkdir(lcmd,1);

	    sprintf(lcmd,"%s%s",mpath,ABMS_STACK_CHROOTSUPPORT);
	    abms_mkdir(lcmd,1);

	    sprintf(lcmd,"%s%s-%s/%s",ABMS_STACK_IMAGES,abms_app_table.stack[i].app.name, abms_app_table.stack[i].app.ver,ABMS_STACK_STACK);
	    abms_app_table.stack[i].loopmap.stack = abms_map_loopadd(ABMS_FS_SQFS,ABMS_LOOP_T_STACK,0,lcmd);

	    sprintf(lcmd,"%s%s-%s/%s",ABMS_STACK_IMAGES,abms_app_table.stack[i].app.name, abms_app_table.stack[i].app.ver,ABMS_STACK_COMMON);
	    abms_app_table.stack[i].loopmap.common = abms_map_loopadd(ABMS_FS_SQFS,ABMS_LOOP_T_COMMON,0,lcmd);

	    sprintf(lcmd,"%s%s-%s/%s",ABMS_STACK_IMAGES,abms_app_table.stack[i].app.name, abms_app_table.stack[i].app.ver,ABMS_STACK_SUPPORT);
	    abms_app_table.stack[i].loopmap.support = abms_map_loopadd(ABMS_FS_SQFS,ABMS_LOOP_T_COMMON,0,lcmd);

	    sprintf(lcmd,"%s%s%s", mpath, ABMS_STACK_CHROOTSTACK, abms_app_table.stack[i].app.name);
	    sprintf(npath,"/dev/loop%u",abms_app_table.stack[i].loopmap.stack);
	    abms_mount(npath,"squashfs",lcmd);

	    sprintf(lcmd,"%s%s", mpath, ABMS_STACK_CHROOTCOMMON);
	    sprintf(npath,"/dev/loop%u",abms_app_table.stack[i].loopmap.common);
	    abms_mount(npath,"squashfs",lcmd);

	    sprintf(lcmd,"%s%s", mpath, ABMS_STACK_CHROOTSUPPORT);
	    sprintf(npath,"/dev/loop%u",abms_app_table.stack[i].loopmap.support);
	    abms_mount(npath,"squashfs",lcmd);

	    sprintf(lcmd,"%s/bin",mpath);
	    sprintf(npath,"app/support/bin/");
	    abms_symlink(npath,lcmd);

	    sprintf(lcmd,"%s/usr",mpath);
	    sprintf(npath,"app/support/usr/");
	    abms_symlink(npath,lcmd);

	    sprintf(lcmd,"%s/lib",mpath);
	    sprintf(npath,"app/support/lib/");
	    abms_symlink(npath,lcmd);

	    sprintf(lcmd,"%s/dev",mpath);
	    abms_mkdir(lcmd, 1);
	    sprintf(lcmd,"%s/dev/pts",mpath);
	    abms_mkdir(lcmd, 1);
	    sprintf(lcmd,"%s/dev/shm",mpath);
	    abms_mkdir(lcmd, 1);
	    sprintf(lcmd,"%s/proc",mpath);
	    abms_mkdir(lcmd, 1);
	    sprintf(lcmd,"%s/sys",mpath);
	    abms_mkdir(lcmd, 1);
	    sprintf(lcmd,"%s/var",mpath);
	    abms_mkdir(lcmd, 1);
	    sprintf(lcmd,"%s/config",mpath);
	    abms_mkdir(lcmd, 1);

	    sprintf(lcmd,"%s%s-%s.cfi",ABMS_STACK_FLASHCONFIG,abms_app_table.stack[i].app.name, abms_app_table.stack[i].app.ver);
	    abms_app_table.stack[i].loopmap.cfg = abms_map_loopadd(ABMS_FS_SQFS,ABMS_LOOP_T_CFG,0,lcmd);

	    sprintf(lcmd,"%s%s", mpath, ABMS_STACK_CHROOTCONFIG);
	    sprintf(npath,"/dev/loop%u",abms_app_table.stack[i].loopmap.cfg);
	    abms_mount(npath,"squashfs",lcmd);

	    sprintf(lcmd,"%s/dev/pts",mpath);
	    abms_mount("devpts","devpts",lcmd);

	    sprintf(lcmd,"%s/dev/shm",mpath);
	    abms_mount("tmpfs","shm",lcmd);

	    sprintf(lcmd,"%s/proc",mpath);
	    abms_mount("proc","proc",lcmd);

	    sprintf(lcmd,"%s/sys",mpath);
	    abms_mount("sysfs","sysfs",lcmd);

	    /* abms_aaf_addentry(abms_app_table.stack[i].net.ip, abms_app_table.stack[i].net.tcp, abms_app_table.stack[i].net.udp, i) */
	}
	printf("\n\n\nEND STACK\n\n\n");

}
