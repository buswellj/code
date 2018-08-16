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

void abms_preinit(void)
{
	ramfs_init();
}

void abms_init(void)
{
	pass_init();
	sshd_init();

	if ((abms_cfg.mode >= 0x64) && (abms_cfg.mode <= 0xc7)) {
	    if (strlen(abms_cfg.flashdev) > 3) {
		abms_ramimage = 0;			/* XXX - make this configurable */
	        abms_flash_init();
	    } else {
		abms_ramimage = 1;
		sqfs_gen();
	    }
	} else {
	    abms_ramimage = 1;
	    sqfs_gen();
	}
	ramfs_clean();
	sqfs_init();
}

void ramfs_init(void)
{
	char lcmd[255];

	sprintf(lcmd,"/proc /dev /root /sys /dev/pts /dev/shm");
	abms_mkdir(lcmd,1);

	abms_mount("tmpfs","shm","/dev/shm");
	abms_mount("proc","proc","/proc");
	abms_mount("sysfs","sysfs","/sys");
	abms_mknod("/dev/tty",ABMS_CHARDEV,5,0);
	abms_mknod("/dev/random",ABMS_CHARDEV,1,8);
	abms_mknod("/dev/urandom",ABMS_CHARDEV,1,9);
	abms_mknod("/dev/frandom",ABMS_CHARDEV,235,11);
	abms_mknod("/dev/erandom",ABMS_CHARDEV,235,12);
	abms_mknod("/dev/ptmx",ABMS_CHARDEV,5,2);
	abms_mknod("/dev/loop0",ABMS_BLOCKDEV,7,0);
	abms_mknod("/dev/grsec",ABMS_CHARDEV,1,13);
	abms_chmod("666","/dev/ptmx");
	abms_chmod("666","/dev/tty");
	abms_mount("devpts","devpts","/dev/pts");

	sprintf(lcmd,"/app/os/var/dhcpcd");
	abms_mkdir(lcmd,1);

	sprintf(lcmd,"/app/os/var/run");
	abms_mkdir(lcmd,1);

	return;
}

void pass_init(void)
{
	char lcmd[255];

	sprintf(lcmd,"%s%s",ABMS_BINPATH,ACMD_GENPASS);
	abms_system(lcmd);

	sprintf(lcmd,"%s%s -rf %s%s",ABMS_BINPATH,ACMD_RM,ABMS_BINPATH,ACMD_GENPASS);
	abms_system(lcmd);

	return;
}

void sshd_init(void)
{
	char lcmd[255];

	sprintf(lcmd,"%s%s /app/os/sys/sshd/keys -p",ABMS_BINPATH,ACMD_MKDIR);
	abms_system(lcmd);

	sprintf(lcmd,"%s -t rsa -f %s",ABMS_KEYGEN,ABMS_KEY);
	abms_system(lcmd);

	return;
}

void sqfs_gen(void)
{
	char lcmd[255];

	sprintf(lcmd,"%s%s %s",ABMS_BINPATH,ACMD_MKDIR,ABMS_IMAGEDIR);
	abms_system(lcmd);

	sprintf(lcmd,"%s%s %s %s%s",ABMS_BINPATH,ACMD_MKSQUASHFS,ABMS_PATH,ABMS_IMAGEDIR,ABMS_IMAGE);
	abms_system(lcmd);

	return;

}

void ramfs_clean(void)
{
	char lcmd[255];
	u8 rmcnt = 0;

	sprintf(lcmd,"%ssshd",ABMS_PATH);
	abms_rm(lcmd);

	for (rmcnt = ABMS_ASCSTART; rmcnt <= ABMS_ASCEND; rmcnt++) {
	    if (rmcnt == 0x61) {
		sprintf(lcmd,"%sarp",ABMS_BINPATH);
	    } else if (rmcnt == 0x6d) {
		continue;
	    } else {
	        sprintf(lcmd,"%s%c*",ABMS_BINPATH,rmcnt);
	    }
	    abms_rm(lcmd);
	}

	sprintf(lcmd,"%slib/libn*",ABMS_PATH);
	abms_rm(lcmd);
	sprintf(lcmd,"%slib/libp*",ABMS_PATH);
	abms_rm(lcmd);
	sprintf(lcmd,"%slib/libm*",ABMS_PATH);
	abms_rm(lcmd);
	sprintf(lcmd,"%slib/librt*",ABMS_PATH);
	abms_rm(lcmd);
	sprintf(lcmd,"%slib/libz*",ABMS_PATH);
	abms_rm(lcmd);
	sprintf(lcmd,"%slib/libutil*",ABMS_PATH);
	abms_rm(lcmd);

	return;

}

void sqfs_init(void)
{
	char lcmd[255];
	char flslot[255];

	if (!abms_ramimage) {
	    sprintf(flslot,"%s/%s/%u/",ABMS_FLASHDST,ABMS_FLASHSLOT,abms_cfg.slot);
	    sprintf(lcmd,"%s%s %s%s %s -t squashfs -o loop=/dev/loop0",ABMS_BINPATH,ACMD_MOUNT,flslot,ABMS_IMAGE,ABMS_PATH);
	    abms_system(lcmd);

	    abms_rm(ABMS_EMFS_COMBINE);
	} else {
	    sprintf(lcmd,"%s%s %s%s %s -t squashfs -o loop=/dev/loop0",ABMS_BINPATH,ACMD_MOUNT,ABMS_IMAGEDIR,ABMS_IMAGE,ABMS_PATH);
	    abms_system(lcmd);
	}

	return;
}
