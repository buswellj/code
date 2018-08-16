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
#include "abms.h"

void abms_init(void)
{
	ramfs_init();
	pass_init();
	sshd_init();
	sqfs_gen();
	ramfs_clean();
	sqfs_init();
}

void ramfs_init(void)
{
	char lcmd[255];

	sprintf(lcmd,"%s%s /proc /dev /root /sys /dev/pts /dev/shm",ABMS_BINPATH,ACMD_MKDIR);
	abms_system(lcmd);

	sprintf(lcmd,"%s%s -vt tmpfs shm /dev/shm",ABMS_BINPATH,ACMD_MOUNT);
	abms_system(lcmd);

	sprintf(lcmd,"%s%s -vt proc proc /proc",ABMS_BINPATH,ACMD_MOUNT);
	abms_system(lcmd);

	sprintf(lcmd,"%s%s -vt sysfs sysfs /sys",ABMS_BINPATH,ACMD_MOUNT);
	abms_system(lcmd);

	sprintf(lcmd,"%s%s /dev/tty c 5 0",ABMS_BINPATH,ACMD_MKNOD);
	abms_system(lcmd);

	sprintf(lcmd,"%s%s /dev/random c 1 8",ABMS_BINPATH,ACMD_MKNOD);
	abms_system(lcmd);

	sprintf(lcmd,"%s%s /dev/urandom c 1 9",ABMS_BINPATH,ACMD_MKNOD);
	abms_system(lcmd);

	sprintf(lcmd,"%s%s /dev/frandom c 235 11",ABMS_BINPATH,ACMD_MKNOD);
	abms_system(lcmd);

	sprintf(lcmd,"%s%s /dev/erandom c 235 12",ABMS_BINPATH,ACMD_MKNOD);
	abms_system(lcmd);

	sprintf(lcmd,"%s%s /dev/ptmx c 5 2",ABMS_BINPATH,ACMD_MKNOD);
	abms_system(lcmd);

	sprintf(lcmd,"%s%s /dev/loop0 b 7 0",ABMS_BINPATH,ACMD_MKNOD);
	abms_system(lcmd);

	sprintf(lcmd,"%s%s /dev/grsec c 1 13",ABMS_BINPATH,ACMD_MKNOD);
	abms_system(lcmd);

	sprintf(lcmd,"%s%s 666 /dev/ptmx",ABMS_BINPATH,ACMD_CHMOD);
	abms_system(lcmd);

	sprintf(lcmd,"%s%s 666 /dev/tty",ABMS_BINPATH,ACMD_CHMOD);
	abms_system(lcmd);

	sprintf(lcmd,"%s%s -vt devpts devpts /dev/pts",ABMS_BINPATH,ACMD_MOUNT);
	abms_system(lcmd);

	sprintf(lcmd,"%s%s /app/os/var/dhcpcd -p",ABMS_BINPATH,ACMD_MKDIR);
	abms_system(lcmd);

	sprintf(lcmd,"%s%s /app/os/var/run -p",ABMS_BINPATH,ACMD_MKDIR);
	abms_system(lcmd);

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

	sprintf(lcmd,"%s%s -rf %ssshd",ABMS_BINPATH,ACMD_RM,ABMS_PATH);
	abms_system(lcmd);

	sprintf(lcmd,"%s%s -rf %sarp",ABMS_BINPATH,ACMD_RM,ABMS_BINPATH);
	abms_system(lcmd);

	sprintf(lcmd,"%s%s -rf %sb*",ABMS_BINPATH,ACMD_RM,ABMS_BINPATH);
	abms_system(lcmd);

	sprintf(lcmd,"%s%s -rf %sc*",ABMS_BINPATH,ACMD_RM,ABMS_BINPATH);
	abms_system(lcmd);

	sprintf(lcmd,"%s%s -rf %sd*",ABMS_BINPATH,ACMD_RM,ABMS_BINPATH);
	abms_system(lcmd);

	sprintf(lcmd,"%s%s -rf %se*",ABMS_BINPATH,ACMD_RM,ABMS_BINPATH);
	abms_system(lcmd);

	sprintf(lcmd,"%s%s -rf %sf*",ABMS_BINPATH,ACMD_RM,ABMS_BINPATH);
	abms_system(lcmd);

	sprintf(lcmd,"%s%s -rf %sg*",ABMS_BINPATH,ACMD_RM,ABMS_BINPATH);
	abms_system(lcmd);

	sprintf(lcmd,"%s%s -rf %sh*",ABMS_BINPATH,ACMD_RM,ABMS_BINPATH);
	abms_system(lcmd);

	sprintf(lcmd,"%s%s -rf %si*",ABMS_BINPATH,ACMD_RM,ABMS_BINPATH);
	abms_system(lcmd);

	sprintf(lcmd,"%s%s -rf %sk*",ABMS_BINPATH,ACMD_RM,ABMS_BINPATH);
	abms_system(lcmd);

	sprintf(lcmd,"%s%s -rf %sn*",ABMS_BINPATH,ACMD_RM,ABMS_BINPATH);
	abms_system(lcmd);

	sprintf(lcmd,"%s%s -rf %sp*",ABMS_BINPATH,ACMD_RM,ABMS_BINPATH);
	abms_system(lcmd);

	sprintf(lcmd,"%s%s -rf %ss*",ABMS_BINPATH,ACMD_RM,ABMS_BINPATH);
	abms_system(lcmd);

	sprintf(lcmd,"%s%s -rf %st*",ABMS_BINPATH,ACMD_RM,ABMS_BINPATH);
	abms_system(lcmd);

	sprintf(lcmd,"%s%s -rf %su*",ABMS_BINPATH,ACMD_RM,ABMS_BINPATH);
	abms_system(lcmd);

	sprintf(lcmd,"%s%s -rf %sv*",ABMS_BINPATH,ACMD_RM,ABMS_BINPATH);
	abms_system(lcmd);

	sprintf(lcmd,"%s%s -rf %sw*",ABMS_BINPATH,ACMD_RM,ABMS_BINPATH);
	abms_system(lcmd);

	sprintf(lcmd,"%s%s -rf %sx*",ABMS_BINPATH,ACMD_RM,ABMS_BINPATH);
	abms_system(lcmd);

	sprintf(lcmd,"%s%s -rf %sz*",ABMS_BINPATH,ACMD_RM,ABMS_BINPATH);
	abms_system(lcmd);

	sprintf(lcmd,"%s%s -rf %slib/libn*",ABMS_BINPATH,ACMD_RM,ABMS_PATH);
	abms_system(lcmd);

	sprintf(lcmd,"%s%s -rf %slib/libp*",ABMS_BINPATH,ACMD_RM,ABMS_PATH);
	abms_system(lcmd);

	sprintf(lcmd,"%s%s -rf %slib/libm*",ABMS_BINPATH,ACMD_RM,ABMS_PATH);
	abms_system(lcmd);

	sprintf(lcmd,"%s%s -rf %slib/librt*",ABMS_BINPATH,ACMD_RM,ABMS_PATH);
	abms_system(lcmd);

	sprintf(lcmd,"%s%s -rf %slib/libz*",ABMS_BINPATH,ACMD_RM,ABMS_PATH);
	abms_system(lcmd);

	sprintf(lcmd,"%s%s -rf %slib/libutil*",ABMS_BINPATH,ACMD_RM,ABMS_PATH);
	abms_system(lcmd);

	return;

}

void sqfs_init(void)
{

	char lcmd[255];

	sprintf(lcmd,"%s%s %s%s %s -t squashfs -o loop=/dev/loop0",ABMS_BINPATH,ACMD_MOUNT,ABMS_IMAGEDIR,ABMS_IMAGE,ABMS_PATH);
	abms_system(lcmd);

	return;

}
