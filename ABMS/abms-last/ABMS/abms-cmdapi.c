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

void abms_mount(char *src, char *fs, char *dst)
{
	char lcmd[255];

	sprintf(lcmd,"%s%s -vt %s %s %s",ABMS_BINPATH,ACMD_MOUNT,src,fs,dst);
	abms_system(lcmd); 
}

void abms_mountshm(char *dst, u32 size)
{
	char lcmd[255];

	if (size == 0) {
  	    sprintf(lcmd,"%s%s -vt tmpfs shm %s",ABMS_BINPATH,ACMD_MOUNT,dst);
	} else {
  	    sprintf(lcmd,"%s%s -vt tmpfs shm %s -o %llu",ABMS_BINPATH,ACMD_MOUNT,dst,size);
	}
	abms_system(lcmd); 
}

void abms_mountf(char *src, char *fs, char *dst, u8 fsrw)
{
	char lcmd[255];

	if (fsrw) {
	    sprintf(lcmd,"%s%s -o noatime,nodiratime,nodev,nosuid,noexec,nouser,rw -vt %s %s %s",ABMS_BINPATH,ACMD_MOUNT,src,fs,dst);
	} else {
	    sprintf(lcmd,"%s%s -o noatime,nodiratime,nodev,nosuid,noexec,nouser,ro -vt %s %s %s",ABMS_BINPATH,ACMD_MOUNT,src,fs,dst);
	}
	abms_system(lcmd); 
}

void abms_mknod(char *dst, u8 type, u16 major, u16 minor)
{
	char lcmd[255];

	if (type == ABMS_BLOCKDEV) {
	    sprintf(lcmd,"%s%s %s b %lu %lu",ABMS_BINPATH,ACMD_MKNOD,dst,major,minor);
	} else {
	    sprintf(lcmd,"%s%s %s c %lu %lu",ABMS_BINPATH,ACMD_MKNOD,dst,major,minor);
	}
	abms_system(lcmd);
}

void abms_mkdir(char *path, u8 p)
{
	char lcmd[255];

	if (p) {
	    sprintf(lcmd,"%s%s %s -p",ABMS_BINPATH,ACMD_MKDIR,path);
	} else {
	    sprintf(lcmd,"%s%s %s",ABMS_BINPATH,ACMD_MKDIR,path);
	}

	abms_system(lcmd);
}

void abms_chmod(char *perm, char *path)
{
	char lcmd[255];

	sprintf(lcmd,"%s%s %s %s",ABMS_BINPATH,ACMD_CHMOD,perm,path);
	abms_system(lcmd);
}

void abms_rm(char *dst)
{
	char lcmd[255];

	sprintf(lcmd,"%s%s -rf %s",ABMS_BINPATH,ACMD_RM,dst);
	abms_system(lcmd);
}


void abms_losetup(char *loop, char *image)
{
	char lcmd[255];

	sprintf(lcmd,"%s%s %s %s",ABMS_BINPATH,ACMD_LOSETUP,loop,image);
	abms_system(lcmd);
}


void abms_symlink(char *src, char *dst)
{
	char lcmd[255];

	sprintf(lcmd,"%s%s -s %s %s",ABMS_BINPATH,ACMD_LN,src,dst);
	abms_system(lcmd);
}


